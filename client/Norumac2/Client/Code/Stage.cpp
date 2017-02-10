#include "stdafx.h"
#include "Stage.h"
#include "SceneMgr.h"
#include "RenderMgr.h"
#include "Terrain.h"
#include "TimeMgr.h"
#include "ObjMgr.h"
#include "Flower.h"
#include "Player.h"
#include "StaticObject.h"

CStage::CStage()
	: m_bFirstLogin(false)
{
}


CStage::~CStage()
{
}

HRESULT CStage::Initialize(void)
{
	if (FAILED(CreateObj()))
		return E_FAIL;

	// 서버와 통신을 위해서, 윈도우 핸들 값을 받아온다.
	

	return S_OK;
}

int CStage::Update(void)
{
	if (m_bFirstLogin == false)
	{
		
		m_bFirstLogin = true;
	}

	CObjMgr::GetInstance()->Update();


	if (GetAsyncKeyState('0'))
	{
		list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"Flower")->begin();
		list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"Flower")->end();

		for (iter; iter != iter_end;)
		{
			CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
			::Safe_Release(*iter);
			CObjMgr::GetInstance()->Get_ObjList(L"Flower")->erase(iter++);
			cout << CObjMgr::GetInstance()->Get_ObjList(L"Flower")->size() << endl;
		}		
		
	}

	return 0;
}

void CStage::Render(void)
{
	float fTime = CTimeMgr::GetInstance()->GetTime();
	CRenderMgr::GetInstance()->Render(fTime);
}

void CStage::Release(void)
{
}

CStage * CStage::Create(void)
{

	CStage* pLogo = new CStage;
	if (FAILED(pLogo->Initialize()))
	{
		::Safe_Delete(pLogo);
	}
	return pLogo;
	
}

HRESULT CStage::CreateObj(void)
{
	CRenderMgr* pRenderer = CRenderMgr::GetInstance();
	//터레인
	CObj* pObj = NULL;

	///////////////야매///////////////
	TCHAR  szMeshKey[MAX_PATH] = L"";
	TCHAR  szTexKey[MAX_PATH] = L"";
	int iHigh = 0;


	for (size_t i = 0; i < 25; ++i)
	{
		iHigh = i % 5;
		wsprintf(szMeshKey, L"Mesh_Town%d", i);
		wsprintf(szTexKey, L"Texture_Town%d", i);

		pObj = CStaticObject::Create(szMeshKey, szTexKey);
		if (pObj == NULL)
			return E_FAIL;

		pObj->SetScale(D3DXVECTOR3(0.05f, 0.05f, 0.05f));
		//pObj->SetPos(D3DXVECTOR3(0.f, 0.f, 10.f*i));
		pObj->SetPos(D3DXVECTOR3(0.f, 60.f, 0.f));
		

		CObjMgr::GetInstance()->AddObject(L"StaticObject", pObj);
	}
	

	for (int i = 0; i < 20; ++i)
	{
		pObj = CFlower::Create();
		if (pObj == NULL)
			return E_FAIL;

		float		fX = float(rand() % VERTEXCOUNTX);
		float		fZ = float(rand() % VERTEXCOUNTZ);

		pObj->SetPos(D3DXVECTOR3(fX, 0.f, fZ));

		CObjMgr::GetInstance()->AddObject(L"Flower", pObj);
	}

	pObj = CPlayer::Create();
	CObjMgr::GetInstance()->AddObject(L"Player", pObj);
	


	return S_OK;
}

