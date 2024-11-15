// GLSL texturing vertex shader

#version 140

#ifdef GL_ES
precision mediump float;
#endif

layout(std140) uniform Scene
{
	mat4 wvpMatrix;
	mat4 wMatrix;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
    vNormal = normalize((wMatrix * vec4(normal, 0)).xyz);
	vTexCoord = texCoord;
}
