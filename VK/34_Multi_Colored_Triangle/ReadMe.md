Template is used from "32_b2_Perspective_Projection_Corrected_Y_Matrix_Way"

vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
NOTE : glm follows column major arrays. Just like OpenGL.
		BUT unlike OPenGL it's matrix array is 4x4, 2D array not 1D array of 16 elements.