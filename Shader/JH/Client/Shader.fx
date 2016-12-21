
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0) //������Ʈ ���� �ѱ�� ����� ����
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION; // ���� �����Ϳ��� ��ġ ������ �����ͼ� Pos�� ����
	float2 Tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION; // ������������ ��ȯ�� ��ġ ����
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUT VS(VS_INPUT input) // ���� ���̴��� ���� �߿��� �ӹ��� ���ð����� �ִ� ������ ��ġ�� ������������ ��ȯ
{	
	VS_OUT output = (VS_OUT)0;
	output.Pos = mul(input.Pos, World); // ���ð����� ��������� ���ؼ� ��������� ��ġ�� ���� 
	output.Pos = mul(output.Pos, View); // ���� ������� ������� ����
	output.Pos = mul(output.Pos, Projection); // ���� ������� ������������ ����
	output.Tex = input.Tex;

	return output; // ������������ ��ȯ�� ���� ��ȯ
}




//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUT input) : SV_Target
{
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
	return txDiffuse.Sample(samLinear, input.Tex);
//return input.Color;
// return float4(1.0f, 1.0f, 0.0f, 1.0f);
}



