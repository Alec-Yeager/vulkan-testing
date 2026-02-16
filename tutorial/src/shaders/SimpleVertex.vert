#version 450

// This is obviously just for testing.
vec2 positions[3] = vec2[] 
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// that VertexIndex is the current vertex index we're working on. VULKAN ONLY
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}