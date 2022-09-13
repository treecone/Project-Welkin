#version 450

//Buffers
layout(binding = 0) uniform PerFrame 
{
    mat4 view;
    mat4 proj;
} 
perFrame;

layout(binding = 1) uniform PerObject 
{
    mat4 world;
    mat4 worldInverseTranpose;
} 
perObject;

layout(binding = 2) uniform PerMaterial 
{
    vec2 uvScale;
    mat4 view;
    mat4 proj;
} 
perMaterial;

//IN
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

//OUT
layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec3 outWorldPos;


void main() 
{

    outWorldPos = vec3(perObject.world * vec4(inPosition, 1.0));

    gl_Position = perFrame.proj * perFrame.view * perObject.world * vec4(inPosition, 1.0);

    outUV = inUV * perMaterial.uvScale;

    //Make sure the normal is in world space, and not local space, 
    //https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/transforming-normals
    outNormal = normalize(mat3(perObject.worldInverseTranpose) * inNormal);
    outTangent = normalize(mat3(perObject.worldInverseTranpose) * inTangent);


}