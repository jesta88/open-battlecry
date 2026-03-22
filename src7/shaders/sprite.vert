#version 460

layout(push_constant) uniform PushConstants
{
    vec2 window_size;
    vec2 camera_position;
} push_constants;

struct Sprite
{
    vec2 position;
    vec2 size;
    uint texture_index;
    uint color;
    uint _pad0;
    uint _pad1;
};

layout(set = 0, binding = 0) readonly buffer SpriteBuffer
{
    Sprite sprites[];
};

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_texcoord;
layout (location = 2) out flat uint out_texture_index;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec2 vertices[6] = vec2[6](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 1),
    vec2(1, 0)
);

vec4 unpack_color(uint color)
{
    return vec4(color & 0xffu, (color >> 8) & 0xffu, (color >> 16) & 0xffu, (color >> 24) & 0xffu) / 255.f;
}

void main()
{
    const uint vertex_index = gl_VertexIndex % 6;
    const uint sprite_index = gl_VertexIndex / 6;

    const vec2 vertex = vertices[vertex_index];
    const Sprite sprite = sprites[sprite_index];

    vec2 position = vertex * sprite.size + sprite.position + push_constants.camera_position;
    float x = position.x / push_constants.window_size.x * 2 - 1;
    float y = position.y / push_constants.window_size.y * 2 - 1;

    gl_Position = vec4(x, y, 0, 1);
    
    out_color = unpack_color(sprite.color);
    out_texcoord = vertex;
    out_texture_index = sprite.texture_index;
}