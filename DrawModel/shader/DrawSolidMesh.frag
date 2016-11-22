#version 430

uniform vec4 color;
//uniform int isLighting;

in vec3 normal;
layout (location = 0) out vec4 fragColor;

void main() {
	//vec4 color = vec4(0.2f, 0.2f, 0.2f, 1);
	//fragColor = vec4(0.5, 0.5, 0.5, 1) * (0.1 + abs(normal.z)) + vec4(0.1, 0.1, 0.1, 1.0) * pow(abs(normal.z), 40.0);
	//if (isLighting == 1) {
		fragColor = color * (0.1 + abs(normal.z)) + vec4(0.1, 0.1, 0.1, 1.0) * pow(abs(normal.z), 40.0);
		fragColor.a = 1;
	//}
	//else {
		//fragColor = color;
	//}
}
