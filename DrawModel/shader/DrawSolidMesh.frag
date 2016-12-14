#version 430

uniform vec4 color;
uniform sampler2D tex0;
uniform int useTexture;
//uniform int isLighting;

in vec3 normal;
in vec2 texCoord;
layout (location = 0) out vec4 fragColor;

void main() {
	//vec4 color = vec4(0.2f, 0.2f, 0.2f, 1);
	//fragColor = vec4(0.5, 0.5, 0.5, 1) * (0.1 + abs(normal.z)) + vec4(0.1, 0.1, 0.1, 1.0) * pow(abs(normal.z), 40.0);
	//if (isLighting == 1) {
	if (useTexture != 0) {
		vec4 tc = texture(tex0, texCoord);
		vec4 dc = color * (1 - tc.a) + tc * tc.a;
		fragColor = dc * (0.1 + abs(normal.z)) + vec4(0.1, 0.1, 0.1, 1.0) * pow(abs(normal.z), 40.0);
		//fragColor = vec4(1, 1, 1, 1);
	}
	else
		//fragColor = color * (0.1 + abs(normal.z)) + texture(tex0, texCoord) * pow(abs(normal.z), 40.0);
		fragColor = color;
	//fragColor = texture(tex0, texCoord);
	fragColor.a = 1;
	//}
	//else {
		//fragColor = color;
	//}
}
