#include "stdafx.h"
#include "CubeTex.h"
#include "Device.h"


CCubeTex::CCubeTex()
{
}


CCubeTex::~CCubeTex()
{
}

HRESULT CCubeTex::CreateBuffer(void)
{
	//����
	HRESULT hr = E_FAIL;
	m_iVertexStrides = sizeof(VTXTEX);
	m_iVertexOffsets = 0;

	D3DXVECTOR3 vMin(-1.f, -1.f, -1.f);
	D3DXVECTOR3 vMax(1.f, 1.f, 1.f);


	VTXTEX Vertex[] =
	{
		{ D3DXVECTOR3(vMin.x, vMax.y, vMin.z), D3DXVECTOR3(vMin.x, vMax.y, vMin.z) },
		{ D3DXVECTOR3(vMax.x, vMax.y, vMin.z), D3DXVECTOR3(vMax.x, vMax.y, vMin.z) },
		{ D3DXVECTOR3(vMax.x, vMin.y, vMin.z), D3DXVECTOR3(vMax.x, vMin.y, vMin.z) },
		{ D3DXVECTOR3(vMin.x, vMin.y, vMin.z), D3DXVECTOR3(vMin.x, vMin.y, vMin.z) },
		{ D3DXVECTOR3(vMin.x, vMax.y, vMax.z), D3DXVECTOR3(vMin.x, vMax.y, vMax.z) },
		{ D3DXVECTOR3(vMax.x, vMax.y, vMax.z), D3DXVECTOR3(vMax.x, vMax.y, vMax.z) },
		{ D3DXVECTOR3(vMax.x, vMin.y, vMax.z), D3DXVECTOR3(vMax.x, vMin.y, vMax.z) },
		{ D3DXVECTOR3(vMin.x, vMin.y, vMax.z), D3DXVECTOR3(vMin.x, vMin.y, vMax.z) },
	};

	D3D11_BUFFER_DESC vbd; // ������ ����� ũ�� ���� �Ҵ�
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT; // ������ �����
	vbd.ByteWidth = sizeof(VTXTEX) * 8; // ������ ũ��
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ������ �뵵
	vbd.CPUAccessFlags = 0; // ����Ʈ
	vbd.MiscFlags = 0; // ����Ʈ
	vbd.StructureByteStride = 0; // ����Ʈ

	D3D11_SUBRESOURCE_DATA vBufferData; // ������->������ �������� ������(��ǥ/����)���� ��� �ִ� ������
	ZeroMemory(&vBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vBufferData.pSysMem = Vertex;
	hr = CDevice::GetInstance()->m_pDevice->CreateBuffer(&vbd, &vBufferData, &m_VertexBuffer);

	if (FAILED(hr))
		return E_FAIL;


	UINT Index[] =
	{
		//+x
		0, 1, 2,
		0, 1, 3,
		//-x
		1, 5, 6,
		1, 6, 2,
		//+y
		3, 2, 6,
		3, 6, 7,
		//-y
		4, 0, 3,
		4, 3, 7,
		//+z
		4, 5, 1,
		4, 1, 0,
		//-z
		5, 4, 7,
		5, 7, 6,
	};

	m_iIndex = 36;

	D3D11_BUFFER_DESC ibd;//�ε�������
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(UINT) * m_iIndex;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = Index;
	hr = CDevice::GetInstance()->m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IndexBuffer);

	if (FAILED(hr))
		return E_FAIL;


	D3D11_BUFFER_DESC cbd;
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;
	hr = CDevice::GetInstance()->m_pDevice->CreateBuffer(&cbd, NULL, &m_ConstantBuffer);

	/*D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CDevice::GetInstance()->m_pDevice->CreateBuffer(&bd, NULL, &m_ConstantBuffer);*/




	if (FAILED(hr))
	{
		MessageBox(NULL, L"System Message", L"Constant Buffer Error", MB_OK);
		return hr;

	}

	return S_OK;
}

CCubeTex * CCubeTex::Create(void)
{
	CCubeTex*	pCubeCol = new CCubeTex;

	if (FAILED(pCubeCol->CreateBuffer()))
		Safe_Delete(pCubeCol);

	return pCubeCol;
}

CResources * CCubeTex::CloneResource(void)
{
	CResources* pResource = new CCubeTex(*this);

	pResource->AddRef();

	return pResource;
}

