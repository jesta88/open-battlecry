#include "console.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

static struct
{
	// Config
	const font* ui_font;
	uint32_t white_tex;
	con_context ctx;

	// Open/close
	bool open;

	// Input line
	char input_buf[CON_MAX_INPUT];
	int input_len;
	int input_cursor;

	// Output scrollback (ring buffer)
	char output[CON_MAX_OUTPUT][CON_MAX_LINE];
	uint32_t output_colors[CON_MAX_OUTPUT];
	int output_head;  // next write index (wraps)
	int output_count; // total lines stored (capped at CON_MAX_OUTPUT)
	int scroll_offset;

	// Command history (ring buffer)
	char history[CON_MAX_HISTORY][CON_MAX_INPUT];
	int history_head;
	int history_count;
	int history_browse; // -1 = not browsing

	// Registered commands
	struct
	{
		char name[32];
		char help[128];
		con_command_fn fn;
	} commands[CON_MAX_COMMANDS];
	int command_count;

	// Registered variables
	struct
	{
		char name[32];
		char help[128];
		con_var_type type;
		union
		{
			int32_t* i;
			float* f;
			bool* b;
		} ptr;
	} vars[CON_MAX_VARS];
	int var_count;

	// Entity targeting
	uint32_t target_unit;

	// Visual
	float cursor_blink;
} s;

// ---------------------------------------------------------------------------
// Colors
// ---------------------------------------------------------------------------

enum
{
	COL_DEFAULT = 0xFFCCCCCC,
	COL_INPUT   = 0xFF88FF00,
	COL_ERROR   = 0xFF4444FF,
	COL_WARNING = 0xFF44CCFF,
	COL_INFO    = 0xFF44FF44,
	COL_BG      = 0xD0181010,
	COL_SEP     = 0x80FFFFFF,
	COL_TARGET  = 0xFF00FFFF,
	COL_PROMPT  = 0xFF88FF00,
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void draw_rect(float x, float y, float w, float h, uint32_t color)
{
	float cam_x, cam_y;
	gfx_get_camera(&cam_x, &cam_y);
	gfx_draw_sprite(x - cam_x, y - cam_y, w, h, s.white_tex, color);
}

static void output_push(const char* text, uint32_t color)
{
	strncpy(s.output[s.output_head], text, CON_MAX_LINE - 1);
	s.output[s.output_head][CON_MAX_LINE - 1] = '\0';
	s.output_colors[s.output_head] = color;
	s.output_head = (s.output_head + 1) % CON_MAX_OUTPUT;
	if (s.output_count < CON_MAX_OUTPUT)
		s.output_count++;
	// Auto-scroll to bottom on new output
	s.scroll_offset = 0;
}

static int output_get_index(int line_from_bottom)
{
	// line_from_bottom: 0 = most recent, 1 = second most recent, ...
	int idx = s.output_head - 1 - line_from_bottom;
	if (idx < 0) idx += CON_MAX_OUTPUT;
	return idx;
}

// ---------------------------------------------------------------------------
// Tokenizer
// ---------------------------------------------------------------------------

static int tokenize(const char* input, const char* tokens[CON_MAX_ARGS], char* buf, int buf_size)
{
	int argc = 0;
	int bi = 0;
	const char* p = input;

	while (*p && argc < CON_MAX_ARGS)
	{
		// Skip whitespace
		while (*p == ' ' || *p == '\t') p++;
		if (!*p) break;

		if (*p == '"')
		{
			// Quoted string
			p++;
			tokens[argc] = buf + bi;
			while (*p && *p != '"' && bi < buf_size - 1)
				buf[bi++] = *p++;
			buf[bi++] = '\0';
			if (*p == '"') p++;
			argc++;
		}
		else
		{
			// Unquoted token
			tokens[argc] = buf + bi;
			while (*p && *p != ' ' && *p != '\t' && bi < buf_size - 1)
				buf[bi++] = *p++;
			buf[bi++] = '\0';
			argc++;
		}
	}
	return argc;
}

// ---------------------------------------------------------------------------
// History
// ---------------------------------------------------------------------------

static void history_push(const char* text)
{
	strncpy(s.history[s.history_head], text, CON_MAX_INPUT - 1);
	s.history[s.history_head][CON_MAX_INPUT - 1] = '\0';
	s.history_head = (s.history_head + 1) % CON_MAX_HISTORY;
	if (s.history_count < CON_MAX_HISTORY)
		s.history_count++;
}

static void history_set_input(int browse_idx)
{
	int idx = s.history_head - 1 - browse_idx;
	if (idx < 0) idx += CON_MAX_HISTORY;
	strncpy(s.input_buf, s.history[idx], CON_MAX_INPUT - 1);
	s.input_buf[CON_MAX_INPUT - 1] = '\0';
	s.input_len = (int)strlen(s.input_buf);
	s.input_cursor = s.input_len;
}

// ---------------------------------------------------------------------------
// Command execution
// ---------------------------------------------------------------------------

static void con_execute(const char* input)
{
	// Echo the command
	char echo[CON_MAX_LINE];
	snprintf(echo, sizeof(echo), "> %s", input);
	output_push(echo, COL_PROMPT);

	// Tokenize
	const char* argv[CON_MAX_ARGS];
	char token_buf[CON_MAX_INPUT];
	int argc = tokenize(input, argv, token_buf, sizeof(token_buf));
	if (argc == 0) return;

	// Look up command
	for (int i = 0; i < s.command_count; i++)
	{
		if (_stricmp(s.commands[i].name, argv[0]) == 0)
		{
			if (!s.commands[i].fn(&s.ctx, argc, argv))
				con_print_color(COL_ERROR, "Command failed: %s", argv[0]);
			return;
		}
	}

	// Look up variable
	for (int i = 0; i < s.var_count; i++)
	{
		if (_stricmp(s.vars[i].name, argv[0]) == 0)
		{
			if (argc == 1)
			{
				// Print current value
				switch (s.vars[i].type)
				{
					case CON_VAR_INT:   con_print("%s = %d", s.vars[i].name, *s.vars[i].ptr.i); break;
					case CON_VAR_FLOAT: con_print("%s = %.2f", s.vars[i].name, (double)*s.vars[i].ptr.f); break;
					case CON_VAR_BOOL:  con_print("%s = %s", s.vars[i].name, *s.vars[i].ptr.b ? "true" : "false"); break;
				}
			}
			else
			{
				// Set value
				switch (s.vars[i].type)
				{
					case CON_VAR_INT:
						*s.vars[i].ptr.i = atoi(argv[1]);
						con_print("%s = %d", s.vars[i].name, *s.vars[i].ptr.i);
						break;
					case CON_VAR_FLOAT:
						*s.vars[i].ptr.f = (float)atof(argv[1]);
						con_print("%s = %.2f", s.vars[i].name, (double)*s.vars[i].ptr.f);
						break;
					case CON_VAR_BOOL:
						*s.vars[i].ptr.b = (_stricmp(argv[1], "true") == 0 || _stricmp(argv[1], "1") == 0);
						con_print("%s = %s", s.vars[i].name, *s.vars[i].ptr.b ? "true" : "false");
						break;
				}
			}
			return;
		}
	}

	con_print_color(COL_ERROR, "Unknown command: %s", argv[0]);
}

// ---------------------------------------------------------------------------
// Entity picking
// ---------------------------------------------------------------------------

static void pick_entity_under_cursor(const gfx_input* input)
{
	float cam_x, cam_y;
	gfx_get_camera(&cam_x, &cam_y);
	float wx = input->mouse_x - cam_x;
	float wy = input->mouse_y - cam_y;

	for (uint32_t i = 0; i < s.ctx.units->count; i++)
	{
		const unit* u = &s.ctx.units->units[i];
		if (u->state == UNIT_STATE_DYING) continue;

		const ani_type* at = &u->type->ani.types[u->anim.anim_type];
		float left   = u->x - (float)at->origin_x;
		float top    = u->y - (float)at->origin_y;
		float right  = left + (float)at->width;
		float bottom = top + (float)at->height;

		if (wx >= left && wx <= right && wy >= top && wy <= bottom)
		{
			s.target_unit = i;
			con_print_color(COL_TARGET, "Target: %s #%u (team %u, hp %d/%d)",
			                u->type->base_code, i, u->team, u->health, u->type->max_health);
			return;
		}
	}

	s.target_unit = UINT32_MAX;
	con_print("No target");
}

// ---------------------------------------------------------------------------
// Built-in commands
// ---------------------------------------------------------------------------

static bool cmd_help(const con_context* ctx, int argc, const char** argv)
{
	(void)ctx;
	if (argc > 1)
	{
		for (int i = 0; i < s.command_count; i++)
		{
			if (_stricmp(s.commands[i].name, argv[1]) == 0)
			{
				con_print("%s: %s", s.commands[i].name, s.commands[i].help);
				return true;
			}
		}
		con_print_color(COL_ERROR, "Unknown command: %s", argv[1]);
		return true;
	}

	con_print_color(COL_INFO, "--- Commands ---");
	for (int i = 0; i < s.command_count; i++)
		con_print("  %-16s %s", s.commands[i].name, s.commands[i].help);
	con_print_color(COL_INFO, "--- Variables ---");
	for (int i = 0; i < s.var_count; i++)
		con_print("  %-16s %s", s.vars[i].name, s.vars[i].help);
	return true;
}

static bool cmd_clear(const con_context* ctx, int argc, const char** argv)
{
	(void)ctx; (void)argc; (void)argv;
	s.output_count = 0;
	s.output_head = 0;
	s.scroll_offset = 0;
	return true;
}

static bool cmd_list_vars(const con_context* ctx, int argc, const char** argv)
{
	(void)ctx; (void)argc; (void)argv;
	for (int i = 0; i < s.var_count; i++)
	{
		switch (s.vars[i].type)
		{
			case CON_VAR_INT:   con_print("  %-16s = %d", s.vars[i].name, *s.vars[i].ptr.i); break;
			case CON_VAR_FLOAT: con_print("  %-16s = %.2f", s.vars[i].name, (double)*s.vars[i].ptr.f); break;
			case CON_VAR_BOOL:  con_print("  %-16s = %s", s.vars[i].name, *s.vars[i].ptr.b ? "true" : "false"); break;
		}
	}
	return true;
}

static bool cmd_inspect(const con_context* ctx, int argc, const char** argv)
{
	(void)argc; (void)argv;
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return true;
	}
	const unit* u = &ctx->units->units[s.target_unit];
	con_print("--- Unit #%u ---", s.target_unit);
	con_print("  Type:   %s", u->type->base_code);
	con_print("  Team:   %u", u->team);
	con_print("  Pos:    %.1f, %.1f", (double)u->x, (double)u->y);
	con_print("  HP:     %d / %d", u->health, u->type->max_health);
	con_print("  Speed:  %.1f", (double)u->speed);
	const char* state_names[] = {"IDLE", "WALKING", "FIGHTING", "DYING"};
	con_print("  State:  %s", u->state < 4 ? state_names[u->state] : "?");
	con_print("  Armor:  %d", u->type->armor);
	con_print("  Damage: %d", u->type->attack_damage);
	return true;
}

static bool cmd_kill(const con_context* ctx, int argc, const char** argv)
{
	(void)argc; (void)argv;
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	u->health = 0;
	u->state = UNIT_STATE_DYING;
	con_print("Killed %s #%u", u->type->base_code, s.target_unit);
	return true;
}

static bool cmd_heal(const con_context* ctx, int argc, const char** argv)
{
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	int amt = (argc > 1) ? atoi(argv[1]) : u->type->max_health;
	u->health += (int16_t)amt;
	if (u->health > u->type->max_health)
		u->health = u->type->max_health;
	con_print("Healed %s #%u to %d/%d", u->type->base_code, s.target_unit, u->health, u->type->max_health);
	return true;
}

static bool cmd_damage(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 2)
	{
		con_print_color(COL_ERROR, "Usage: damage <amount>");
		return false;
	}
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	int amt = atoi(argv[1]);
	u->health -= (int16_t)amt;
	if (u->health < 0) u->health = 0;
	con_print("Damaged %s #%u: hp=%d/%d", u->type->base_code, s.target_unit, u->health, u->type->max_health);
	if (u->health == 0)
	{
		u->state = UNIT_STATE_DYING;
		con_print("  -> Unit died");
	}
	return true;
}

static bool cmd_spawn(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 4)
	{
		con_print_color(COL_ERROR, "Usage: spawn <type_idx> <x> <y> [team]");
		return false;
	}
	int type_idx = atoi(argv[1]);
	if (type_idx < 0 || (uint32_t)type_idx >= ctx->num_unit_types)
	{
		con_print_color(COL_ERROR, "Invalid type index %d (max %u)", type_idx, ctx->num_unit_types - 1);
		return false;
	}
	float x = (float)atof(argv[2]);
	float y = (float)atof(argv[3]);
	uint8_t team = (argc > 4) ? (uint8_t)atoi(argv[4]) : 0;
	uint32_t idx = unit_spawn(ctx->units, &ctx->unit_types[type_idx], x, y, team);
	if (idx == UINT32_MAX)
	{
		con_print_color(COL_ERROR, "Failed to spawn (unit limit reached)");
		return false;
	}
	con_print("Spawned %s #%u at (%.0f, %.0f) team %u",
	          ctx->unit_types[type_idx].base_code, idx, (double)x, (double)y, team);
	return true;
}

static bool cmd_tp(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 3)
	{
		con_print_color(COL_ERROR, "Usage: tp <x> <y>");
		return false;
	}
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	u->x = (float)atof(argv[1]);
	u->y = (float)atof(argv[2]);
	u->state = UNIT_STATE_IDLE;
	u->path.length = 0;
	con_print("Teleported %s #%u to (%.0f, %.0f)", u->type->base_code, s.target_unit, (double)u->x, (double)u->y);
	return true;
}

static bool cmd_set_team(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 2)
	{
		con_print_color(COL_ERROR, "Usage: set_team <team>");
		return false;
	}
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	u->team = (uint8_t)atoi(argv[1]);
	con_print("Set %s #%u team to %u", u->type->base_code, s.target_unit, u->team);
	return true;
}

static bool cmd_give(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 3)
	{
		con_print_color(COL_ERROR, "Usage: give <gold|metal|crystal|stone> <amount> [team]");
		return false;
	}
	const char* res_names[] = {"gold", "metal", "crystal", "stone"};
	int res = -1;
	for (int i = 0; i < RES_COUNT; i++)
	{
		if (_stricmp(argv[1], res_names[i]) == 0)
		{
			res = i;
			break;
		}
	}
	if (res < 0)
	{
		con_print_color(COL_ERROR, "Unknown resource: %s", argv[1]);
		return false;
	}
	int32_t amount = atoi(argv[2]);
	int team = (argc > 3) ? atoi(argv[3]) : 0;
	if (team < 0 || team >= MAX_TEAMS)
	{
		con_print_color(COL_ERROR, "Invalid team %d", team);
		return false;
	}
	resource_add(&ctx->banks[team], (uint8_t)res, amount);
	con_print("Gave %d %s to team %d", amount, res_names[res], team);
	return true;
}

static bool cmd_list_units(const con_context* ctx, int argc, const char** argv)
{
	int filter_team = -1;
	if (argc > 1) filter_team = atoi(argv[1]);

	int count = 0;
	for (uint32_t i = 0; i < ctx->units->count; i++)
	{
		const unit* u = &ctx->units->units[i];
		if (u->state == UNIT_STATE_DYING) continue;
		if (filter_team >= 0 && u->team != (uint8_t)filter_team) continue;
		const char* states[] = {"IDLE", "WALK", "FIGHT", "DYING"};
		con_print("  #%-3u %-6s team=%u hp=%d/%d state=%s pos=(%.0f,%.0f)",
		          i, u->type->base_code, u->team, u->health, u->type->max_health,
		          u->state < 4 ? states[u->state] : "?",
		          (double)u->x, (double)u->y);
		count++;
	}
	con_print("%d unit(s)", count);
	return true;
}

static bool cmd_target(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 2)
	{
		if (s.target_unit == UINT32_MAX)
			con_print("No target");
		else
		{
			const unit* u = &ctx->units->units[s.target_unit];
			con_print("Target: %s #%u", u->type->base_code, s.target_unit);
		}
		return true;
	}
	if (_stricmp(argv[1], "clear") == 0)
	{
		s.target_unit = UINT32_MAX;
		con_print("Target cleared");
		return true;
	}
	uint32_t idx = (uint32_t)atoi(argv[1]);
	if (idx >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "Invalid index %u (count=%u)", idx, ctx->units->count);
		return false;
	}
	s.target_unit = idx;
	const unit* u = &ctx->units->units[idx];
	con_print_color(COL_TARGET, "Target: %s #%u (team %u, hp %d/%d)",
	                u->type->base_code, idx, u->team, u->health, u->type->max_health);
	return true;
}

static bool cmd_cam(const con_context* ctx, int argc, const char** argv)
{
	(void)ctx;
	if (argc < 3)
	{
		float cx, cy;
		gfx_get_camera(&cx, &cy);
		con_print("Camera: %.0f, %.0f", (double)cx, (double)cy);
		return true;
	}
	float x = (float)atof(argv[1]);
	float y = (float)atof(argv[2]);
	gfx_set_camera(x, y);
	con_print("Camera set to %.0f, %.0f", (double)x, (double)y);
	return true;
}

static bool cmd_set_speed(const con_context* ctx, int argc, const char** argv)
{
	if (argc < 2)
	{
		con_print_color(COL_ERROR, "Usage: set_speed <speed>");
		return false;
	}
	if (s.target_unit == UINT32_MAX || s.target_unit >= ctx->units->count)
	{
		con_print_color(COL_ERROR, "No target selected");
		return false;
	}
	unit* u = &ctx->units->units[s.target_unit];
	u->speed = (float)atof(argv[1]);
	con_print("Set %s #%u speed to %.1f", u->type->base_code, s.target_unit, (double)u->speed);
	return true;
}

static bool cmd_list_types(const con_context* ctx, int argc, const char** argv)
{
	(void)argc; (void)argv;
	for (uint32_t i = 0; i < ctx->num_unit_types; i++)
	{
		const unit_type* t = &ctx->unit_types[i];
		con_print("  [%u] %-6s hp=%d dmg=%d armor=%d", i, t->base_code, t->max_health, t->attack_damage, t->armor);
	}
	con_print("%u type(s)", ctx->num_unit_types);
	return true;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void con_init(const con_config* config)
{
	memset(&s, 0, sizeof(s));
	s.ui_font = config->ui_font;
	s.white_tex = config->white_tex;
	s.ctx = config->ctx;
	s.target_unit = UINT32_MAX;
	s.history_browse = -1;

	// Register built-in commands
	con_register_command("help",       "List commands, or help <cmd> for details", cmd_help);
	con_register_command("clear",      "Clear console output",                     cmd_clear);
	con_register_command("list_vars",  "List all registered variables",            cmd_list_vars);
	con_register_command("inspect",    "Print targeted unit's state",              cmd_inspect);
	con_register_command("kill",       "Kill targeted unit",                       cmd_kill);
	con_register_command("heal",       "Heal targeted unit [amount]",              cmd_heal);
	con_register_command("damage",     "Damage targeted unit <amount>",            cmd_damage);
	con_register_command("spawn",      "Spawn <type_idx> <x> <y> [team]",         cmd_spawn);
	con_register_command("tp",         "Teleport targeted unit <x> <y>",           cmd_tp);
	con_register_command("set_team",   "Change targeted unit's team <team>",       cmd_set_team);
	con_register_command("set_speed",  "Set targeted unit's speed <speed>",        cmd_set_speed);
	con_register_command("give",       "Give <gold|metal|crystal|stone> <amt> [team]", cmd_give);
	con_register_command("list_units", "List living units [team]",                 cmd_list_units);
	con_register_command("list_types", "List loaded unit types",                   cmd_list_types);
	con_register_command("target",     "Target <index> or target clear",           cmd_target);
	con_register_command("cam",        "Get/set camera <x> <y>",                   cmd_cam);

	con_print_color(COL_INFO, "Dev console ready. Type 'help' for commands.");
}

bool con_update(const gfx_input* input)
{
	// Toggle
	if (input->key_backtick_pressed)
	{
		s.open = !s.open;
		if (s.open)
			gfx_start_text_input();
		else
			gfx_stop_text_input();
		return s.open;
	}

	if (!s.open) return false;

	// --- Console is open: consume all input ---

	// Entity targeting on left click
	if (input->mouse_left_pressed)
		pick_entity_under_cursor(input);

	// Append text input (filter backtick)
	for (uint8_t i = 0; i < input->text_input_len; i++)
	{
		char ch = input->text_input[i];
		if (ch == '`') continue;
		if (ch < 32) continue;
		if (s.input_len < CON_MAX_INPUT - 1)
		{
			memmove(&s.input_buf[s.input_cursor + 1],
			        &s.input_buf[s.input_cursor],
			        s.input_len - s.input_cursor);
			s.input_buf[s.input_cursor] = ch;
			s.input_cursor++;
			s.input_len++;
			s.input_buf[s.input_len] = '\0';
		}
	}

	// Backspace
	if (input->key_backspace_pressed && s.input_cursor > 0)
	{
		memmove(&s.input_buf[s.input_cursor - 1],
		        &s.input_buf[s.input_cursor],
		        s.input_len - s.input_cursor);
		s.input_cursor--;
		s.input_len--;
		s.input_buf[s.input_len] = '\0';
	}

	// Delete
	if (input->key_delete_pressed && s.input_cursor < s.input_len)
	{
		memmove(&s.input_buf[s.input_cursor],
		        &s.input_buf[s.input_cursor + 1],
		        s.input_len - s.input_cursor - 1);
		s.input_len--;
		s.input_buf[s.input_len] = '\0';
	}

	// Cursor movement
	if (input->key_left_pressed && s.input_cursor > 0) s.input_cursor--;
	if (input->key_right_pressed && s.input_cursor < s.input_len) s.input_cursor++;
	if (input->key_home_pressed) s.input_cursor = 0;
	if (input->key_end_pressed) s.input_cursor = s.input_len;

	// Enter: execute
	if (input->key_enter_pressed && s.input_len > 0)
	{
		con_execute(s.input_buf);
		history_push(s.input_buf);
		s.input_buf[0] = '\0';
		s.input_len = 0;
		s.input_cursor = 0;
		s.history_browse = -1;
	}

	// History navigation
	if (input->key_up_pressed)
	{
		if (s.history_browse < s.history_count - 1)
		{
			s.history_browse++;
			history_set_input(s.history_browse);
		}
	}
	if (input->key_down_pressed)
	{
		if (s.history_browse > 0)
		{
			s.history_browse--;
			history_set_input(s.history_browse);
		}
		else if (s.history_browse == 0)
		{
			s.history_browse = -1;
			s.input_buf[0] = '\0';
			s.input_len = 0;
			s.input_cursor = 0;
		}
	}

	// Scroll output
	if (input->key_page_up_pressed)
		s.scroll_offset += 5;
	if (input->key_page_down_pressed)
		s.scroll_offset -= 5;
	if (s.scroll_offset < 0) s.scroll_offset = 0;
	int max_scroll = s.output_count > 0 ? s.output_count - 1 : 0;
	if (s.scroll_offset > max_scroll) s.scroll_offset = max_scroll;

	// Escape closes console
	if (input->key_escape_pressed)
	{
		s.open = false;
		gfx_stop_text_input();
	}

	return true;
}

void con_draw(float dt, uint32_t screen_w, uint32_t screen_h)
{
	if (!s.open) return;

	s.cursor_blink += dt;
	if (s.cursor_blink > 1.0f) s.cursor_blink -= 1.0f;

	float sh = (float)screen_h;
	float sw = (float)screen_w;
	float con_height = sh * 0.4f;
	float con_top = sh - con_height;
	float padding = 6.0f;
	float line_h = s.ui_font->line_height;
	float input_area_h = line_h + padding * 2.0f;
	float sep_y = sh - input_area_h;
	float output_area_h = con_height - input_area_h;
	int visible_lines = (int)(output_area_h / line_h);
	if (visible_lines < 1) visible_lines = 1;

	// Background
	draw_rect(0, con_top, sw, con_height, COL_BG);

	// Separator line
	draw_rect(0, sep_y, sw, 1.0f, COL_SEP);

	// Output lines (bottom-up from separator)
	for (int i = 0; i < visible_lines && i + s.scroll_offset < s.output_count; i++)
	{
		int line_idx = output_get_index(i + s.scroll_offset);
		float y = sep_y - (float)(i + 1) * line_h;
		if (y < con_top) break;
		font_draw_text(s.ui_font, padding, y, s.output[line_idx], s.output_colors[line_idx]);
	}

	// Input line
	float input_y = sep_y + padding;
	float cursor_x = font_draw_text(s.ui_font, padding, input_y, "> ", COL_PROMPT);

	// Draw text before cursor, then cursor, then text after cursor
	if (s.input_len > 0)
	{
		// Text before cursor
		if (s.input_cursor > 0)
		{
			char before[CON_MAX_INPUT];
			memcpy(before, s.input_buf, s.input_cursor);
			before[s.input_cursor] = '\0';
			cursor_x = font_draw_text(s.ui_font, cursor_x, input_y, before, COL_DEFAULT);
		}

		// Text after cursor
		if (s.input_cursor < s.input_len)
		{
			font_draw_text(s.ui_font, cursor_x, input_y, &s.input_buf[s.input_cursor], COL_DEFAULT);
		}
	}

	// Blinking cursor
	if (s.cursor_blink < 0.5f)
	{
		draw_rect(cursor_x, input_y, 2.0f, line_h, COL_DEFAULT);
	}

	// Target indicator (bottom-right)
	if (s.target_unit != UINT32_MAX && s.target_unit < s.ctx.units->count)
	{
		const unit* u = &s.ctx.units->units[s.target_unit];
		char target_str[64];
		snprintf(target_str, sizeof(target_str), "[%s #%u]", u->type->base_code, s.target_unit);
		float tw = font_measure_text(s.ui_font, target_str);
		font_draw_text(s.ui_font, sw - tw - padding, input_y, target_str, COL_TARGET);
	}

	// Scroll indicator
	if (s.scroll_offset > 0)
	{
		char scroll_str[32];
		snprintf(scroll_str, sizeof(scroll_str), "-- scroll: +%d --", s.scroll_offset);
		float scroll_tw = font_measure_text(s.ui_font, scroll_str);
		font_draw_text(s.ui_font, (sw - scroll_tw) * 0.5f, con_top + 2.0f, scroll_str, COL_SEP);
	}

	// Draw highlight on targeted unit in the world
	if (s.target_unit != UINT32_MAX && s.target_unit < s.ctx.units->count)
	{
		const unit* u = &s.ctx.units->units[s.target_unit];
		if (u->state != UNIT_STATE_DYING)
		{
			const ani_type* at = &u->type->ani.types[u->anim.anim_type];
			float left   = u->x - (float)at->origin_x;
			float top    = u->y - (float)at->origin_y;
			float w      = (float)at->width;
			float h      = (float)at->height;
			float t      = 1.0f;
			// Draw rectangle outline (4 edges) in world space
			gfx_draw_sprite(left, top, w, t, s.white_tex, COL_TARGET);
			gfx_draw_sprite(left, top + h - t, w, t, s.white_tex, COL_TARGET);
			gfx_draw_sprite(left, top, t, h, s.white_tex, COL_TARGET);
			gfx_draw_sprite(left + w - t, top, t, h, s.white_tex, COL_TARGET);
		}
	}
}

bool con_is_open(void)
{
	return s.open;
}

void con_register_command(const char* name, const char* help, con_command_fn fn)
{
	if (s.command_count >= CON_MAX_COMMANDS) return;
	strncpy(s.commands[s.command_count].name, name, 31);
	s.commands[s.command_count].name[31] = '\0';
	strncpy(s.commands[s.command_count].help, help, 127);
	s.commands[s.command_count].help[127] = '\0';
	s.commands[s.command_count].fn = fn;
	s.command_count++;
}

void con_register_var_int(const char* name, int32_t* ptr, const char* help)
{
	if (s.var_count >= CON_MAX_VARS) return;
	strncpy(s.vars[s.var_count].name, name, 31);
	s.vars[s.var_count].name[31] = '\0';
	strncpy(s.vars[s.var_count].help, help, 127);
	s.vars[s.var_count].help[127] = '\0';
	s.vars[s.var_count].type = CON_VAR_INT;
	s.vars[s.var_count].ptr.i = ptr;
	s.var_count++;
}

void con_register_var_float(const char* name, float* ptr, const char* help)
{
	if (s.var_count >= CON_MAX_VARS) return;
	strncpy(s.vars[s.var_count].name, name, 31);
	s.vars[s.var_count].name[31] = '\0';
	strncpy(s.vars[s.var_count].help, help, 127);
	s.vars[s.var_count].help[127] = '\0';
	s.vars[s.var_count].type = CON_VAR_FLOAT;
	s.vars[s.var_count].ptr.f = ptr;
	s.var_count++;
}

void con_register_var_bool(const char* name, bool* ptr, const char* help)
{
	if (s.var_count >= CON_MAX_VARS) return;
	strncpy(s.vars[s.var_count].name, name, 31);
	s.vars[s.var_count].name[31] = '\0';
	strncpy(s.vars[s.var_count].help, help, 127);
	s.vars[s.var_count].help[127] = '\0';
	s.vars[s.var_count].type = CON_VAR_BOOL;
	s.vars[s.var_count].ptr.b = ptr;
	s.var_count++;
}

void con_print(const char* fmt, ...)
{
	char buf[CON_MAX_LINE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	output_push(buf, COL_DEFAULT);
}

void con_print_color(uint32_t color, const char* fmt, ...)
{
	char buf[CON_MAX_LINE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	output_push(buf, color);
}

uint32_t con_get_target(void)
{
	return s.target_unit;
}

void con_set_target(uint32_t unit_index)
{
	s.target_unit = unit_index;
}
