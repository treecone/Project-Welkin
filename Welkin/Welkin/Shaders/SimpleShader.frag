#version 450


//IN
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inWorldPos;

layout (set = 1, binding = 0) uniform sampler2D materialTextures[1];

//OUT
layout(location = 0) out vec4 outColor;


void main() 
{
    //outColor = vec4(1, 1, 1, 1);
    //outColor = vec4(inNormal, 1.0);
    outColor = texture(materialTextures[0], inUV);
}