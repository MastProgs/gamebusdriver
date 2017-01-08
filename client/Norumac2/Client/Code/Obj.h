#pragma once

class CDevice;
class CComponent;
class CInfo;
class CObj
{
public:
	CObj();
	virtual ~CObj();

protected:
	map<wstring, CComponent*>	m_mapComponent;
	CInfo*						m_pInfo;
	float						m_fViewZ;
	bool						m_bZSort;
	CDevice*					m_pGrapicDevice;
	ReleaseType					m_eReleaseType;
	vector<string>				vecAniName;

public:
	virtual HRESULT Initialize(void);
	virtual int		Update(void);
	virtual void	Render(void) {};

public:
	static CObj* Create(ReleaseType _eType);

private:
	virtual void Release(void);

public:
	const float&	GetViewZ(void) const;
	CInfo*			GetInfo(void) { return m_pInfo; }
	ReleaseType&	GetReleaseType(void) { return m_eReleaseType; }
	

public:
	void SetPos(D3DXVECTOR3 vPos); player_data*	GetPacketData(void);
	void SetPacketData(player_data* pPacketData);

protected:
	void Compute_ViewZ(const D3DXVECTOR3* pPos);
};

