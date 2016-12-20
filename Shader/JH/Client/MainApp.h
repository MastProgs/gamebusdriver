#pragma once
class CDevice;
class CRenderer;
class CInfo;
class CVIBuffer;
class CShader;
class CTexture;
class CResources;
class CStaticMesh;
class CInput;
class CMainApp
{
public:
	CMainApp();
	virtual ~CMainApp();

private:
	CDevice*	m_pGrapicDevcie;
	CRenderer*	m_pRenderer;
	CVIBuffer*  m_pRcCol;
	CInfo*		m_pInfo;
	CShader*	m_pVertexShader;
	CShader*	m_pPixelShader;
	CTexture*	m_pTexture;
	//CInput*		m_pInput;
	CVIBuffer* m_pMesh;


public:
	HRESULT Initialize(void);
	int		Update(void);
	void	Render(void);

public:
	static CMainApp* Create(void);
private:
	void Release(void);
};

