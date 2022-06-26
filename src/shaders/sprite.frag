#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 0) uniform sampler point_sampler;
layout (set = 0, binding = 1) uniform texture2D textures[];

struct Sprite
{
    uint tint;
    uint texture_index;
};

layout (set = 1, binding = 0) readonly buffer SpriteBuffer
{
    Sprite sprites[];
};

layout (location = 0) in vec2 in_texcoord;
layout (location = 1) flat in uint in_sprite_index;

layout (location = 0) out vec4 out_color;

vec4 unpack_color(uint color)
{
    return vec4(color & 0xffu, (color >> 8) & 0xffu, (color >> 16) & 0xffu, (color >> 24) & 0xffu) / 255.f;
}

void main()
{
    Sprite sprite = sprites[in_sprite_index];
    vec4 color = texture(nonuniformEXT(sampler2D(textures[sprite.texture_index], point_sampler)), in_texcoord);
    //vec4 color = texture(textures[nonuniformEXT(in_texture_index)], in_texcoord);
    out_color = color;
    out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
