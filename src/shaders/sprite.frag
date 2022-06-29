#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 1, binding = 0) uniform texture2D textures[];
layout (set = 2, binding = 0) uniform sampler point_sampler;

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in flat uint in_texture_index;

layout (location = 0) out vec4 out_color;

void main()
{
    vec4 color = texture(nonuniformEXT(sampler2D(textures[in_texture_index], point_sampler)), in_texcoord);
    //vec4 color = texture(textures[nonuniformEXT(in_texture_index)], in_texcoord);
    out_color = color;
    out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
