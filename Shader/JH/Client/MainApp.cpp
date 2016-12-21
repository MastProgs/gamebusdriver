#include "stdafx.h"
#include "MainApp.h"

//�̱��� ��ü
#include "Device.h"
#include "TimeMgr.h"
#include "ResourcesMgr.h"
#include "ShaderMgr.h"

//�Ϲݰ�ü
#include "Renderer.h"
#include "Shader.h"
#include "Obj.h"
#include "Camera.h"
#include "RcCol.h"
#include "Input.h"
#include "Info.h"
#include "CubeCol.h"
#include "CylinderCol.h"
#include "Texture.h"
#include "RcTex.h"
#include "CubeTex.h"
#include "CylinderTex.h"
#include "StaticMesh.h"


CMainApp::CMainApp()
	:m_pRenderer(NULL),
	 m_pRcCol(NULL),
	m_pInfo(NULL),
	m_pTexture(NULL),
	m_pMesh(NULL)
{
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);
	SetConsoleTitleA("Debug");
}


CMainApp::~CMainApp()
{
	FreeConsole();
	Release();

}

HRESULT CMainApp::Initialize(void)
{
	
	m_pInfo = CInfo::Create(D3DXVECTOR3(0,0,1));

	CDevice::GetInstance()->CreateDevice();
	m_pGrapicDevcie = CDevice::GetInstance();

	CTimeMgr::GetInstance()->InitTime();

	m_pTexture = CTexture::Create(L"../Resource/bird.png");

	//m_pRcCol = CCubeCol::Create();
	//m_pRcCol = CCylinderTex::Create(1,1,1,20,20);
	//m_pRcCol = CCubeTex::Create();

	char cModelPath[MAX_PATH];
	//WideCharToMultiByte(CP_ACP, 0, L"../Resource/bird.FBX", MAX_PATH, cModelPath, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, L"../Resource/town.FBX", MAX_PATH, cModelPath, MAX_PATH, NULL, NULL);

	m_pMesh = CStaticMesh::Create(cModelPath);

	if (FAILED(CCamera::GetInstance()->Initialize()))
	{
		MessageBox(NULL, L"System Message", L"Camera Initialize Failed", MB_OK);
	}

	if (FAILED(CInput::GetInstance()->Initialize(g_hInst, g_hWnd)))
	{
		MessageBox(NULL, L"System Message", L"Input Initialize Failed", MB_OK);
	}
	
	m_pRenderer = CRenderer::Create();
	NULL_CHECK_RETURN_MSG(m_pRenderer, NULL, L"Renderer Create Failed");

	HRESULT hr = S_OK;

	if (FAILED(CResourcesMgr::GetInstance()->ReserveContainerSize(RESOURCE_END)))
	{
		MessageBox(NULL, L"System Message", L"Resource Container Reserve Failed", MB_OK);
		return E_FAIL;
	}

	hr = CShaderMgr::GetInstance()->AddShaderFiles(L"VERTEXSHADER", L"Shader.fx", "VS", "vs_5_0", SHADER_VS); //Ű��, fx��ġ, ���̴��̸�, ����, 
	if (FAILED(hr))
	{
		MessageBox(NULL, L"System Message", L"Vertex Shader Create Failed", MB_OK);
		return hr;
	}

	hr = CShaderMgr::GetInstance()->AddShaderFiles(L"PIXELSHADER", L"Shader.fx", "PS", "ps_5_0", SHADER_PS);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"System Message", L"PIXEL Shader Create Failed", MB_OK);
		return hr;
	}

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	m_pVertexShader = CShaderMgr::GetInstance()->Clone_Shader(L"VERTEXSHADER");
	m_pPixelShader = CShaderMgr::GetInstance()->Clone_Shader(L"PIXELSHADER");
	
	return S_OK;
}

int CMainApp::Update(void)
{

	CCamera::GetInstance()->Update();
	CInput::GetInstance()->SetInputState();
	m_pInfo->Update();
	CTimeMgr::GetInstance()->SetTime();




	m_pInfo->m_vPos.x = 0.f;
	m_pInfo->m_vPos.y = 0.f;
	m_pInfo->m_vPos.z = 0.f;

	m_pInfo->m_vScale = D3DXVECTOR3(0.1f, 0.1f, 0.1f);
	m_pInfo->m_fAngle[ANGLE_X] += D3DXToRadian(10.f) * CTimeMgr::GetInstance()->GetTime();
	m_pInfo->m_fAngle[ANGLE_Y] += D3DXToRadian(10.f) * CTimeMgr::GetInstance()->GetTime();
	//m_pInfo->m_fAngle[ANGLE_Z] += D3DXToRadian(10.f) * CTimeMgr::GetInstance()->GetTime();

	

	return 0;
}

void CMainApp::Render(void)
{
	m_pGrapicDevcie->BeginDevice();
	m_pRenderer->Render(CTimeMgr::GetInstance()->GetTime());

	// Initialize the projection matrix
	ConstantBuffer cb;

	D3DXMatrixTranspose(&cb.matWorld, &m_pInfo->m_matWorld);//9������ �׳� �ȾƵ������� 11������ ��ġ��� ������ �Ⱦƾ� ��
	D3DXMatrixTranspose(&cb.matView, &CCamera::GetInstance()->m_matView);
	D3DXMatrixTranspose(&cb.matProjection, &CCamera::GetInstance()->m_matProj);

	m_pGrapicDevcie->m_pDeviceContext->IASetInputLayout(m_pVertexShader->m_pVertexLayout);
	m_pGrapicDevcie->m_pDeviceContext->UpdateSubresource(m_pMesh->m_ConstantBuffer, 0, NULL, &cb, 0, 0);
	


	//�̿��� �������� ��������Ʈ ���� - ��/ ��� - 
	//D3D11_MAPPED_SUBRESOURCE MapResource;
	//m_pGrapicDevcie->m_pDeviceContext->Map(m_pRcCol->m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MapResource);
	//ConstantBuffer* pConstantBuffer = (ConstantBuffer *)MapResource.pData;
	////pConstantBuffer->matWorld = m_pInfo->m_matWorld;
	////pConstantBuffer->matView = CCamera::GetInstance()->m_matView;
	////pConstantBuffer->matProjection = CCamera::GetInstance()->m_matProj;
	//D3DXMatrixTranspose(&pConstantBuffer->matWorld, &m_pInfo->m_matWorld);
	//D3DXMatrixTranspose(&pConstantBuffer->matView, &CCamera::GetInstance()->m_matView);
	//D3DXMatrixTranspose(&pConstantBuffer->matProjection, &CCamera::GetInstance()->m_matProj);
	//m_pGrapicDevcie->m_pDeviceContext->Unmap(m_pRcCol->m_ConstantBuffer, 0);


	//D3D11_RASTERIZER_DESC RSDesc;
	//RSDesc.FillMode = D3D11_FILL_SOLID;           // ����ϰ� ������
	//RSDesc.CullMode = D3D11_CULL_NONE;         // �ø�� ���� ����
	//RSDesc.FrontCounterClockwise = FALSE;        // �ð������ �޸��� CCW
	//RSDesc.DepthBias = 0;                      //���� ���̾ �� 0
	//RSDesc.DepthBiasClamp = 0;
	//RSDesc.SlopeScaledDepthBias = 0;
	//RSDesc.DepthClipEnable = FALSE;            // ���� Ŭ���� ����
	//RSDesc.ScissorEnable = FALSE;             // ���� �׽�Ʈ ���� ����
	//RSDesc.MultisampleEnable = FALSE;          // ��Ƽ ���ø� ���� ����
	//RSDesc.AntialiasedLineEnable = FALSE;         // ���� ��Ƽ�ٸ���� ����

	//m_pGrapicDevcie->m_pDeviceContext->CreateRasterizerState(&RSDesc, &g_pRasterizerState);

	m_pGrapicDevcie->m_pDeviceContext->VSSetShader(m_pVertexShader->m_pVertexShader, NULL, 0);
	m_pGrapicDevcie->m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pMesh->m_ConstantBuffer);
	m_pGrapicDevcie->m_pDeviceContext->PSSetShader(m_pPixelShader->m_pPixelShader, NULL, 0);// ���� 9���� settransform �ߴ��͵�
	m_pGrapicDevcie->m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTexture->m_pTextureRV);
	m_pGrapicDevcie->m_pDeviceContext->PSGetSamplers(0, 1, &m_pTexture->m_pSamplerLinear);

	m_pGrapicDevcie->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pMesh->m_VertexBuffer, &m_pMesh->m_iVertexStrides, &m_pMesh->m_iVertexOffsets);


	//m_pGrapicDevcie->m_pDeviceContext->Draw(m_pMesh->m_iVertices, 0);
	m_pMesh->Render();

	m_pGrapicDevcie->EndDevice();
}

CMainApp * CMainApp::Create(void)
{
	CMainApp*		pMainApp = new CMainApp;
	if (FAILED(pMainApp->Initialize()))
	{
		delete pMainApp;
		pMainApp = NULL;
	}
	return pMainApp;
}

void CMainApp::Release(void)
{
	CDevice::GetInstance()->DestroyInstance();
	CTimeMgr::GetInstance()->DestroyInstance();
	CCamera::GetInstance()->DestroyInstance();
	CShaderMgr::GetInstance()->DestroyInstance();
	CInput::GetInstance()->DestroyInstance();
}
