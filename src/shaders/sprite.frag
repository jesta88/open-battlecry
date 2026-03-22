#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec2 in_uv;
layout(location = 1) in flat uint in_texture_index;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec4 out_color;

void main() {
    vec4 tex_color = texture(textures[nonuniformEXT(in_texture_index)], in_uv);
    out_color = tex_color * in_color;
}
