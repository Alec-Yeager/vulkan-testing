#version 450

//layout takes a location value, this is the output location. We use 0 here.
//then we declare it as out.
layout (location = 0) out vec4 outColor;
void main () {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}