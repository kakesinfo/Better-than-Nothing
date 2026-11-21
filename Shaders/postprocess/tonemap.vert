#version 450

layout(location = 0) out vec2 outUV;

void main() {
    // Fullscreen triangle without vertex buffer
    vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vec2 pos = positions[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    outUV = pos * 0.5 + 0.5;
}
