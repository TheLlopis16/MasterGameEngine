cbuffer PerFrame : register(b1)
{
    float3 lightDir;
    float3 lightColor;
    float3 ambientColor;
    float3 viewPos;
};

cbuffer PerInstance : register(b2)
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    
    float4 diffuseColour;
    float Kd;
    float Ks;
    float shininess;
    bool hasDiffuseTex;
};

cbuffer Transforms : register(b0)
{
    float4x4 mvp;
};

struct VertexOutput
{
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 position : SV_POSITION;
};

VertexOutput main(float3 position : POSITION, float2 texCoord : TEXCOORD, float3 normal : NORMAL)
{
    VertexOutput output;
    output.position = mul(float4(position, 1.0f), mvp);
    output.texCoord = texCoord;
    output.worldPosition = mul(float4(position, 1.0f), modelMatrix).xyz;
    output.normal = mul((float3x3) normalMatrix, normal);
    
    return output;
}