#version 460

#define SET_GLOBAL 0
#define SET_PER_FRAME 1
#define SET_PER_BATCH 2
#define SET_PER_DRAW 3

layout(push_constant) uniform PushConstants
{
    mat4x3 view_projection;
} push_constants;

struct Transform
{
    vec2 position;
};

layout(set = 1, binding = 0) readonly buffer TransformBuffer
{
    Transform transforms[];
};

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) flat out uint out_sprite_index;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec2 quad_positions[6] = vec2[6](
    vec2(-0.5f, -0.5f),
    vec2( 0.5f, -0.5f),
    vec2( 0.5f,  0.5f),
    vec2( 0.5f,  0.5f),
    vec2(-0.5f,  0.5f),
    vec2(-0.5f, -0.5f)
);

const vec2 quad_texcoords[6] = vec2[6](
    vec2(0.0f, 1.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f),
    vec2(0.0f, 1.0f)
);

void main()
{
    const uint sprite_index = gl_VertexIndex / 6;
    const uint vertex_index = gl_VertexIndex % 6;

    Transform transform = transforms[sprite_index];

    out_texcoord = quad_texcoords[vertex_index];
    out_sprite_index = sprite_index;

    vec4 world_position = vec4(quad_positions[vertex_index], 0, 1);
    world_position.xy += transform.position;
    gl_Position = vec4(push_constants.view_projection * world_position, 1);
}