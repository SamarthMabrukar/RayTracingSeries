#version 450 core

#extension GL_ARB_separate_shader_objects : enable
layout(location=0) in vec2 out_TexCoord;
layout(location=0) out vec4 FragColor;

// Set 2: Per-material data (textures) - Bind per material
layout(set = 2, binding = 0) uniform sampler2D uTextureSampler;

void main(void)
{
	// code
	vec4 texColor = texture(uTextureSampler, out_TexCoord);
	FragColor = vec4(texColor.r, texColor.g, texColor.b, 1.0);
}
