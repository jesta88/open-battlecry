#version 450
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
layout(triangles, max_vertices = 64, max_primitives = 126) out;

// Descriptor bindings
layout(binding = 0, set = 0) readonly buffer TransformBuffer {
    mat4 transforms[];
} transformBuffer;

layout(binding = 1, set = 0) readonly buffer VisibilityBuffer {
    uint visibility[];
} visibilityBuffer;

// Push constants or uniforms
layout(push_constant) uniform PushConstants {
    mat4 viewProj;
} pc;

// Output to fragment shader
layout(location = 0) out vec3 fragColor[];

void main()
{
    uint meshletIndex = gl_WorkGroupID.x;

    // Check visibility
    if (visibilityBuffer.visibility[meshletIndex] == 0) {
        SetMeshOutputsEXT(0, 0);
        return;
    }

    // Simple cube generation for demonstration
    vec3 vertices[8] = vec3[](
        vec3(-0.5, -0.5, -0.5),
        vec3( 0.5, -0.5, -0.5),
        vec3( 0.5,  0.5, -0.5),
        vec3(-0.5,  0.5, -0.5),
        vec3(-0.5, -0.5,  0.5),
        vec3( 0.5, -0.5,  0.5),
        vec3( 0.5,  0.5,  0.5),
        vec3(-0.5,  0.5,  0.5)
    );

    // Define cube faces (12 triangles)
    uvec3 indices[12] = uvec3[](
        uvec3(0, 1, 2), uvec3(2, 3, 0),  // Front
        uvec3(1, 5, 6), uvec3(6, 2, 1),  // Right
        uvec3(5, 4, 7), uvec3(7, 6, 5),  // Back
        uvec3(4, 0, 3), uvec3(3, 7, 4),  // Left
        uvec3(3, 2, 6), uvec3(6, 7, 3),  // Top
        uvec3(4, 5, 1), uvec3(1, 0, 4)   // Bottom
    );

    mat4 mvp = pc.viewProj * transformBuffer.transforms[meshletIndex];

    // Output vertices
    SetMeshOutputsEXT(8, 12);

    for (uint i = 0; i < 8; ++i) {
        gl_MeshVerticesEXT[i].gl_Position = mvp * vec4(vertices[i], 1.0);
        fragColor[i] = vertices[i] + 0.5; // Simple color based on position
    }

    // Output primitives
    for (uint i = 0; i < 12; ++i) {
        gl_PrimitiveTriangleIndicesEXT[i] = indices[i];
    }
}