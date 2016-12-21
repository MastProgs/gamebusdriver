#pragma once
#include "include.h"

class CTimeMgr;
class CDevice
{
public:
	CDevice();
	virtual ~CDevice();

public:
	DECLARE_SINGLETON(CDevice)

public:
	ID3D11Device*					m_pDevice; // ��� ���� ���˰� �ڿ� �Ҵ翡 ���̴� �������̽�
	IDXGISwapChain*					m_pSwapChain; // ������� �ؽ�ó�� �ĸ���� �ؽ�ó�� ��� �������̽�
	ID3D11RenderTargetView*			m_pRenderTargetView; // ���� ��� ��
	ID3D11DeviceContext*			m_pDeviceContext; // ���� ����� �����ϰ�, �ڿ��� �׷��� ������ ���ο� ����, gpu�� ������ ������ ��ɵ��� �����ϴ� �������̽�
	ID3D11Texture2D*				m_pDepthStencilBuffer; // ���� / ���ٽ� ���۸� ����Ű�� ������
	ID3D11DepthStencilView*			m_pDepthStencilView; //����/���ٽ� ��

public:
	HRESULT CreateDevice(void);
	bool	CreateSwapChain(void);
	bool	CreateRenderTargetStanciView(void);

public:
	void	BeginDevice(void);
	void	EndDevice(void);

private:
	void Release(void);
};



