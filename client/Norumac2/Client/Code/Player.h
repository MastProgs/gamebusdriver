#pragma once
#include "Obj.h"

class CShader;
class CVIBuffer;
class CTexture;
class CTerrainCol;
class CDynamicMesh;
class CPlayer :
	public CObj
{
public:
	CPlayer();
	virtual ~CPlayer();

private:
	CVIBuffer*		m_pBuffer;
	CShader*		m_pVertexShader;
	CShader*		m_pPixelShader;
	CShader*		m_pShadowAniVertexShader;
	CShader*		m_pShadowNonAniVertexShader;
	CTexture*		m_pTexture;
	VTXTEX*			m_pVerTex;
	CTerrainCol*	m_pTerrainCol;
	Packet*			m_Packet;
	float			m_fSeverTime;
	bool			m_bSendServer;
	PLAYER_STATE	m_ePlayerState;//�ִϽ�����Ʈ
	bool			m_bPush;
	DWORD			m_dwTime;
	bool			m_bMoving;
	
	DWORD			m_dwCellNum;//�׺�޽���
	bool			m_bStart;
	float			m_fComboTime;//�޺� �����ð�
	bool			m_bCombo[2];//�޺�����
	bool			m_bMoveSend;// �̵�����ȭ�� �ӽ�
	float			m_fSkillMoveTime;//��ų������ ���ʺ��� �̵��� �ɾ����� �ϴ� �ð���
	bool			m_bSkillUsed;//��ų��뿩��
	float			m_fKeyCool;//����Ű�� ��

	

	// Scene meshes shader constant buffers
	ID3D11Buffer*		m_pSceneVertexShaderCB;
	ID3D11Buffer*		m_pScenePixelShaderCB;

	bool			m_bQuestFlag = { false }; // ����Ʈ ���� ��Ŷ �ѹ��� �������� �ϱ� ���� ��ȣ ���� - �������� ���� �Ұ�
public:
	float			m_fPotionTime;//���� ��
	float			m_fTpTime;//�ڷ���Ʈ ��
	bool			m_bPotionCool;//���� ��
	bool			m_bTpCool;//�ڷ���Ʈ ��
	int				m_iQuestStateMount;
	QUESTSTATE		m_eQuestState;

public:
	virtual HRESULT Initialize(void);
	virtual int		Update(void);
	virtual void	Render(void);
	virtual void	ShadowmapRender(void);
	void			KeyInput(void);
	Packet*			GetPacket(void);
	void			AniMove(void);
	void			SetNaviIndex(DWORD dwIndex) { m_dwCellNum = dwIndex; }
	void			TimeSetter(void);

public:
	static CPlayer* Create(void);

private:
	HRESULT	AddComponent(void);
};

