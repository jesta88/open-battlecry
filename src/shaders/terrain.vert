#version 460

layout(push_constant) uniform PushConstants {
    mat4x3 view_projection;
} push_constants;

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_color;
layout (location = 3) in uint in_texture_index;

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) flat out uint out_texture_index;

const vec2 quad_positions[6] = vec2[6](
vec2(-0.5f, -0.5f),
vec2( 0.5f, -0.5f),
vec2( 0.5f,  0.5f),
vec2( 0.5f,  0.5f),
vec2(-0.5f,  0.5f),
vec2(-0.5f, -0.5f));

const vec2 quad_texcoords[6] = vec2[6](
vec2(0.0f, 1.0f),
vec2(1.0f, 1.0f),
vec2(1.0f, 0.0f),
vec2(1.0f, 0.0f),
vec2(0.0f, 0.0f),
vec2(0.0f, 1.0f));

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    const uint vertex_index = gl_VertexIndex % 6;

    out_texcoord = quad_texcoords[vertex_index];
    out_texture_index = in_texture_index;

    vec4 world_position = vec4(quad_positions[vertex_index], 0, 1);
    world_position.xy += in_position;
    gl_Position = vec4(push_constants.view_projection * world_position, 1);
}