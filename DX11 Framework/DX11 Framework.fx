//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    
    float4 AmbientLight;
    float4 AmbientMaterial;
    float4 DiffuseMaterial;
    float4 DiffuseLight;
    
    float4 SpecularMaterial;
    float4 SpecularLight;
    float SpecularPower;
    float3 EyePosW;
    
    float3 LightVecw;
    float1 Time;
    

    

    
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 PosW : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 normal: NORMAL )
{

    VS_OUTPUT output = (VS_OUTPUT)0;
	//Pos.xy += 0.5 * sin(Pos.x) * sin(3.0f * Time);
	//Pos.z *= 0.6f + 0.4f * sin(2.0 * Time);
    output.Pos = mul( Pos, World );
    output.PosW = output.Pos.xyz;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    
    //convert from local space to world space
    //W component of vector is 0 as vectors cant be translated
    float3 normalW = mul(float4(normal, 0.0f), World).xyz;
    normalW = normalize(normalW);
    output.Normal = normalW;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
    float3 toEye = normalize(EyePosW - input.PosW);
    input.Normal = normalize(input.Normal);
    //compute color 
    float diffuseAmount = max(dot(LightVecw, input.Normal), 0.0f);
    float3 diffuse = diffuseAmount * (DiffuseMaterial * DiffuseLight).rgb;
    float3 ambient = (AmbientLight * AmbientMaterial).rgb;
    float3 r = reflect(-LightVecw, input.Normal); //reflection vector
    //determine how much (if any) specular light makes it to the eye
    float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);
    float3 specular = specularAmount * (SpecularMaterial * SpecularLight).rgb;

    float4 color;
    color.rgb = diffuse + ambient + specular;
    color.a = DiffuseMaterial.a;
    return color;
}
