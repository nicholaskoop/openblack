#include "GameShaders.h"

using namespace OpenBlack::Shaders;

const char* WorldObject::VertexShader = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

out vec3 v_norm;

uniform mat4 viewProj;

void main()
{
    gl_Position = viewProj * vec4(position, 1.0);
	v_norm = normal;
}
)";

const char* WorldObject::FragmentShader = R"(
#version 150 core

in vec3 v_norm;

out vec4 outColor;

void main()
{
	vec3 col = 0.5 + 0.5 * v_norm;
    outColor = vec4(col, 1.0);
}
)";