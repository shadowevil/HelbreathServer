#pragma once
#include <windows.h>

class CCharInfo  
{
public:
	CCharInfo();
	virtual ~CCharInfo();
	char m_cName[12], m_cMapName[12];
	short m_sSkinCol, m_sSex, m_sAppr1, m_sAppr2, m_sAppr3, m_sAppr4;
	short	m_sStr, m_sVit, m_sDex, m_sInt, m_sMag, m_sChr;
	int	m_sLevel;
//	short m_sLevel;
	DWORD   m_iExp;
	int   m_iApprColor;
	int   m_iYear, m_iMonth, m_iDay, m_iHour, m_iMinute;
};