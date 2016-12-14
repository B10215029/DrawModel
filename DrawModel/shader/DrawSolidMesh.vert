#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 uv;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 normal;
out vec2 texCoord;

void main() {
	mat4 mvp = projection_matrix * view_matrix * model_matrix;
	gl_Position = mvp * vec4(position, 1);
	normal = mat3(view_matrix * model_matrix) * in_normal;
	texCoord = vec2(uv.x, 1-uv.y);
	//gl_Position = vec4((uv - vec2(0.5)) * 2, 0, 1);
}
