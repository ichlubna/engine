#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform uniformMatrix
{
     mat4 viewProjectionMatrix;
} matrix;

void main()
{
    gl_Position = matrix.viewProjectionMatrix * vec4(inPosition ,1.0);
}
