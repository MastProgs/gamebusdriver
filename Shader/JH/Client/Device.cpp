#include "stdafx.h"
#include "Device.h"

IMPLEMENT_SINGLETON(CDevice)

CDevice::CDevice()
{
	m_pDevice = NULL;
	m_pSwapChain = NULL;
	m_pRenderTargetView = NULL;
	m_pDeviceContext = NULL;

	m_pDepthStencilBuffer = NULL;
	m_pDepthStencilView = NULL;

}


CDevice::~CDevice()
{
	Release();
}

HRESULT CDevice::CreateDevice(void)
{

	if (CreateSwapChain() == false)
		return E_FAIL;


	// ����� �׷� �ְ��� �ϴ� �ĸ� ������ �κ� ���簢�� ������ ����Ʈ�� �Ѵ�
	D3D11_VIEWPORT vp; 
	vp.TopLeftX = 0; // ���簢���� ��ġ
	vp.TopLeftY = 0; // ���簢���� ��ġ
	vp.Width = (FLOAT)WINCX; // ���簢���� ũ��
	vp.Height = (FLOAT)WINCY; // ���簢���� ũ��
	vp.MinDepth = 0.0f; // �ּ� ���� ���� ��
	vp.MaxDepth = 1.0f; // �ִ� ���� ���� ��
	
	m_pDeviceContext->RSSetViewports( // Directx���� ����Ʈ�� �˷���
		1, // ���� ����Ʈ ����
		&vp); // ����Ʈ �迭�� ����Ű�� ������


	return S_OK;
}

bool CDevice::CreateSwapChain(void)
{
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WINCX;
	dxgiSwapChainDesc.BufferDesc.Height = WINCY;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = g_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH/*0*/;

	UINT dwCreateDeviceFlags = 0;
#ifdef _DEBUG
	dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE d3dDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT nDriverTypes = sizeof(d3dDriverTypes) / sizeof(D3D_DRIVER_TYPE);

	D3D_FEATURE_LEVEL pd3dFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0, // �켱 d3d 11 ���� ���� ����
		D3D_FEATURE_LEVEL_10_1, // �״��� d3d 10.1 ���� ���� ����
		D3D_FEATURE_LEVEL_10_0 // �״��� d3d 10 ���� ���� ����
	};
	UINT nFeatureLevels = sizeof(pd3dFeatureLevels) / sizeof(D3D_FEATURE_LEVEL);

	D3D_DRIVER_TYPE nd3dDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL nd3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT hResult = S_OK;
	for (UINT i = 0; i < nDriverTypes; i++)
	{
		nd3dDriverType = d3dDriverTypes[i];
		if (SUCCEEDED(hResult = D3D11CreateDeviceAndSwapChain(NULL, nd3dDriverType, NULL, dwCreateDeviceFlags, pd3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION, &dxgiSwapChainDesc, &m_pSwapChain, &m_pDevice, &nd3dFeatureLevel, &m_pDeviceContext))) break;
	}

	if (!CreateRenderTargetStanciView()) return(false);


	return true;
}

bool CDevice::CreateRenderTargetStanciView(void)
{
	HRESULT hResult = S_OK;

	ID3D11Texture2D *pd3dBackBuffer;

	if (FAILED(hResult = m_pSwapChain->GetBuffer( // ��ȯ�罽�� ����Ű�� �����͸� ����
		0, // ����� �ϴ� �ĸ������ ����(�ĸ� ���۸� �ϳ��� ���⶧���� 0)
		__uuidof(ID3D11Texture2D), // ���� �������̽� ������ ����
		(LPVOID *)&pd3dBackBuffer))) // �ĸ� ���۸� ����Ű�� �����͸� ������
		return(false);

	if (FAILED(hResult = m_pDevice->CreateRenderTargetView( // ���� ���並 �����ϴ� �Լ�
		pd3dBackBuffer, // ���� ������� ����� �ڿ�(�ĸ���ۿ� ���� ���� ���並 �����ϹǷ� ��� ���� �ĸ���۸� ����)
		NULL, // D3D11_RENDER_TARGET_VIEW_DESC ����ü�� ����Ű�� ������(������ ������ �����ؼ� �ڿ��� �����ߴٸ� NULL�̿��� ��)
		&m_pRenderTargetView))) // ���� ���並 ������
		return(false);

	if (pd3dBackBuffer)
		pd3dBackBuffer->Release(); // GetBuffer�� ����ϸ� COM���� Ƚ���� ���� �׷��� �ٽ� ������

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC d3dDepthStencilBufferDesc; // ������ �ؽ�ó�� �����ϴ� ����ü
	ZeroMemory(&d3dDepthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	d3dDepthStencilBufferDesc.Width = WINCX; // �ؽ�ó�� �ʺ�
	d3dDepthStencilBufferDesc.Height = WINCY; // �ؽ�ó�� ����
	d3dDepthStencilBufferDesc.MipLevels = 1; // �Ӹ� ������ ����(����/���ٽ� ���۸� ���� �ؽ�ó������ �Ӹ� ������ �ϳ��� �ʿ�)
	d3dDepthStencilBufferDesc.ArraySize = 1; // �ؽ�ó �迭�� �ؽ�ó ����(����/���ٽ� ������ ��쿡�� �ؽ�ó �ϳ��� �ʿ�)
	d3dDepthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // �ؼ��� ������ ���ϴ� �ʵ�
	// (�� �ڼ���[0, 1]�������� ���Ǵ� ��ȣ���� 24��Ʈ ���� �� �ϳ��� [0, 255] �������� ���Ǵ� 8��Ʈ ��ȣ���� ���� ���ٽ� ������ ����)
	d3dDepthStencilBufferDesc.SampleDesc.Count = 1;
	d3dDepthStencilBufferDesc.SampleDesc.Quality = 0;
	d3dDepthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT; // �ؽ�ó�� �뵵�� ���ϴ� �ʵ�(�ڿ��� GPU�� �а� ��� �Ѵٸ� �̿뵵 ����)
	d3dDepthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // �ڿ��� ���������ο� ������� ���� �������� �����ϴ� �ϳ� �̻��� �÷��׵��� OR�� �����ؼ� ����
	// (����/���ٽ� ������ ��� D3D11_BIND_DEPTH_STENCIL ����)
	d3dDepthStencilBufferDesc.CPUAccessFlags = 0; // CPU�� �ڿ��� �����ϴ� ����� �����ϴ� �÷��׵��� �����Ѵ�
	// (����/���ٽ� ������ ��� GPU�� ���� CPU�� �������� �����Ƿ� 0)
	d3dDepthStencilBufferDesc.MiscFlags = 0; // ��Ÿ �÷��׵�� ����/���ٽ� ���ۿ��� ������� �����Ƿ� �׳� 0

	if (FAILED(hResult = m_pDevice->CreateTexture2D(
		&d3dDepthStencilBufferDesc, // ������ �ؽ�ó�� �����ϴ� ����ü
		NULL, // �ؽ�ó�� ä�� �ʱ� �ڷḦ ����Ű�� ������(���� ���ؽ�ó�� ����/���ٽ� ���۷� ����� ���̹Ƿ� ���� �ڷḦ ä���ʿ����)
		&m_pDepthStencilBuffer))) // ����/���ٽ� ���۸� ����Ű�� �����͸� �����ش�
		return(false);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = d3dDepthStencilBufferDesc.Format;
	d3dDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Texture2D.MipSlice = 0;
	
	if (FAILED(hResult = m_pDevice->CreateDepthStencilView(
		m_pDepthStencilBuffer, // �並 �����ϰ��� �ϴ� �ڿ�
		&d3dDepthStencilViewDesc/*nullptr*/, // D3D11_DEPTH_STENCIL_VIEW_DESC�� ����Ű�� ������
		&m_pDepthStencilView)))  // ����/���ٽ� �並 �����ش�
		return(false);

	m_pDeviceContext->OMSetRenderTargets( // ����� ������������ ��� ���б� �ܰ迡 ���� �Լ�
		1, // ������ �ϴ� ���� ����� ����
		&m_pRenderTargetView, // ���������ο� ���� ���� ��� ����� ����Ű�� �����͵��� ���� �迭�� ù ���Ҹ� ����Ű�� ������
		m_pDepthStencilView); // ���������ο� ���� ����/���ٽ� �並 ����Ű�� ������


	return true;
}

void CDevice::BeginDevice(void)
{
	D3DXCOLOR DevcieColor(0, 0, 1, 1);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, DevcieColor);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CDevice::EndDevice(void)
{
	m_pSwapChain->Present(0, 0); // ���� ���ۿ� �ĸ� ���۸� ��ȯ�ؼ� ȭ�鿡 ǥ��
}

void CDevice::Release(void)
{
	if (m_pDeviceContext)
		m_pDeviceContext->ClearState();

	::Safe_Release(m_pRenderTargetView);
	::Safe_Release(m_pDepthStencilBuffer);
	::Safe_Release(m_pDepthStencilView);
	::Safe_Release(m_pSwapChain);
	::Safe_Release(m_pDeviceContext);
	::Safe_Release(m_pDevice);

}
