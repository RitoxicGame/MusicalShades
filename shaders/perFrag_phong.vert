#version 430

uniform mat4 modelMat;
uniform mat4 viewMat; 
uniform mat4 projMat; 
uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform vec3 lookAt;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out vec3 color;
out vec3 color2;
out vec4 normal;
out vec4 lightDir;
out vec4 lightDir2;
out vec4 look;

void main() {
  vec4 lpos = viewMat * vec4(lightPos, 1.0f);
  vec4 ppos = viewMat * modelMat * vec4 (vertex_position, 1.0f);
  lightDir = normalize(lpos - ppos);
  
  vec4 lpos2 = viewMat * vec4(lightPos2, 1.0f);
  vec4 ppos2 = viewMat * modelMat * vec4 (vertex_position, 1.0f);
  lightDir2 = normalize(lpos2 - ppos2);
  
  normal = normalize (viewMat * modelMat * vec4(vertex_normal, 0.0f));
  
  color = vec3(1.0, 1.0, 0.0);
  color2 = vec3(0.0, 0.0, 1.0);
  
  look = normalize(vec4(lookAt, 1.0));
	
  gl_Position = projMat * viewMat * modelMat * vec4(vertex_position, 1.0f);
}