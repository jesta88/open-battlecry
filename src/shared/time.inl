#pragma once

#include "sokol_time.h"

static inline uint64_t s_get_tick(void) { return stm_now(); }
static inline uint64_t s_ticks_since(uint64_t* last_tick) { return stm_laptime(last_tick); }

static inline double s_get_sec(uint64_t ticks) { return stm_sec(ticks); }
static inline double s_get_ms(uint64_t ticks) { return stm_ms(ticks); }
static inline double s_get_us(uint64_t ticks) { return stm_us(ticks); }
static inline double s_get_ns(uint64_t ticks) { return stm_ns(ticks); }
