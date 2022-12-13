#version 450

//Buffers

layout(push_constant) uniform PushConst
{
    int instanceID;
} 
pushConst;

//Per Frame ---------------------------------------
layout(set = 0, binding = 0) uniform PerFrame 
{
    mat4 view;
    mat4 proj;
} 
perFrame;

//Per Transform ----------------------------------
struct PerTransformStruct
{
	mat4 world;
	mat4 worldInverseTranspose;
};

layout(std140, set = 2, binding = 0) readonly buffer PerTransformBuffer
{
    PerTransformStruct perTransforms[];
} 
perTransformBuffer;


//IN - Vertex attributes -------------------------
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
    mat4 worldMatrix = perTransformBuffer.perTransforms[pushConst.instanceID].world;
    mat4 inverseTWorldMatrix = perTransformBuffer.perTransforms[pushConst.instanceID].worldInverseTranspose;


    outWorldPos = vec3(worldMatrix * vec4(inPosition, 1.0));

    gl_Position = perFrame.proj * perFrame.view * worldMatrix * vec4(inPosition, 1.0);

    outUV = inUV; // * perMaterial.uvScale;

    //Make sure the normal is in world space, and not local space, 
    //https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/transforming-normals
    outNormal = normalize(mat3(inverseTWorldMatrix) * inNormal);
    outTangent = vec3(0, 0, 0); //normalize(mat3(Push.worldInverseTranspose) * inTangent);
}