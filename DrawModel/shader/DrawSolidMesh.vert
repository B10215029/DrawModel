#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 in_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 normal;

void main() {
	mat4 mvp = projection_matrix * view_matrix * model_matrix;
	gl_Position = mvp * vec4(position, 1);
	normal = mat3(view_matrix * model_matrix) * in_normal;
}
