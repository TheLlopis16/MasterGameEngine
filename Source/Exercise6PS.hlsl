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

Texture2D colorTex : register(t0);
SamplerState colorSamp : register(s0);

float4 main(float3 worldPosition : POSITION, float3 normal : NORMAL, float2 coord : TEXCOORD) : SV_TARGET
{
    float3 baseColor = diffuseColour.rgb;
    if (hasDiffuseTex)
        baseColor *= colorTex.Sample(colorSamp, coord).rgb;
    
    float3 N = normalize(normal);
    float3 L = normalize(lightDir);
    float3 V = normalize(viewPos - worldPosition);
    
    float NdotL = max(dot(N, L), 0.0f);
    float ambient = ambientColor * baseColor;
    float3 diffuse = lightColor * baseColor * (Kd * NdotL);
    
    float3 specular = 0.0f;
    if (NdotL > 0.0f)
    {
        float3 R = reflect(-L, N);
        float RdotV = max(dot(R, V), 0.0f);
        specular = lightColor * (Ks * pow(RdotV, shininess));
    }
    
    float3 color = ambient + diffuse + specular;
    return float4(color, 1.0f);
}