#ifndef SoundMgr_h__
#define SoundMgr_h__
/*********************************************************************************************
@ FileName : SoundMgr.h
@ FilePath : /System/Codes/
@ Date : 15.03.26
@ Author : SongSong
@ Brife : ���� ����� ����ϴ� �̱��� Ŭ����
*********************************************************************************************/
#include "fmod.h"
#include "fmod.hpp"
#include "fmod_dsp.h"
#include "fmod_errors.h"
#include "Include.h"

class CSoundMgr
{
	DECLARE_SINGLETON(CSoundMgr)
private:
	FMOD_SYSTEM*	m_pSystem;		//FMOD System Pointer
	FMOD_CHANNEL*	m_pEffCh;		//����Ʈ ä��
	FMOD_CHANNEL*	m_pBgaCh;		//����BGM ä��
	FMOD_CHANNEL*	m_pSkillCh;		//��ų �÷��� ä��
	FMOD_CHANNEL*	m_pMonster1;	//�ö�� ���� ä��
	FMOD_CHANNEL*	m_pMonster2;	//���� ���� ä��
	FMOD_CHANNEL*	m_pMonster3;	//���� ���� ä��1
	FMOD_CHANNEL*	m_pMonster4;	//���� ���� ä��2
	FMOD_CHANNEL*	m_pMonster5;	//���� ���� ä��
	FMOD_CHANNEL*	m_pRedBull;
	FMOD_CHANNEL*	m_pEffCh2;		//����Ʈ ä��2
	FMOD_CHANNEL*	m_pEffCh3;		//����Ʈ ä��3
	FMOD_CHANNEL*	m_pQuest1;		//����Ʈ ä��1
	FMOD_CHANNEL*	m_pQuest2;		//����Ʈ ä��2
	FMOD_CHANNEL*	m_pQuest3;		//����Ʈ ä��3


	FMOD_RESULT		m_Result;		//FMOD Running result;

	unsigned int	m_iVersion;
	map<TCHAR*, FMOD_SOUND*>	m_mapSound;		//������� ������ �ִ� map
public:
	void	Initialize();
	void	LoadSoundFile();
	void	PlaySound(TCHAR*	szSoundKey);
	void	PlaySoundByNum(int iNum);
	void	PlayBgm(TCHAR*	szSoundKey);
	void	PlaySkillSound(TCHAR*	szSoundKey);
	void	Monster1Sound( TCHAR*	szSoundKey);
	void	Monster2Sound( TCHAR*	szSoundKey);
	void	Monster3Sound( TCHAR*	szSoundKey);
	void	Monster4Sound( TCHAR*	szSoundKey);

	void	PlaySkillSound2( TCHAR*	szSoundKey);
	void	PlaySkillSound3( TCHAR*	szSoundKey);
	void	PlayInterface1( TCHAR*	szSoundKey);
	void	PlayInterface2( TCHAR*	szSoundKey);
	void	PlayInterface3( TCHAR*	szSoundKey);

	void	RedBullSound( TCHAR*	szSoundKey);
	void	RedBullSoundStop();
	void	UpdateSound();
	void	StopBga();
	void	StopSoundAll();
	map<TCHAR*, FMOD_SOUND*>*	GetSoundMap(){return &m_mapSound;}
private:
	void	ErrorCheck(FMOD_RESULT _res);
private:
	explicit CSoundMgr(void);
public:
	~CSoundMgr(void);
};


#endif // SoundMgr_h__