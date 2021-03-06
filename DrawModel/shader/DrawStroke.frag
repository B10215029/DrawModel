#version 430

uniform vec4 color;
uniform sampler2D colorTexture;
uniform sampler2D strokeTexture;

//in vec2 texCoord;
out vec4 fragColor;

void main() {
	//vec2 flipTexCoord = vec2(1, -1) * texCoord;
	fragColor = vec4(color.rgb, color.a * texture(strokeTexture, gl_PointCoord).a);
	//fragColor = color;
	//fragColor = texture(image, gl_PointCoord);;
}
