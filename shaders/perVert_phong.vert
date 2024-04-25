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

void main() {
  
  vec4 lpos = viewMat * vec4(lightPos, 1.0f);
  vec4 ppos = viewMat * modelMat * vec4 (vertex_position, 1.0f);
  vec4 lightDir = normalize(lpos - ppos);
  
  vec4 lpos2 = viewMat * vec4(lightPos2, 1.0f);
  vec4 ppos2 = viewMat * modelMat * vec4 (vertex_position, 1.0f);
  vec4 lightDir2 = normalize(lpos2 - ppos2);
  
  vec4 normal = normalize (viewMat * modelMat * vec4(vertex_normal, 0.0f));
  
  //vec4 look = normalize(vec4(lookAt, 1.0));
  //vec4 look = normalize(vec4(0.0, 0.0, 0.0, 1.0));
  
  float intensity = max(dot(normal, lightDir), 0.0);
  float intensity2 = max(dot(normal, lightDir2), 0.0);
  
  //float v = max(dot(lookAt, normal), 0.0);
  
  vec4 r = (2 * normal * dot(normal, lightDir)) - lightDir;
  vec4 r2 = (2 * normal * dot(normal, lightDir2)) - lightDir2;
  
  //thank you to Ed for helping me figure this out!
  float iR = 0.0 + intensity + pow(max(dot(normalize(-ppos), r), 0.0), 10);
  float iG = 0.15 + intensity;
  float iB = 0.0;
  
  float iR2 = 0.0 + pow(max(dot(normalize(-ppos), r2), 0.0), 10);
  float iG2 = 0.0;
  float iB2 = 0.15 + intensity2;
  
  color = vec3((iR + iR2)*0.5, (iG + iG2)*0.5, (iB + iB2)*0.5);
	
  gl_Position = projMat * viewMat * modelMat * vec4(vertex_position, 1.0f);
}




