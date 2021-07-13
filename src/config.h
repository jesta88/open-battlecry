#pragma once

#include "types.h"

void wbConfigLoad(const char* file_name);
void wbConfigSave(const char* file_name);

WbConfig* wbCvarGetInt(const char* name, int32_t value, bool save);

void wbCvarRegisterInt(WbConfig* cvar, const char* name, int32_t int_value, bool save);
void wbCvarRegisterFloat(WbConfig* cvar, const char* name, float float_value, bool save);
void wbCvarRegisterBool(WbConfig* cvar, const char* name, bool bool_value, bool save);
void wbCvarRegisterString(WbConfig* cvar, const char* name, const char* string_value, bool save);
