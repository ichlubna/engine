#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 1) uniform sampler2D textures[1];

void main()
{
    outColor = texture(textures[0],uv);
}
