#version 430

uniform vec4 color;
in vec3 pos;
layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(1 - pos.z, 0, pos.z, 1);
}
