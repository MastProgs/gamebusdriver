
// float vs half �����ؼ� �ڼ��� �˾ƺ��� ���̴� ����ȭ �ܰ迡�� �ʿ��ϰ� �ɰ��ɼ��� �ִ�
// float : ���� ������ �ε� �Ҽ���. �Ϲ������� 32��Ʈ
// half : �߰� ������ ������ �ε� �Ҽ���. �Ϲ������� 16��Ʈ

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
	half4 Pos : POSITION; // ���� �����Ϳ��� ��ġ ������ �����ͼ� Pos�� ����
	half2 Tex : TEXCOORD0;
};

struct VS_OUT
{
	half4 Pos : SV_POSITION; // ������������ ��ȯ�� ��ġ ����
	half2 Tex : TEXCOORD0;
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
	return txDiffuse.Sample(samLinear, input.Tex);
	//return input.Color;
}



