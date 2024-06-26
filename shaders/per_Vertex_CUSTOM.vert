#version 430

uniform mat4 modelMat; 
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 lightPos_0;
uniform vec3 lightPos_1;
uniform vec3 lightPos_2;
uniform vec3 lightPos_3;
uniform vec3 lightPos_4;
uniform vec3 lightPos_5;
uniform float time;
//uniform float noise_level;
//uniform float low_freq;
uniform float high_freq;
//uniform vec3 eye;
uniform vec4 camera_n;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out vec3 color;

//This part remains unchanged from the displacement.vert shader
//created by Prof. Chao Peng for the IGME740 Assignment 3 shader example code
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

// Classic Perlin noise, periodic variant
// credit: https://github.com/ashima/webgl-noise/blob/master/src/classicnoise3D.glsl
// more about Perlin noise: https://en.wikipedia.org/wiki/Perlin_noise
vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}
vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}
float cnoise(vec3 P)
{
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//*end unchanged bits*

void main()
{
  // Value range of perlin noise is[-sqr(N/4), sqr(N/4)], 
  // where N is the dimension. For N = 3, range is [-0.866, 0.866]
  float noise = (cnoise(vertex_normal*2.5 + vec3(time)*1.5) + 0.866) / (2* 0.866); 
  
  //altered by QP
  //need the vertex normal to be in model space
  vec4 normal = normalize (modelMat * vec4(vertex_normal, 0.0f));
  
  //square the noise and freq scale to increase distinction between peaks
  float displacement = pow(pow(noise, 2) * high_freq, 2) * 
  //only peaks that are roughly perpendicular to the camera view should jut out!
  						pow(abs(dot(normalize(camera_n), normal)) - 1, 2);
  vec3 newPosition = vertex_position + (vertex_normal * displacement);
  
  vec4 ppos = viewMat * modelMat * vec4 (newPosition, 1.0f);
  
  vec4 lpos_0 = viewMat * vec4(lightPos_0, 1.0f);
  vec4 lightDir_0 = normalize(lpos_0 - ppos);
  
  vec4 lpos_1 = viewMat * vec4(lightPos_1, 1.0f);
  vec4 lightDir_1 = normalize(lpos_1 - ppos);
  
  vec4 lpos_2 = viewMat * vec4(lightPos_2, 1.0f);
  vec4 lightDir_2 = normalize(lpos_2 - ppos);
  
  vec4 lpos_3 = viewMat * vec4(lightPos_3, 1.0f);
  vec4 lightDir_3 = normalize(lpos_3 - ppos);
  
  vec4 lpos_4 = viewMat * vec4(lightPos_4, 1.0f);
  vec4 lightDir_4 = normalize(lpos_4 - ppos);
  
  vec4 lpos_5 = viewMat * vec4(lightPos_5, 1.0f);
  vec4 lightDir_5 = normalize(lpos_5 - ppos);
  
  normal = normalize (viewMat * modelMat * vec4(vertex_normal, 0.0f));
  
  float intensity_0 = max(dot(normal, lightDir_0), 0.0);
  float intensity_1 = max(dot(normal, lightDir_1), 0.0);
  float intensity_2 = max(dot(normal, lightDir_2), 0.0);
  float intensity_3 = max(dot(normal, lightDir_3), 0.0);
  float intensity_4 = max(dot(normal, lightDir_4), 0.0);
  float intensity_5 = max(dot(normal, lightDir_5), 0.0);
  
  vec4 r_0 = (2 * normal * dot(normal, lightDir_0)) - lightDir_0;
  vec4 r_1 = (2 * normal * dot(normal, lightDir_1)) - lightDir_1;
  vec4 r_2 = (2 * normal * dot(normal, lightDir_2)) - lightDir_2;
  vec4 r_3 = (2 * normal * dot(normal, lightDir_3)) - lightDir_3;
  vec4 r_4 = (2 * normal * dot(normal, lightDir_4)) - lightDir_4;
  vec4 r_5 = (2 * normal * dot(normal, lightDir_5)) - lightDir_5;
  
  //thank you to Ed for helping me figure out the specular components!
  float alpha = 20; //completely arbitrary
  
  //yellow
  float iR_0 = 0.20 + (2*intensity_0) + pow(max(dot(normalize(-ppos), r_0), 0.0), alpha);
  float iG_0 = 0.20 + (2*intensity_0) + pow(max(dot(normalize(-ppos), r_0), 0.0), alpha);
  float iB_0 = 0.05;
  
  //magenta
  float iR_1 = 0.20 + (2*intensity_1) + pow(max(dot(normalize(-ppos), r_1), 0.0), alpha);
  float iG_1 = 0.05;
  float iB_1 = 0.20 + (2*intensity_1) + pow(max(dot(normalize(-ppos), r_1), 0.0), alpha);
  
  //cyan
  float iR_2 = 0.05;
  float iG_2 = 0.20 + (2*intensity_2) + pow(max(dot(normalize(-ppos), r_2), 0.0), alpha);
  float iB_2 = 0.20 + (2*intensity_2) + pow(max(dot(normalize(-ppos), r_2), 0.0), alpha);
  
  //blue
  float iR_3 = 0.05;
  float iG_3 = 0.05;
  float iB_3 = 0.40 + (2*intensity_3) + pow(max(dot(normalize(-ppos), r_3), 0.0), alpha);
  
  
  //green
  float iR_4 = 0.05;
  float iG_4 = 0.20 + (2*intensity_4) + pow(max(dot(normalize(-ppos), r_4), 0.0), alpha);
  float iB_4 = 0.05;
  
  //red
  float iR_5 = 0.40 + (2*intensity_5) + pow(max(dot(normalize(-ppos), r_5), 0.0), alpha);
  float iG_5 = 0.05;
  float iB_5 = 0.05;
  
  color = vec3(
  (iR_0 + iR_1 + iR_2 + iR_3 + iR_4 + iR_5)*0.17, 
  (iG_0 + iG_1 + iG_2 + iG_3 + iG_4 + iG_5)*0.17, 
  (iB_0 + iB_1 + iB_2 + iB_3 + iB_4 + iB_5)*0.17);
	
  gl_Position = projMat * viewMat * modelMat * vec4(newPosition, 1.0f);
}




