#version 450

//layout(location = 0) modifier specifies the index of the framebuffer
//location and pairs the var, so the names dont need to match in the vert shader
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) in vec3 fragColor;

void main() 
{
    outColor = vec4(fragTexCoord, 0.0, 1.0);
}