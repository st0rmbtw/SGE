// GLSL shader version 3.30 (for OpenGL 3.3)
#version 330

#ifdef GL_ES
precision mediump float;
#endif

in vec4 vColor;

out vec4 outColor;

void main()
{
	outColor = vColor;
}
