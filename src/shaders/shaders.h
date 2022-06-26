#pragma once

#define DECLARE_SHADER_SPV(name) \
	extern unsigned char name##_spv[]; \
	extern int name##_spv_size;

DECLARE_SHADER_SPV(sprite_vert);
DECLARE_SHADER_SPV(sprite_frag);

#undef DECLARE_SHADER_SPV