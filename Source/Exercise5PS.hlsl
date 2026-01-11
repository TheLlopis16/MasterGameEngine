cbuffer Material : register(b1)
{
    float4 color;
    bool hasColorTex;
};

Texture2D colorTex : register(t0);
SamplerState colorSamp : register(s0);

float4 main(float2 coord : TEXCOORD) : SV_TARGET
{
    return hasColorTex ? colorTex.Sample(colorSamp, coord) * color : color;
}