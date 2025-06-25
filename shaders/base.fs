#version 460 core
out vec4 fragColor;
in vec2 fuv;

uniform vec4 color;

uniform sampler2D trimmingTexture;


void main()
{
  if(texture(trimmingTexture, fuv).r  == 1.f){
    discard;
  }
  fragColor = color;
}
