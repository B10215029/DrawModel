#version 430

const float c_pone = 1.0;
const float c_none = -1.0;
const float c_zero = 0.0;

const vec2 vertexs[4] = vec2[](
	vec2(c_none, c_none),
	vec2(c_pone, c_none),
	vec2(c_pone, c_pone),
	vec2(c_none, c_pone));

const vec4 colors[4] = vec4[](
	vec4(c_pone, c_zero, c_zero, c_pone),
	vec4(c_zero, c_pone, c_zero, c_pone),
	vec4(c_zero, c_zero, c_pone, c_pone),
	vec4(c_pone, c_pone, c_zero, c_pone));

out vec4 color;

void main() {
	vec2 vposition = vertexs[gl_VertexID];
	color = colors[gl_VertexID];
	gl_Position = vec4(vposition, vec2(c_zero, c_pone));
}