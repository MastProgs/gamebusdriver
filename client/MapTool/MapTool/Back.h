#pragma once
#include "Scene.h"

class CMapToolView;
class CBack :
	public CScene
{
public:
	CMapToolView* m_pMainView;
public:
	CBack();
	virtual ~CBack();

	// CScene��(��) ���� ��ӵ�
	virtual HRESULT Initialize(void);
	virtual int Update(void);
	virtual void Render(void);
	virtual void Release(void);

public:
	static CBack* Create(void);
	void SetMainView(CMapToolView* pMainView);

public:
	HRESULT	CreateObj(void);
};

