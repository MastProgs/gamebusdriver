#include "common.fx"

// shader input/output structure
cbuffer cbDirLight : register( b1 )
{
  float3 AmbientDown	: packoffset( c0 );
  float3 AmbientRange	: packoffset( c1 );
  float3 DirToLight		: packoffset( c2 );
  float3 DirLightColor	: packoffset( c3 );
}

// �� ���ؽ��� Ŭ���� ���� ��ġ�� �����ϱ� ���� ���ؽ� ���̴��� ��� �迭 ����
static const float2 arrBasePos[4] = {
	float2(-1.0, 1.0),
	float2(1.0, 1.0),
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
};

// Vertex shader
struct VS_OUTPUT
{
    float4 Position : SV_Position; // vertex position
	float2 cpPos	: TEXCOORD0;
};

// ���ؽ� �ε����� ���� ���ؽ� ������ġ�� Ŭ���� ���� xy��ġ�� ��ȯ
VS_OUTPUT DirLightVS(uint VertexID : SV_VertexID)
{
	VS_OUTPUT Output;

	Output.Position = float4( arrBasePos[VertexID].xy, 0.0, 1.0);
	Output.cpPos = Output.Position.xy;

	return Output;    
}

// Pixel shaders
// Ambient light calculation helper function
float3 CalcAmbient(float3 normal, float3 color)
{
	// Convert from [-1, 1] to [0, 1]
	float up = normal.y * 0.5 + 0.5;

	// Calculate the ambient value
	float3 ambient = AmbientDown + up * AmbientRange;

	// Apply the ambient value to the color
	return ambient * color;
}

// Directional light calculation helper function
float3 CalcDirectional(float3 position, Material material)
{
   // Phong diffuse
   float NDotL = dot(DirToLight, material.normal);
   float3 finalColor = DirLightColor.rgb * saturate(NDotL);
   // ��
   finalColor = ceil(finalColor * 5) / 5.f;
   
   // Blinn specular
   float3 ToEye = EyePosition - position;
   ToEye = normalize(ToEye);
   float3 HalfWay = normalize(ToEye + DirToLight);
   float NDotH = saturate(dot(HalfWay, material.normal));
   finalColor += DirLightColor.rgb * pow(NDotH, material.specPow) * material.specIntensity;

   return finalColor * material.diffuseColor.rgb;
}

// ���ؽ� ���̴����� ��ȯ�� �� �� �ȼ� �ؽ�ó ��ġ�� Ŭ���� ���� ��ġ�� ���� �ش� ���̴��� ���� ���� ��ȯ
float4 DirLightPS(VS_OUTPUT In) : SV_TARGET
{
	// GBuffer ����ŷ
	SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy);
	
	// �����͸� ���� ����ü�� ��ȯ
	Material mat;
	MaterialFromGBuffer(gbd, mat);

	// ���� ��ġ ����
	float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);

	// �ں��Ʈ ����Ʈ ���� ���
	float3 finalColor = CalcAmbient(mat.normal, mat.diffuseColor.rgb);

	// �𷺼ų� ����Ʈ ���� ���
	finalColor += CalcDirectional(position, mat);

	// Return the final color
	return float4(finalColor, 1.0);
}
