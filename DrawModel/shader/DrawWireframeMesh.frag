#version 430

in vec3 normal;
layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(0.5, 0.5, 0.5, 1) * (0.1 + abs(normal.z)) + vec4(0.1, 0.1, 0.1, 1.0) * pow(abs(normal.z), 40.0);
}
