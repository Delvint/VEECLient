//common definitions

//light defintion
//type: 0-directional, 1-point, 2-spot
//param: light parameters

#define NUM_SHADOW_CASCADE 6

struct cameraData_t {
  mat4 camModel;
  mat4 camView;
  mat4 camProj;
  vec4 param;
};

struct lightData_t {
  ivec4  itype;
  mat4x4 lightModel;
  vec4   col_ambient;
  vec4   col_diffuse;
  vec4   col_specular;
  vec4   param;
  cameraData_t shadowCameras[NUM_SHADOW_CASCADE];
};

struct perObjectData_t {
  mat4 model;
  mat4 modelInvTrans;
  vec4 color;
  vec4 param;
};
