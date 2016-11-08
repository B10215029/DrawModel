#version 430

uniform sampler2D image;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    //vec2 flipTexCoord = vec2(1,-1) * texCoord;
	fragColor = texture(image, texCoord);
	//fragColor = vec4(1, 0, 0, 1);
}
