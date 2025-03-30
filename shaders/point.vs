#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  mat4 mvp = projection * view * model;
	float w = (mvp * vec4(0,0,0,1)).w;
  gl_Position = mvp * vec4(aPos * w, 1.0f);
}

