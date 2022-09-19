#version 450

/*
//IN
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inWorldPos;

layout(binding = 1) uniform sampler2D texSampler;
*/
//OUT
layout(location = 0) out vec4 outColor;




void main() 
{
    outColor = vec4(0, 0, 0, 0);
    //outColor = vec4(inNormal, 1.0);
}