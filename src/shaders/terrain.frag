#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 0) uniform sampler2D textures[];

layout (location = 0) in vec2 in_texcoord;
layout (location = 1) flat in uint in_texture_index;

layout (location = 0) out vec4 out_color;

vec4 unpack_color(uint color)
{
    return vec4(color & 0xffu, (color >> 8) & 0xffu, (color >> 16) & 0xffu, (color >> 24) & 0xffu) / 255.f;
}

void main()
{
    vec4 color = texture(textures[nonuniformEXT(in_texture_index)], in_texcoord);
    out_color = color;
}