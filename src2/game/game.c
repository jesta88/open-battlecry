#include "include/game.h"

#include "../../include/engine/arena.h"
#include "../../include/engine/jobs.h"
#include "../../include/engine/memory.h"

#include <SDL3/SDL_log.h>
#include <math.h>

typedef struct
{
    float x, y, z;
    float vx, vy, vz;
    float health;
    uint32_t unit_type;
    uint32_t player_id;
} Unit;

typedef struct
{
    Unit* units;
    arena_t* arena;
    uint32_t unit_count;
    uint32_t capacity;
} GameWorld;

// Example task data structures
typedef struct
{
    Unit* units;
    uint32_t start_index;
    uint32_t count;
    float delta_time;
} MovementTaskData;

typedef struct
{
    Unit* units;
    uint32_t start_index;
    uint32_t count;
    GameWorld* world;
} AITaskData;

// AI decision making task
void process_ai_decisions(void* data)
{
    AITaskData* ai_data = (AITaskData*) data;

    for (uint32_t i = 0; i < ai_data->count; i++)
    {
        Unit* unit = &ai_data->units[ai_data->start_index + i];

        // Simple AI: move towards center if health is good
        if (unit->health > 50.0f)
        {
            float dx = 0.0f - unit->x;
            float dy = 0.0f - unit->y;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance > 1.0f)
            {
                unit->vx = (dx / distance) * 10.0f;
                unit->vy = (dy / distance) * 10.0f;
            }
        }
    }
}

// Movement processing task
void process_movement(void* data)
{
    MovementTaskData* move_data = (MovementTaskData*) data;

    for (uint32_t i = 0; i < move_data->count; i++)
    {
        Unit* unit = &move_data->units[move_data->start_index + i];

        // Update position based on velocity
        unit->x += unit->vx * move_data->delta_time;
        unit->y += unit->vy * move_data->delta_time;
        unit->z += unit->vz * move_data->delta_time;

        // Simple boundary checking
        if (unit->x < -100.0f)
            unit->x = -100.0f;
        if (unit->x > 100.0f)
            unit->x = 100.0f;
        if (unit->y < -100.0f)
            unit->y = -100.0f;
        if (unit->y > 100.0f)
            unit->y = 100.0f;
    }
}

// Combat resolution task
void process_combat(void* data)
{
    MovementTaskData* combat_data = (MovementTaskData*) data;

    // Simple combat: reduce health over time
    for (uint32_t i = 0; i < combat_data->count; i++)
    {
        Unit* unit = &combat_data->units[combat_data->start_index + i];
        unit->health -= 1.0f * combat_data->delta_time;
        if (unit->health < 0.0f)
            unit->health = 0.0f;
    }
}

//-------------------------------------------------------------------------------------------------
// Task system integration with game loop
//-------------------------------------------------------------------------------------------------

void wc_game_frame_with_tasks(GameWorld* world, float delta_time)
{
    const uint32_t units_per_task = 256;
    const uint32_t total_units = world->unit_count;
    const uint32_t num_jobs = (total_units + units_per_task - 1) / units_per_task;

    JobHandle* ai_jobs = wc_malloc(sizeof(JobHandle) * num_jobs);
    JobHandle* move_jobs = wc_malloc(sizeof(JobHandle) * num_jobs);
    JobHandle* combat_jobs = wc_malloc(sizeof(JobHandle) * num_jobs);

    // Task data arrays (allocated from frame arena)
    AITaskData* ai_data = arena_alloc(world->arena, num_jobs * sizeof(AITaskData));
    MovementTaskData* movement_data = arena_alloc(world->arena, num_jobs * sizeof(MovementTaskData));
    MovementTaskData* combat_data = arena_alloc(world->arena, num_jobs * sizeof(MovementTaskData));

    // Phase 1: Create AI jobs (no dependencies)
    for (uint32_t i = 0; i < num_jobs; i++)
    {
        uint32_t start_index = i * units_per_task;
        uint32_t count = (start_index + units_per_task > total_units) ? total_units - start_index : units_per_task;

        // Setup AI task data
        ai_data[i].units = world->units;
        ai_data[i].start_index = start_index;
        ai_data[i].count = count;
        ai_data[i].world = world;

        // Create AI task
        // ai_jobs[i] = job_schedule("Unit AI", process_ai_decisions, &ai_data[i], g_job_none);
    }

    // Phase 2: Create movement tasks (depend on AI)
    for (uint32_t i = 0; i < num_jobs; i++)
    {
        uint32_t start_index = i * units_per_task;
        uint32_t count = (start_index + units_per_task > total_units) ? total_units - start_index : units_per_task;

        // Setup movement task data
        movement_data[i].units = world->units;
        movement_data[i].start_index = start_index;
        movement_data[i].count = count;
        movement_data[i].delta_time = delta_time;

        // Create movement task
        // move_jobs[i] = job_schedule("Unit Movement", process_movement, &movement_data[i], ai_jobs[i]);
    }

    // Phase 3: Create combat tasks (depend on movement)
    for (uint32_t i = 0; i < num_jobs; i++)
    {
        uint32_t start_index = i * units_per_task;
        uint32_t count = (start_index + units_per_task > total_units) ? total_units - start_index : units_per_task;

        // Setup combat task data
        combat_data[i].units = world->units;
        combat_data[i].start_index = start_index;
        combat_data[i].count = count;
        combat_data[i].delta_time = delta_time;

        // Create combat task
        // combat_jobs[i] = job_schedule("Unit Combat", process_combat, &combat_data[i], move_jobs[i]);
    }

    wc_free(ai_jobs);
    wc_free(move_jobs);
    wc_free(combat_jobs);
}

//-------------------------------------------------------------------------------------------------
// Example usage and integration
//-------------------------------------------------------------------------------------------------

#define UNIT_COUNT 10000
#define UNITS_PER_TASK 256

static GameWorld g_world;

static void create_test_world()
{
    g_world.capacity = UNIT_COUNT;
    g_world.unit_count = UNIT_COUNT;
    g_world.units = wc_malloc(UNIT_COUNT * sizeof(Unit));

    const uint32_t num_jobs = (g_world.unit_count + UNITS_PER_TASK - 1) / UNITS_PER_TASK;

    const u64 arena_size = sizeof(AITaskData) * num_jobs + sizeof(MovementTaskData) * num_jobs + sizeof(MovementTaskData) * num_jobs;

    g_world.arena = arena_create(arena_size, "Test Arena");

    // Initialize units with random positions
    for (uint32_t i = 0; i < UNIT_COUNT; i++)
    {
        g_world.units[i].x = (float) (SDL_rand(200) - 100);
        g_world.units[i].y = (float) (SDL_rand(200) - 100);
        g_world.units[i].z = 0.0f;
        g_world.units[i].vx = 0.0f;
        g_world.units[i].vy = 0.0f;
        g_world.units[i].vz = 0.0f;
        g_world.units[i].health = 100.0f;
        g_world.units[i].unit_type = SDL_rand(3);
        g_world.units[i].player_id = SDL_rand(4);
    }
}

int war_game_init()
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

    jobs_init(NULL);

    // create_test_world();

    return 0;
}

void war_game_update(const double delta_time)
{
    // wc_game_frame_with_tasks(&g_world, (float) delta_time);

    arena_reset(g_world.arena);
}

void war_game_render(const double interpolant)
{
}

void war_game_quit()
{
    arena_destroy(g_world.arena);
    wc_free(g_world.units);
    jobs_shutdown();
}
