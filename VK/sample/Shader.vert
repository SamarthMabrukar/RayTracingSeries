#version 450 core

#extension GL_ARB_separate_shader_objects : enable
layout(location=0) in vec4 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=0) out vec2 out_TexCoord;

// Set 0: Per-frame data (camera matrices) - Bind once per frame
layout(set = 0, binding = 0) uniform MVPmatrix
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
} uMVP;

// Set 1: Per-object data (model matrix) - Bind per object
layout(set = 1, binding = 0) uniform ModelMatrix
{
    mat4 modelMatrix;
} uModel;

void main(void)
{
	// code
	out_TexCoord = vTexCoord;
	gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uModel.modelMatrix * vPosition;
}
