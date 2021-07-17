#pragma once

#include "sokol_time.h"

static inline uint64_t time_now(void) { return stm_now(); }
static inline uint64_t time_since(uint64_t* last_tick) { return stm_laptime(last_tick); }

static inline double time_sec(uint64_t ticks) { return stm_sec(ticks); }
static inline double time_ms(uint64_t ticks) { return stm_ms(ticks); }
static inline double time_us(uint64_t ticks) { return stm_us(ticks); }
static inline double time_ns(uint64_t ticks) { return stm_ns(ticks); }
