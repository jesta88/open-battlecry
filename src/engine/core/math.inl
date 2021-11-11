#pragma once

#include "math.h"

#include <math.h>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#ifdef _WIN32
#include <intrin.h>
#endif

#ifdef __clang__
#include <avxintrin.h>
#endif

static inline vec2_t vec2_add(vec2_t a, vec2_t b);
static inline vec2_t vec2_sub(vec2_t a, vec2_t b);
static inline float vec2_dot(vec2_t a, vec2_t b);
static inline float vec2_cross(vec2_t a, vec2_t b);
static inline vec2_t vec2_mul(vec2_t a, float b);
static inline vec2_t vec2_mul_add(vec2_t a, vec2_t b, float mul);
static inline vec2_t vec2_element_mul(vec2_t a, vec2_t b);
static inline vec2_t vec2_element_div(vec2_t a, vec2_t b);
static inline float vec2_length(vec2_t v);
static inline float vec2_length_squared(vec2_t v);
static inline vec2_t vec2_normalize(vec2_t);
static inline bool vec2_equal(vec2_t a, vec2_t b);
static inline bool vec2_equal_abs_eps(vec2_t a, vec2_t b, float epsilon);
static inline vec2_t vec2_lerp(vec2_t a, vec2_t b, float t);
static inline vec2_t vec2_element_lerp(vec2_t a, vec2_t b, vec2_t t);
static inline vec2_t vec2_min(vec2_t a, vec2_t b);
static inline vec2_t vec2_max(vec2_t a, vec2_t b);
static inline vec2_t vec2_clamp(vec2_t v, vec2_t lo, vec2_t hi);
static inline float vec2_dist_sqr(vec2_t a, vec2_t b);
static inline vec2_t vec2_abs(vec2_t v);

static inline uint32_t uint32_count_bits(uint32_t value);
static inline uint32_t uint32_count_trailing_zero_bits(uint32_t value);
static inline uint32_t uint32_count_leading_zero_bits(uint32_t value);
static inline uint32_t uint32_log(uint32_t value);
static inline uint32_t uint32_round_up_to_power_of_two(uint32_t value);
static inline uint32_t uint32_align_to_power_of_two(uint32_t value, uint32_t p);
static inline uint32_t uint32_div_ceil(uint32_t value, uint32_t d);

static inline uint64_t uint64_align_to_power_of_two(uint64_t value, uint64_t p);
static inline uint64_t uint64_div_ceil(uint64_t value, uint64_t d);

static inline float lerp(float s, float e, float t);
static inline float inverse_lerp(float s, float e, float value);
static inline float remap(float s0, float e0, float s1, float e1, float value);
static inline bool equal_abs_eps(float a, float b, float epsilon);

static inline int16_t rect_right(rect_t rect) { return (int16_t) (rect.x + rect.w); }
static inline int16_t rect_bottom(rect_t rect) { return (int16_t) (rect.y + rect.h); }

static inline rect_t rect_contains_point(rect_t rect, point_t point)
{
    return point.x > rect.x && point.x <= rect.x + rect.w && point.y > rect.y && point.y <= rect.y + rect.h;
}

static inline bool rect_intersects(rect_t a, rect_t b)
{
    const int16_t left = max(a.x, b.x);
    const int16_t top = max(a.y, b.y);
    const int16_t right = min(rect_right(a), rect_right(b));
    const int16_t bottom = min(rect_bottom(a), rect_bottom(b));
    return left <= right && top <= bottom;
}

static inline void rects_intersect(const rect_t a[4], const rect_t b[4], bool results[4])
{
    
}