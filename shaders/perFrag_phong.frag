#version 430

//uniform vec4 lookAt;

in vec3 color;
in vec3 color2;
in vec4 normal;
in vec4 lightDir; 
in vec4 lightDir2;
in vec4 look;

out vec4 frag_color;

void main() {
  float intensity = max(dot(normal, lightDir), 0.0);
  float intensity2 = max(dot(normal, lightDir2), 0.0);
  
  //float v = max(dot(lookAt, normal), 0.0);
  
  vec4 r = (2 * normal * dot(normal, lightDir)) - lightDir;
  vec4 r2 = (2 * normal * dot(normal, lightDir2)) - lightDir2;
  
  float iR = 0.0 + intensity + pow(max(dot(r, look), 0.0), 10);
  float iG = 0.15 + intensity;
  float iB = 0.0;
  
  float iR2 = 0.0 + intensity2 + pow(max(dot(r, look), 0.0), 10);
  float iG2 = 0.0;
  float iB2 = 0.15 + intensity2;
  
  frag_color = vec4(iR + iR2, iG + iG2, iB + iB2, 1.0);
}