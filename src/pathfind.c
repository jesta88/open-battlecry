#include "pathfind.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// A* pathfinding with bucketed open list
// Matches WBC3's MovePath.cpp approach: integer costs, Chebyshev heuristic,
// 8-directional expansion, bucket array for O(1) insert / near-O(1) extract.
// ---------------------------------------------------------------------------

enum
{
	MAX_BUCKETS = 4000,
	NODE_NONE   = 0,
	NODE_OPEN   = 1,
	NODE_CLOSED = 2,
};

typedef struct
{
	uint16_t prev_x, prev_y;
	uint16_t cost_so_far;
	uint16_t total_cost;
	uint32_t generation; // avoids memset: node valid only if == current search generation
	uint8_t status;      // NODE_NONE / NODE_OPEN / NODE_CLOSED
} pathfind_node;

// Open list bucket entry (intrusive linked list via index)
typedef struct
{
	uint32_t x, y;
	uint32_t next; // index into open_entries pool, UINT32_MAX = end
} open_entry;

// Search state (allocated once, reused across calls)
static struct
{
	pathfind_node* nodes; // map_width * map_height
	uint32_t nodes_w, nodes_h;
	uint32_t generation;

	// Bucketed open list
	uint32_t buckets[MAX_BUCKETS]; // head index per cost bucket
	uint32_t min_bucket;           // lowest non-empty bucket

	// Open entry pool
	open_entry* pool;
	uint32_t pool_count;
	uint32_t pool_capacity;
} s;

static pathfind_node* get_node(uint32_t x, uint32_t y)
{
	return &s.nodes[y * s.nodes_w + x];
}

static bool node_is_valid(const pathfind_node* n)
{
	return n->generation == s.generation;
}

static void ensure_state(uint32_t w, uint32_t h)
{
	if (s.nodes_w == w && s.nodes_h == h && s.nodes)
		return;
	free(s.nodes);
	free(s.pool);
	s.nodes_w = w;
	s.nodes_h = h;
	s.nodes = calloc((size_t)w * h, sizeof(pathfind_node));
	s.pool_capacity = w * h; // worst case
	s.pool = malloc(s.pool_capacity * sizeof(open_entry));
	s.generation = 0;
}

static void reset_search(void)
{
	s.generation++;
	// If generation wraps, force-clear (extremely rare)
	if (s.generation == 0)
	{
		memset(s.nodes, 0, (size_t)s.nodes_w * s.nodes_h * sizeof(pathfind_node));
		s.generation = 1;
	}
	for (uint32_t i = 0; i < MAX_BUCKETS; i++)
		s.buckets[i] = UINT32_MAX;
	s.min_bucket = MAX_BUCKETS;
	s.pool_count = 0;
}

static void open_push(uint32_t x, uint32_t y, uint16_t total_cost)
{
	uint32_t bucket = total_cost < MAX_BUCKETS ? total_cost : MAX_BUCKETS - 1;

	// Allocate from pool
	uint32_t idx = s.pool_count++;
	s.pool[idx] = (open_entry){.x = x, .y = y, .next = s.buckets[bucket]};
	s.buckets[bucket] = idx;

	if (bucket < s.min_bucket)
		s.min_bucket = bucket;
}

static bool open_pop(uint32_t* out_x, uint32_t* out_y)
{
	while (s.min_bucket < MAX_BUCKETS)
	{
		uint32_t idx = s.buckets[s.min_bucket];
		while (idx != UINT32_MAX)
		{
			open_entry* e = &s.pool[idx];
			pathfind_node* n = get_node(e->x, e->y);
			// Skip stale entries (node already closed or re-opened at lower cost)
			if (node_is_valid(n) && n->status == NODE_OPEN)
			{
				*out_x = e->x;
				*out_y = e->y;
				s.buckets[s.min_bucket] = e->next;
				n->status = NODE_CLOSED;
				return true;
			}
			idx = e->next;
		}
		s.buckets[s.min_bucket] = UINT32_MAX;
		s.min_bucket++;
	}
	return false;
}

static uint16_t heuristic(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
	int dx = abs((int)ax - (int)bx);
	int dy = abs((int)ay - (int)by);
	// Chebyshev distance * min terrain cost (2)
	return (uint16_t)((dx > dy ? dx : dy) * 2);
}

// 8-directional neighbor offsets
static const int dx8[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int dy8[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
static const bool diag[8] = {false, true, false, true, false, true, false, true};

bool pathfind_find(const game_map* map, uint16_t sx, uint16_t sy,
                   uint16_t dx, uint16_t dy, uint8_t move_mode,
                   path_result* out)
{
	out->length = 0;
	out->current = 0;

	// Bounds check
	if (sx >= map->width || sy >= map->height || dx >= map->width || dy >= map->height)
		return false;

	// Destination must be walkable
	if (!map_walkable(map, dx, dy, move_mode))
		return false;

	// Trivial case
	if (sx == dx && sy == dy)
	{
		out->points[0] = (path_point){dx, dy};
		out->length = 1;
		return true;
	}

	ensure_state(map->width, map->height);
	reset_search();

	// Initialize start node
	pathfind_node* start = get_node(sx, sy);
	start->generation = s.generation;
	start->prev_x = sx;
	start->prev_y = sy;
	start->cost_so_far = 0;
	start->total_cost = heuristic(sx, sy, dx, dy);
	start->status = NODE_OPEN;
	open_push(sx, sy, start->total_cost);

	bool found = false;

	while (!found)
	{
		uint32_t cx, cy;
		if (!open_pop(&cx, &cy))
			break; // no path

		if (cx == dx && cy == dy)
		{
			found = true;
			break;
		}

		pathfind_node* cur = get_node(cx, cy);

		// Expand 8 neighbors
		for (int d = 0; d < 8; d++)
		{
			int nx = (int)cx + dx8[d];
			int ny = (int)cy + dy8[d];
			if (nx < 0 || ny < 0 || (uint32_t)nx >= map->width || (uint32_t)ny >= map->height)
				continue;

			uint16_t terrain_cost = map_cost(map, (uint32_t)nx, (uint32_t)ny, move_mode);
			if (terrain_cost >= 1000)
				continue; // impassable

			// Diagonal movement cost = terrain_cost * 3 / 2
			uint16_t step_cost = diag[d] ? (terrain_cost * 3 / 2) : terrain_cost;
			uint16_t new_cost = cur->cost_so_far + step_cost;

			pathfind_node* nb = get_node((uint32_t)nx, (uint32_t)ny);

			if (node_is_valid(nb))
			{
				// Already visited — only update if cheaper
				if (nb->status == NODE_CLOSED || new_cost >= nb->cost_so_far)
					continue;
			}

			uint16_t h = heuristic((uint16_t)nx, (uint16_t)ny, dx, dy);
			uint16_t total = new_cost + h;

			nb->generation = s.generation;
			nb->prev_x = (uint16_t)cx;
			nb->prev_y = (uint16_t)cy;
			nb->cost_so_far = new_cost;
			nb->total_cost = total;
			nb->status = NODE_OPEN;
			open_push((uint32_t)nx, (uint32_t)ny, total);
		}
	}

	if (!found)
		return false;

	// Trace back from destination to start
	uint16_t trace[PATHFIND_MAX_PATH * 2]; // x,y pairs
	uint32_t trace_len = 0;
	uint32_t tx = dx, ty = dy;
	while (trace_len < PATHFIND_MAX_PATH)
	{
		trace[trace_len * 2 + 0] = (uint16_t)tx;
		trace[trace_len * 2 + 1] = (uint16_t)ty;
		trace_len++;

		if (tx == sx && ty == sy)
			break;

		pathfind_node* n = get_node(tx, ty);
		uint32_t px = n->prev_x, py = n->prev_y;
		tx = px;
		ty = py;
	}

	// Reverse into output
	if (trace_len > PATHFIND_MAX_PATH)
		trace_len = PATHFIND_MAX_PATH;
	out->length = (uint16_t)trace_len;
	for (uint32_t i = 0; i < trace_len; i++)
	{
		uint32_t ri = trace_len - 1 - i;
		out->points[i] = (path_point){trace[ri * 2 + 0], trace[ri * 2 + 1]};
	}

	return true;
}
