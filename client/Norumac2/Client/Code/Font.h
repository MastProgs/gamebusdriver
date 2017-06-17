#pragma once

#ifndef Font_h__
#define Font_h__

#include "Resources.h"
#include "FW1FontWrapper.h"

class CFont
{

private:
	IFW1FontWrapper*		m_pFW1FontWarpper;
	IFW1Factory*			m_pFW1FontFactory;
	ID3D11DeviceContext*	m_pDeivceContext;

public:
	FontType		m_eType;//�ܰ��� ����
	wstring			m_wstrText;//�ؽ�Ʈ ����
	float			m_fSize;//��Ʈ ������
	UINT32			m_nColor;//��Ʈ ���� (0xAABBGGRR)
	UINT			m_nFlag;/*
	��Ʈ�� �÷��� ����.
	FW1_LEFT - ��������(����Ʈ)
	FW1_CENTER - �������
	FW1_RIGHT - ����������

	FW1_TOP - �ؽ�Ʈ�� ���̾ƿ��ڽ� ���� ����(����Ʈ)
	FW1_VCENTER - �ؽ�Ʈ�� �߾ӿ� �������� ����
	FW1_BOTTOM - �ؽ�Ʈ�� ���̾ƿ� �ڽ� �Ʒ��� ����.

	FW1_NOWORDWRAP - �ڵ����� ���̾ƿ� �ڽ��� ��ġ�°� ���� ����.
	FW1_ALIASED - ��Ƽ�������¡�� �����ϰ� ���.
	FW1_CLIPRECT - clip-rect�� �� �÷��׿� ��õǾ�����, ��� �ؽ�Ʈ���� �ٰ����� Ŭ����.
	FW1_NOGEOMETRYSHADER - ���ϼ��̴� ���� �׷�����, �ε��̵� ������� ���ϼ��̴� ��� cpu�� ���
	FW1_CONSTANTSPREPARED - Ʈ������ ��İ� clpi-rect�� ������Ʈ ���ۿ� ������Ʈ ���� ����. ������ ������ ���� ���� ����ȭ��.
	FW1_BUFFERSPREPARED - ���������� �ε��� ���۰� �̹� ���ε��Ǿ�����, ������ ������ ���۸� ȣ���Ͽ� ����ȭ��.
	FW1_STATEPREPARED - �ùٸ� ���̴�/������Ʈ ���� ���� �̹� ���ε��Ǿ�����, ������ ������ ���³� �������̵��� �⺻ ���¸� ȣ���Ͽ� ����ȭ��.
	FW1_IMMEDIATECALL = FW1_CONSTANTSPREPARED | FW1_BUFFERSPREPARED | FW1_STATEPREPARED - ����ȭ�� �̷��� �Ҽ� ����. ���� �ؽ�Ʈ���� ���� ������ �����ÿ� ��õ.
	FW1_RESTORESTATE - ��� ����� ���ϵǸ�, ����̽� ���ؽ�Ʈ�� ������ ȣ�� ���¸� ������(����Ʈ).
	FW1_NOFLUSH - ȣ���ϴµ��� �߰��� �� ���ڵ��� ����̽�-���ҽ����� �������� ����. ������ �۾� ������ �����ϱ� ���� ���� �����忡 ���� �ؽ�Ʈ�� ���ÿ� �׸� �� �� �÷��׸� ����Ͽ� ���۵� ���ؽ�Ʈ�� ���� �� �÷��׸� ����ϴ� ���� ����.
	FW1_CACHEONLY - Any new glyphs will be cached in the atlas and glyph-maps, but no geometry is drawn.
	FW1_NONEWGLYPHS - No new glyphs will be added to the atlas or glyph-maps. Any glyphs not already present in the atlas will be replaced with a default fall-back glyph (empty box).
	FW1_ANALYZEONLY - A text-layout will be run through DirectWrite and new fonts will be prepared, but no actual drawing will take place, and no additional glyphs will be cached.
	FW1_UNUSED - ������.
	*/
	D3DXVECTOR2		m_vPos;//��Ʈ�� ��ġ(2D)
	float			m_fOutlineSize;//�ܰ��� ������
	UINT32			m_nOutlineColor;//�ܰ��� ���� (0xAARRGGBB)

	DWORD			m_dwRefCount;

public:
	static CFont*		Create(const wstring& _wstPath);
	virtual CFont*		CloneFont(void);

private:
	HRESULT			Load_Font(const wstring& _wstPath);

public:
	HRESULT			Initialize(const wstring& _wstPath);
	void			Render();
	DWORD			Release();


protected:
	 CFont();
public:
	virtual ~CFont();
};

#endif // Font_h__
