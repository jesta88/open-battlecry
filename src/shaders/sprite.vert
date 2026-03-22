#version 460

layout(push_constant) uniform PushConstants {
    vec2 window_size;
    vec2 camera_position;
} pc;

struct Sprite {
    vec2 position;
    vec2 size;
    vec2 uv_offset;
    vec2 uv_scale;
    uint texture_index;
    uint color;
    uint _pad0, _pad1;
};

layout(set = 0, binding = 0) readonly buffer SpriteBuffer {
    Sprite sprites[];
};

layout(location = 0) out vec2 out_uv;
layout(location = 1) out flat uint out_texture_index;
layout(location = 2) out vec4 out_color;

const vec2 vertices[6] = vec2[6](
    vec2(0, 0), vec2(0, 1), vec2(1, 1),
    vec2(0, 0), vec2(1, 1), vec2(1, 0)
);

vec4 unpack_color(uint c) {
    return vec4(c & 0xFFu, (c >> 8) & 0xFFu, (c >> 16) & 0xFFu, (c >> 24) & 0xFFu) / 255.0;
}

void main() {
    uint vertex_index = gl_VertexIndex % 6;
    uint sprite_index = gl_VertexIndex / 6;

    vec2 v = vertices[vertex_index];
    Sprite s = sprites[sprite_index];

    vec2 pixel_pos = v * s.size + s.position + pc.camera_position;
    vec2 ndc = pixel_pos / pc.window_size * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);
    out_uv = v * s.uv_scale + s.uv_offset;
    out_texture_index = s.texture_index;
    out_color = unpack_color(s.color);
}
