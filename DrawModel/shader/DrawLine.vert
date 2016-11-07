#version 430

layout (location = 0) in vec3 position;
out vec3 pos;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	mat4 mvp = projection_matrix * view_matrix * model_matrix;
	gl_Position = mvp * vec4(position, 1);
	pos = gl_Position.xyz;
}
