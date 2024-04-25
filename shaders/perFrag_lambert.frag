#version 430
in vec3 color;
in vec4 normal;
in vec4 lightDir; 

out vec4 frag_color;

void main() {
  float intensity = max(dot(normal, lightDir), 0.0);

  frag_color = vec4(intensity*color, 1.0f);
}