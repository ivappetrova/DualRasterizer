// Global Variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : World;
float3 gCameraPosition : CameraPosition;

// Textures
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

// Light Properties
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float gKd = 7.0f;
float gShininess = 25.0f;

// Ambient
float3 gAmbient = float3(0.03f, 0.03f, 0.03f);

// PI constant
static const float PI = 3.14159265359f;

// Sampler States
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
    MaxAnisotropy = 16;
};

// Blend State
BlendState gBlendState
{
    BlendEnable[0] = false;
};

// Depth Stencil State
DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less;
    StencilEnable = false;
};

// Input/Output Structures
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

// Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.0f), gWorld);
    output.Normal = normalize(mul(input.Normal, (float3x3) gWorld));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) gWorld));
    output.TexCoord = input.TexCoord;
    
    return output;
}

float4 CalculateLighting(VS_OUTPUT input, SamplerState sampleState)
{
    float3 diffuseColor = gDiffuseMap.Sample(sampleState, input.TexCoord).rgb;
    float3 normalMapSample = gNormalMap.Sample(sampleState, input.TexCoord).rgb;
    float specularValue = gSpecularMap.Sample(sampleState, input.TexCoord).r;
    float glossiness = gGlossinessMap.Sample(sampleState, input.TexCoord).r;
    
    float3 tangentSpaceNormal;
    tangentSpaceNormal.x = 2.0f * normalMapSample.r - 1.0f;
    tangentSpaceNormal.y = 2.0f * normalMapSample.g - 1.0f;
    tangentSpaceNormal.z = 2.0f * normalMapSample.b - 1.0f;
    tangentSpaceNormal = normalize(tangentSpaceNormal);

    float3 binormal = normalize(cross(input.Normal, input.Tangent));
    float3x3 tangentSpaceAxis = float3x3(
        input.Tangent,
        binormal,
        input.Normal
    );
    
    float3 finalNormal = normalize(mul(tangentSpaceNormal, tangentSpaceAxis));
    float3 viewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);
    float3 lightDir = normalize(gLightDirection);
    
    float observedArea = max(0.0f, dot(finalNormal, -lightDir));
    float3 lambertDiffuse = (diffuseColor * gKd / PI) * observedArea;
    float3 phongSpecular = float3(0.0f, 0.0f, 0.0f);
    if (observedArea > 0.0f)
    {
        float3 reflect = lightDir - 2.0f * dot(lightDir, finalNormal) * finalNormal;
        float cosAlpha = max(0.0f, dot(reflect, viewDirection));
        float phongExponent = glossiness * gShininess;
        float phongSpecularReflection = specularValue * pow(cosAlpha, phongExponent);
        phongSpecular = float3(phongSpecularReflection, phongSpecularReflection, phongSpecularReflection);
    }
    
    float3 finalColor = lambertDiffuse + phongSpecular + gAmbient;
    
    return float4(finalColor, 1.0f);
}

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
    return CalculateLighting(input, samPoint);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
    return CalculateLighting(input, samLinear);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
    return CalculateLighting(input, samAnisotropic);
}

// Techniques
technique11 PointTechnique
{
    pass P0
    {
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(gDepthStencilState, 0);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Point()));
    }
}

technique11 LinearTechnique
{
    pass P0
    {
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(gDepthStencilState, 0);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
    }
}

technique11 AnisotropicTechnique
{
    pass P0
    {
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(gDepthStencilState, 0);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
    }
}