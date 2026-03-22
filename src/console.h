#pragma once

#include "entity.h"
#include "building.h"
#include "resource.h"
#include "mine.h"
#include "map.h"
#include "font.h"
#include "gfx.h"
#include <stdbool.h>
#include <stdint.h>

enum
{
	CON_MAX_INPUT    = 256,
	CON_MAX_OUTPUT   = 128,
	CON_MAX_LINE     = 256,
	CON_MAX_COMMANDS = 64,
	CON_MAX_VARS     = 64,
	CON_MAX_HISTORY  = 32,
	CON_MAX_ARGS     = 16,
};

typedef enum
{
	CON_VAR_INT   = 0,
	CON_VAR_FLOAT = 1,
	CON_VAR_BOOL  = 2,
} con_var_type;

// Pointers to mutable game state, passed to command callbacks
typedef struct
{
	unit_array* units;
	const unit_type* unit_types;
	uint32_t num_unit_types;
	game_map* map;
	building_array* buildings;
	resource_bank* banks;
	mine_array* mines;
} con_context;

// Command callback: return true on success, false to indicate error
typedef bool (*con_command_fn)(const con_context* ctx, int argc, const char** argv);

typedef struct
{
	const font* ui_font;
	uint32_t white_tex;
	con_context ctx;
} con_config;

void con_init(const con_config* config);

// Process input. Returns true if the console is open (game should suppress normal input).
bool con_update(const gfx_input* input);

// Draw the console overlay. Call after HUD, before menu.
void con_draw(float dt, uint32_t screen_w, uint32_t screen_h);

bool con_is_open(void);

// Register a named command
void con_register_command(const char* name, const char* help, con_command_fn fn);

// Register named variables (pointers to existing memory)
void con_register_var_int(const char* name, int32_t* ptr, const char* help);
void con_register_var_float(const char* name, float* ptr, const char* help);
void con_register_var_bool(const char* name, bool* ptr, const char* help);

// Print a line to the console output (printf-style). Callable from anywhere.
void con_print(const char* fmt, ...);

// Print with explicit color
void con_print_color(uint32_t color, const char* fmt, ...);

// Get/set the currently targeted entity index (UINT32_MAX if none)
uint32_t con_get_target(void);
void con_set_target(uint32_t unit_index);
