// Sprite.cpp: implementation of the CSprite class.
//
//////////////////////////////////////////////////////////////////////

#include "Sprite.h"

extern char G_cSpriteAlphaDegree;

extern int G_iAddTable31[64][510], G_iAddTable63[64][510];
extern int G_iAddTransTable31[510][64], G_iAddTransTable63[510][64]; 

extern long    G_lTransG100[64][64], G_lTransRB100[64][64];
extern long    G_lTransG70[64][64], G_lTransRB70[64][64];
extern long    G_lTransG50[64][64], G_lTransRB50[64][64];
extern long    G_lTransG25[64][64], G_lTransRB25[64][64];
extern long    G_lTransG2[64][64], G_lTransRB2[64][64];

template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
	return (v < lo) ? lo : (hi < v) ? hi : v;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSprite::CSprite(HANDLE hPakFile, DXC_ddraw *pDDraw, const char *cPakFileName, short sNthFile, bool bAlphaEffect)
{
	DWORD  nCount;
	int iASDstart;

	m_stBrush	= 0;
	m_lpSurface = 0;
	m_bIsSurfaceEmpty = true;
	ZeroMemory(m_cPakFileName, sizeof(m_cPakFileName));

	m_cAlphaDegree = 1;
	m_bOnCriticalSection = false;
	m_iTotalFrame = 0;
	m_pDDraw = pDDraw;
	SetFilePointer(hPakFile, 24 + sNthFile*8, 0, FILE_BEGIN);
	ReadFile(hPakFile, &iASDstart,  4, &nCount, 0); 
	//i+100       Sprite Confirm
	SetFilePointer(hPakFile, iASDstart+100, 0, FILE_BEGIN); 
	ReadFile(hPakFile, &m_iTotalFrame,  4, &nCount, 0);
	m_dwBitmapFileStartLoc = iASDstart  + (108 + (12*m_iTotalFrame));
	m_stBrush = new stBrush[m_iTotalFrame];
	ReadFile(hPakFile, m_stBrush, 12*m_iTotalFrame, &nCount, 0);
	// PAK
	memcpy(m_cPakFileName, cPakFileName, strlen(cPakFileName));
	m_bAlphaEffect = bAlphaEffect;
}

CSprite::~CSprite()
{
	if (m_stBrush != 0) delete[] m_stBrush;
	if (m_lpSurface != 0) m_lpSurface->Release();
}

IDirectDrawSurface7 * CSprite::_pMakeSpriteSurface()
{
 IDirectDrawSurface7 * pdds4;
 HDC hDC;
 WORD * wp;

	m_bOnCriticalSection = true;

	if( m_stBrush == 0 ) return 0;

	CMyDib mydib(m_cPakFileName, m_dwBitmapFileStartLoc);
	m_wBitmapSizeX = mydib.m_wWidthX;
	m_wBitmapSizeY = mydib.m_wWidthY;
	pdds4 = m_pDDraw->pCreateOffScreenSurface(m_wBitmapSizeX, m_wBitmapSizeY);
    if (pdds4 == 0) return 0; 
	pdds4->GetDC(&hDC);
	mydib.PaintImage(hDC);
	pdds4->ReleaseDC(hDC);

	DDSURFACEDESC2  ddsd;
	ddsd.dwSize = 124;
	if (pdds4->Lock(0, &ddsd, DDLOCK_WAIT, 0) != DD_OK) return 0;
	pdds4->Unlock(0);

	wp = (WORD *)ddsd.lpSurface;
	m_wColorKey = *wp;

	m_bOnCriticalSection = false;

    return pdds4;
}


void CSprite::PutSpriteFast(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_lpBackB4) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = sX + pvx, dY = sY + pvy;

	const RECT& clip = m_pDDraw->m_rcClipArea;

	if (dX < clip.left) {
		short delta = (short)(clip.left - dX);
		sx += delta; szx -= delta;
		dX = (short)clip.left;
	}
	else if (dX + szx > clip.right) {
		szx -= (short)((dX + szx) - clip.right);
	}

	if (szx <= 0) { m_bOnCriticalSection = false; return; }

	if (dY < clip.top) {
		short delta = (short)(clip.top - dY);
		sy += delta; szy -= delta;
		dY = (short)clip.top;
	}
	else if (dY + szy > clip.bottom) {
		szy -= (short)((dY + szy) - clip.bottom);
	}

	if (szy <= 0) { m_bOnCriticalSection = false; return; }

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2) _SetAlphaDegree();
		else { _iCloseSprite(); if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; } }
	}

	RECT rcRect = { sx, sy, (short)(sx + szx), (short)(sy + szy) };
	m_rcBound = { dX, dY, (short)(dX + szx), (short)(dY + szy) };

	HRESULT hr = m_pDDraw->m_lpBackB4->BltFast(dX, dY, m_lpSurface, &rcRect, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
	if (FAILED(hr)) {

	}

	m_bOnCriticalSection = false;
}

void CSprite::PutSpriteFastNoColorKey(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_lpBackB4) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
	}

	if (szx <= 0) { m_bOnCriticalSection = false; return; }

	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
	}

	if (szy <= 0) { m_bOnCriticalSection = false; return; }

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	RECT rcRect = { sx, sy, static_cast<short>(sx + szx), static_cast<short>(sy + szy) };
	m_rcBound = { dX, dY, static_cast<short>(dX + szx), static_cast<short>(dY + szy) };

	HRESULT hr = m_pDDraw->m_lpBackB4->BltFast(
		dX, dY, m_lpSurface, &rcRect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

	if (FAILED(hr)) {
		// optional: handle DirectDraw error
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutSpriteFastNoColorKeyDst(LPDIRECTDRAWSURFACE7 lpDstS, int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!lpDstS) return;
	if (!m_pDDraw) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
	}

	if (szx <= 0) { m_bOnCriticalSection = false; return; }

	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
	}

	if (szy <= 0) { m_bOnCriticalSection = false; return; }

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	RECT rcRect = { sx, sy, static_cast<short>(sx + szx), static_cast<short>(sy + szy) };
	m_rcBound = { dX, dY, static_cast<short>(dX + szx), static_cast<short>(dY + szy) };

	HRESULT hr = lpDstS->BltFast(
		dX, dY, m_lpSurface, &rcRect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

	if (FAILED(hr)) {
		// optional: handle DirectDraw error
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutSpriteFastWidth(int sX, int sY, int sFrame, int sWidth, DWORD dwTime, bool bVertical)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_lpBackB4) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	if (!bVertical) {
		if (sWidth < szx)
			szx = static_cast<short>(sWidth);
	}
	else {
		if (sWidth < szy)
			szy = static_cast<short>(sWidth);
	}

	const RECT& clip = m_pDDraw->m_rcClipArea;

	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
	}

	if (szx <= 0) { m_bOnCriticalSection = false; return; }

	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
	}

	if (szy <= 0) { m_bOnCriticalSection = false; return; }

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	RECT rcRect = { sx, sy, static_cast<short>(sx + szx), static_cast<short>(sy + szy) };
	m_rcBound = { dX, dY, static_cast<short>(dX + szx), static_cast<short>(dY + szy) };

	HRESULT hr = m_pDDraw->m_lpBackB4->BltFast(
		dX, dY, m_lpSurface, &rcRect, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);

	if (FAILED(hr)) {
		// optional: handle DirectDraw error
	}

	m_bOnCriticalSection = false;
}

void CSprite::iRestore()
{
	HDC hDC;

	if (m_bIsSurfaceEmpty) return;
	if (m_stBrush == 0) return;
	if (m_lpSurface->IsLost() == DD_OK) return;

	m_lpSurface->Restore();
	DDSURFACEDESC2  ddsd;
	ddsd.dwSize = 124;
	if (m_lpSurface->Lock(0, &ddsd, DDLOCK_WAIT, 0) != DD_OK) return;
	m_pSurfaceAddr = (WORD*)ddsd.lpSurface;
	m_lpSurface->Unlock(0);
	CMyDib mydib(m_cPakFileName, m_dwBitmapFileStartLoc);
	m_lpSurface->GetDC(&hDC);
	mydib.PaintImage(hDC);
	m_lpSurface->ReleaseDC(hDC);
}

void CSprite::PutShadowSprite(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}

	if (!m_pSurfaceAddr) { m_bOnCriticalSection = false; return; }

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr);

	const int maxX = 799;
	const int maxY = 547;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;
	const int srcPitchStep = m_sPitch * 3;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; iy += 3) {
			for (int ix = 0; ix < szx; ++ix) {
				int iSangX = sX + pvx + ix + (iy - szy) / 3;
				int iSangY = sY + pvy + (iy + szy + szy) / 3;

				if (pSrc[ix] != m_wColorKey &&
					iSangX >= 0 && iSangX < maxX &&
					iSangY >= 0 && iSangY < maxY)
				{
					WORD& dst = pDst[iSangY * dstPitch + iSangX];
					dst = static_cast<WORD>((dst & 0xE79C) >> 2);
				}
			}
			pSrc += srcPitchStep;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; iy += 3) {
			for (int ix = 0; ix < szx; ++ix) {
				int iSangX = sX + pvx + ix + (iy - szy) / 3;
				int iSangY = sY + pvy + (iy + szy + szy) / 3;

				if (pSrc[ix] != m_wColorKey &&
					iSangX >= 0 && iSangX < maxX &&
					iSangY >= 0 && iSangY < maxY)
				{
					WORD& dst = pDst[iSangY * dstPitch + iSangX];
					dst = static_cast<WORD>((dst & 0x739C) >> 2);
				}
			}
			pSrc += srcPitchStep;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutShadowSpriteClip(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}

	if (!m_pSurfaceAddr) { m_bOnCriticalSection = false; return; }

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + (sy + szy - 1) * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) +
		dX + (dY + szy - 1) * m_pDDraw->m_sBackB4Pitch;

	const int pitchStepSrc = m_sPitch * 3;
	const int pitchStepDst = m_pDDraw->m_sBackB4Pitch + 1;
	const int maxX = m_pDDraw->m_rcClipArea.right;
	const int minX = m_pDDraw->m_rcClipArea.left;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; iy += 3) {
			for (int ix = 0; ix < szx; ++ix) {
				if (pSrc[ix] != m_wColorKey) {
					int shadowX = dX - (iy / 3) + ix;
					if (shadowX > minX && shadowX < maxX)
						pDst[ix] = static_cast<WORD>((pDst[ix] & 0xE79C) >> 2);
				}
			}
			pSrc -= pitchStepSrc;
			pDst -= pitchStepDst;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; iy += 3) {
			for (int ix = 0; ix < szx; ++ix) {
				if (pSrc[ix] != m_wColorKey) {
					int shadowX = dX - (iy / 3) + ix;
					if (shadowX > minX && shadowX < maxX)
						pDst[ix] = static_cast<WORD>((pDst[ix] & 0x739C) >> 2);
				}
			}
			pSrc -= pitchStepSrc;
			pDst -= pitchStepDst;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite(int sX, int sY, int sFrame, DWORD dwTime, int alphaDepth)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB100[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG100[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB100[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG100[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite_NoColorKey(int sX, int sY, int sFrame, DWORD dwTime, int alphaDepth)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB100[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
					(G_lTransG100[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
					G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB100[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
					(G_lTransG100[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
					G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite70(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB70[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG70[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB70[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB70[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG70[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB70[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}


void CSprite::PutTransSprite70_NoColorKey(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB70[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
					(G_lTransG70[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
					G_lTransRB70[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB70[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
					(G_lTransG70[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
					G_lTransRB70[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite50(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB50[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG50[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB50[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB50[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG50[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB50[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}
void CSprite::PutTransSprite50_NoColorKey(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB50[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
					(G_lTransG50[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
					G_lTransRB50[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB50[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
					(G_lTransG50[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
					G_lTransRB50[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite25(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB25[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG25[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB25[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB25[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG25[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB25[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite25_NoColorKey(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB25[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
					(G_lTransG25[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
					G_lTransRB25[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_lTransRB25[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
					(G_lTransG25[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
					G_lTransRB25[(dstPix & 0x001F)][(srcPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSprite2(int sX, int sY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx, sy = brush.sy, szx = brush.szx, szy = brush.szy;
	short pvx = brush.pvx, pvy = brush.pvy;
	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB2[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG2[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB2[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB2[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG2[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB2[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutShiftTransSprite2(int sX, int sY, int shX, int shY, int sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx + static_cast<short>(shX);
	short sy = brush.sy + static_cast<short>(shY);
	short szx = 128; // forced block width
	short szy = 128; // forced block height
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB2[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] << 11) |
						(G_lTransG2[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] << 5) |
						G_lTransRB2[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_lTransRB2[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] << 10) |
						(G_lTransG2[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] << 5) |
						G_lTransRB2[(dstPix & 0x001F)][(srcPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutFadeSprite(short sX, short sY, short sFrame, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx;
	short sy = brush.sy;
	short szx = brush.szx;
	short szy = brush.szy;
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && (m_cAlphaDegree != G_cSpriteAlphaDegree)) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey)
					pDst[ix] = static_cast<WORD>((pDst[ix] & 0xE79C) >> 2);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey)
					pDst[ix] = static_cast<WORD>((pDst[ix] & 0x739C) >> 2);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

bool CSprite::_iOpenSprite()
{
  	if (m_lpSurface != 0) return false;
	m_lpSurface = _pMakeSpriteSurface(); 
	if (m_lpSurface == 0) return false;
	m_pDDraw->iSetColorKey(m_lpSurface, m_wColorKey);
	m_bIsSurfaceEmpty  = false;
	DDSURFACEDESC2  ddsd;
	ddsd.dwSize = 124;
	if (m_lpSurface->Lock(0, &ddsd, DDLOCK_WAIT, 0) != DD_OK) return false;	
	m_pSurfaceAddr = (WORD *)ddsd.lpSurface;
	m_sPitch = (short)ddsd.lPitch >> 1;	
	m_lpSurface->Unlock(0);
	_SetAlphaDegree();
//	m_dwRefTime = timeGetTime();
	return true;
}

void CSprite::_iCloseSprite()
{
	if( m_stBrush == 0 ) return;
	if (m_lpSurface == 0) return;
	if (m_lpSurface->IsLost() != DD_OK)	return;
	m_lpSurface->Release();
	m_lpSurface = 0;
	m_bIsSurfaceEmpty = true;
	m_cAlphaDegree = 1;
}

void CSprite::PutSpriteRGB(int sX, int sY, int sFrame, int sRed, int sGreen, int sBlue, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx;
	short sy = brush.sy;
	short szx = brush.szx;
	short szy = brush.szy;
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && (m_cAlphaDegree != G_cSpriteAlphaDegree)) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	int iRedPlus255 = sRed + 255;
	int iGreenPlus255 = sGreen + 255;
	int iBluePlus255 = sBlue + 255;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					pDst[ix] = static_cast<WORD>(
						(G_iAddTable31[(srcPix & 0xF800) >> 11][iRedPlus255] << 11) |
						(G_iAddTable63[(srcPix & 0x07E0) >> 5][iGreenPlus255] << 5) |
						G_iAddTable31[(srcPix & 0x001F)][iBluePlus255]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					pDst[ix] = static_cast<WORD>(
						(G_iAddTable31[(srcPix & 0x7C00) >> 10][iRedPlus255] << 10) |
						(G_iAddTable31[(srcPix & 0x03E0) >> 5][iGreenPlus255] << 5) |
						G_iAddTable31[(srcPix & 0x001F)][iBluePlus255]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::PutTransSpriteRGB(int sX, int sY, int sFrame, int sRed, int sGreen, int sBlue, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx;
	short sy = brush.sy;
	short szx = brush.szx;
	short szy = brush.szy;
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && (m_cAlphaDegree != G_cSpriteAlphaDegree)) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	int iRedPlus255 = sRed + 255;
	int iGreenPlus255 = sGreen + 255;
	int iBluePlus255 = sBlue + 255;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_iAddTransTable31[G_lTransRB100[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] + iRedPlus255][(dstPix & 0xF800) >> 11] << 11) |
						(G_iAddTransTable63[G_lTransG100[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] + iGreenPlus255][(dstPix & 0x07E0) >> 5] << 5) |
						G_iAddTransTable31[G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)] + iBluePlus255][(dstPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				if (srcPix != m_wColorKey) {
					WORD dstPix = pDst[ix];
					pDst[ix] = static_cast<WORD>(
						(G_iAddTransTable31[G_lTransRB100[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] + iRedPlus255][(dstPix & 0x7C00) >> 10] << 10) |
						(G_iAddTransTable31[G_lTransG100[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] + iGreenPlus255][(dstPix & 0x03E0) >> 5] << 5) |
						G_iAddTransTable31[G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)] + iBluePlus255][(dstPix & 0x001F)]
						);
				}
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}
void CSprite::PutTransSpriteRGB_NoColorKey(int sX, int sY, int sFrame, int sRed, int sGreen, int sBlue, DWORD dwTime)
{
	if (!this) return;
	if (!m_pDDraw || !m_pDDraw->m_pBackB4Addr) return;
	if (!m_stBrush || sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx;
	short sy = brush.sy;
	short szx = brush.szx;
	short szy = brush.szy;
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) { m_bOnCriticalSection = false; return; }
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) { m_bOnCriticalSection = false; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
	}
	else if (m_bAlphaEffect && (m_cAlphaDegree != G_cSpriteAlphaDegree)) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) { m_bOnCriticalSection = false; return; }
		}
	}

	if (szx == 0 || szy == 0 || !m_pSurfaceAddr) {
		m_bOnCriticalSection = false;
		return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr) + sx + sy * m_sPitch;
	WORD* pDst = reinterpret_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + dY * m_pDDraw->m_sBackB4Pitch;

	const int srcPitch = m_sPitch;
	const int dstPitch = m_pDDraw->m_sBackB4Pitch;

	int iRedPlus255 = sRed + 255;
	int iGreenPlus255 = sGreen + 255;
	int iBluePlus255 = sBlue + 255;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_iAddTransTable31[G_lTransRB100[(dstPix & 0xF800) >> 11][(srcPix & 0xF800) >> 11] + iRedPlus255][(dstPix & 0xF800) >> 11] << 11) |
					(G_iAddTransTable63[G_lTransG100[(dstPix & 0x07E0) >> 5][(srcPix & 0x07E0) >> 5] + iGreenPlus255][(dstPix & 0x07E0) >> 5] << 5) |
					G_iAddTransTable31[G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)] + iBluePlus255][(dstPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				WORD srcPix = pSrc[ix];
				WORD dstPix = pDst[ix];
				pDst[ix] = static_cast<WORD>(
					(G_iAddTransTable31[G_lTransRB100[(dstPix & 0x7C00) >> 10][(srcPix & 0x7C00) >> 10] + iRedPlus255][(dstPix & 0x7C00) >> 10] << 10) |
					(G_iAddTransTable31[G_lTransG100[(dstPix & 0x03E0) >> 5][(srcPix & 0x03E0) >> 5] + iGreenPlus255][(dstPix & 0x03E0) >> 5] << 5) |
					G_iAddTransTable31[G_lTransRB100[(dstPix & 0x001F)][(srcPix & 0x001F)] + iBluePlus255][(dstPix & 0x001F)]
					);
			}
			pSrc += srcPitch;
			pDst += dstPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}

void CSprite::_GetSpriteRect(int sX, int sY, int sFrame)
{
	if (!this) return;
	if (!m_pDDraw || !m_stBrush) return;
	if (sFrame < 0 || sFrame >= m_iTotalFrame) return;

	const auto& brush = m_stBrush[sFrame];
	short sx = brush.sx;
	short sy = brush.sy;
	short szx = brush.szx;
	short szy = brush.szy;
	short pvx = brush.pvx;
	short pvy = brush.pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	const RECT& clip = m_pDDraw->m_rcClipArea;

	m_rcBound = { -1, -1, -1, -1 };

	// Horizontal clipping
	if (dX < clip.left) {
		short delta = static_cast<short>(clip.left - dX);
		sx += delta;
		szx -= delta;
		if (szx <= 0) return;
		dX = static_cast<short>(clip.left);
	}
	else if (dX + szx > clip.right) {
		szx -= static_cast<short>((dX + szx) - clip.right);
		if (szx <= 0) return;
	}

	// Vertical clipping
	if (dY < clip.top) {
		short delta = static_cast<short>(clip.top - dY);
		sy += delta;
		szy -= delta;
		if (szy <= 0) return;
		dY = static_cast<short>(clip.top);
	}
	else if (dY + szy > clip.bottom) {
		szy -= static_cast<short>((dY + szy) - clip.bottom);
		if (szy <= 0) return;
	}

	m_rcBound.left = dX;
	m_rcBound.top = dY;
	m_rcBound.right = dX + szx;
	m_rcBound.bottom = dY + szy;

	m_sPivotX = pvx;
	m_sPivotY = pvy;
}

void CSprite::_SetAlphaDegree()
{
	if (!this) return;
	if (!m_stBrush || !m_pSurfaceAddr) return;

	m_bOnCriticalSection = true;

	if (!m_bAlphaEffect || (m_cAlphaDegree == G_cSpriteAlphaDegree)) {
		m_bOnCriticalSection = false;
		return;
	}

	m_cAlphaDegree = G_cSpriteAlphaDegree;

	int sRed = 0, sGreen = 0, sBlue = 0;
	switch (m_cAlphaDegree) {
	case 1: sRed = sGreen = sBlue = 0; break;
	case 2: sRed = -3; sGreen = -3; sBlue = 2; break;
	}

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr);
	if (!pSrc) { m_bOnCriticalSection = false; return; }

	switch (m_pDDraw->m_cPixelFormat) {
	case 1: // 555 format
		for (int iy = 0; iy < m_wBitmapSizeY; ++iy) {
			for (int ix = 0; ix < m_wBitmapSizeX; ++ix) {
				WORD color = pSrc[ix];
				if (color == m_wColorKey) continue;

				int iR = ((color & 0xF800) >> 11) + sRed;
				int iG = ((color & 0x07E0) >> 5) + sGreen;
				int iB = (color & 0x001F) + sBlue;

				iR = clamp(iR, 0, 31);
				iG = clamp(iG, 0, 63);
				iB = clamp(iB, 0, 31);

				WORD newColor = static_cast<WORD>((iR << 11) | (iG << 5) | iB);
				if (newColor == m_wColorKey) ++newColor;
				pSrc[ix] = newColor;
			}
			pSrc += m_sPitch;
		}
		break;

	case 2: // 565 format
		for (int iy = 0; iy < m_wBitmapSizeY; ++iy) {
			for (int ix = 0; ix < m_wBitmapSizeX; ++ix) {
				WORD color = pSrc[ix];
				if (color == m_wColorKey) continue;

				int iR = ((color & 0x7C00) >> 10) + sRed;
				int iG = ((color & 0x03E0) >> 5) + sGreen;
				int iB = (color & 0x001F) + sBlue;

				iR = clamp(iR, 0, 31);
				iG = clamp(iG, 0, 31);
				iB = clamp(iB, 0, 31);

				WORD newColor = static_cast<WORD>((iR << 10) | (iG << 5) | iB);
				if (newColor == m_wColorKey) ++newColor;
				pSrc[ix] = newColor;
			}
			pSrc += m_sPitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}


bool CSprite::_bCheckCollison(int sX, int sY, short sFrame, int msX, int msY)
{
	if (!this || !m_stBrush || m_bIsSurfaceEmpty) return false;
	if (sFrame < 0 || sFrame >= m_iTotalFrame) return false;

	if (msX < m_pDDraw->m_rcClipArea.left + 3 ||
		msX > m_pDDraw->m_rcClipArea.right - 3 ||
		msY < m_pDDraw->m_rcClipArea.top + 3 ||
		msY > m_pDDraw->m_rcClipArea.bottom - 3)
		return false;

	short sx = m_stBrush[sFrame].sx;
	short sy = m_stBrush[sFrame].sy;
	short szx = m_stBrush[sFrame].szx;
	short szy = m_stBrush[sFrame].szy;
	short pvx = m_stBrush[sFrame].pvx;
	short pvy = m_stBrush[sFrame].pvy;

	short dX = sX + pvx;
	short dY = sY + pvy;

	if (msX < dX || msX > dX + szx || msY < dY || msY > dY + szy)
		return false;

	if (dX < m_pDDraw->m_rcClipArea.left + 3) {
		short diff = (m_pDDraw->m_rcClipArea.left + 3) - dX;
		sx += diff;
		szx -= diff;
		if (szx < 0) return false;
		dX = m_pDDraw->m_rcClipArea.left + 3;
	}
	else if (dX + szx > m_pDDraw->m_rcClipArea.right - 3) {
		short diff = (dX + szx) - (m_pDDraw->m_rcClipArea.right - 3);
		szx -= diff;
		if (szx < 0) return false;
	}

	if (dY < m_pDDraw->m_rcClipArea.top + 3) {
		short diff = (m_pDDraw->m_rcClipArea.top + 3) - dY;
		sy += diff;
		szy -= diff;
		if (szy < 0) return false;
		dY = m_pDDraw->m_rcClipArea.top + 3;
	}
	else if (dY + szy > m_pDDraw->m_rcClipArea.bottom - 3) {
		short diff = (dY + szy) - (m_pDDraw->m_rcClipArea.bottom - 3);
		szy -= diff;
		if (szy < 0) return false;
	}

	SetRect(&m_rcBound, dX, dY, dX + szx, dY + szy);

	WORD* pSrc = reinterpret_cast<WORD*>(m_pSurfaceAddr);
	if (!pSrc) return false;
	pSrc += sx + (sy * m_sPitch);

	int tX = dX;
	int tY = dY;

	if (msY < tY + 3 || msX < tX + 3) return false;

	pSrc += m_sPitch * (msY - tY - 3);
	for (int iy = 0; iy <= 6; ++iy) {
		for (int ix = msX - tX - 3; ix <= msX - tX + 3; ++ix) {
			if (pSrc[ix] != m_wColorKey)
				return true;
		}
		pSrc += m_sPitch;
	}

	return false;
}

void CSprite::PutShiftSpriteFast(int sX, int sY, int shX, int shY, int sFrame, DWORD dwTime)
{
	if (!this || !m_stBrush) return;
	if (sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	short sx = m_stBrush[sFrame].sx + shX;
	short sy = m_stBrush[sFrame].sy + shY;
	short szx = 128;
	short szy = 128;
	short pvx = m_stBrush[sFrame].pvx;
	short pvy = m_stBrush[sFrame].pvy;

	short dX = sX + pvx;
	short dY = sY + pvy;

	if (dX < m_pDDraw->m_rcClipArea.left) {
		short diff = m_pDDraw->m_rcClipArea.left - dX;
		sx += diff;
		szx -= diff;
		if (szx <= 0) { m_rcBound.top = -1; return; }
		dX = m_pDDraw->m_rcClipArea.left;
	}
	else if (dX + szx > m_pDDraw->m_rcClipArea.right) {
		short diff = (dX + szx) - m_pDDraw->m_rcClipArea.right;
		szx -= diff;
		if (szx <= 0) { m_rcBound.top = -1; return; }
	}

	if (dY < m_pDDraw->m_rcClipArea.top) {
		short diff = m_pDDraw->m_rcClipArea.top - dY;
		sy += diff;
		szy -= diff;
		if (szy <= 0) { m_rcBound.top = -1; return; }
		dY = m_pDDraw->m_rcClipArea.top;
	}
	else if (dY + szy > m_pDDraw->m_rcClipArea.bottom) {
		short diff = (dY + szy) - m_pDDraw->m_rcClipArea.bottom;
		szy -= diff;
		if (szy <= 0) { m_rcBound.top = -1; return; }
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) return;
	}
	else if (m_bAlphaEffect && (m_cAlphaDegree != G_cSpriteAlphaDegree)) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) return;
		}
	}

	RECT rcRect = { sx, sy, static_cast<short>(sx + szx), static_cast<short>(sy + szy) };
	m_rcBound = { dX, dY, static_cast<short>(dX + szx), static_cast<short>(dY + szy) };

	m_pDDraw->m_lpBackB4->BltFast(dX, dY, m_lpSurface, &rcRect,
		DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

	m_bOnCriticalSection = false;
}

void CSprite::PutRevTransSprite(int sX, int sY, int sFrame, DWORD dwTime, int alphaDepth)
{
	if (!this || !m_stBrush) return;
	if (sFrame < 0 || sFrame >= m_iTotalFrame) return;

	m_rcBound.top = -1;
	m_bOnCriticalSection = true;

	short sx = m_stBrush[sFrame].sx;
	short sy = m_stBrush[sFrame].sy;
	short szx = m_stBrush[sFrame].szx;
	short szy = m_stBrush[sFrame].szy;
	short pvx = m_stBrush[sFrame].pvx;
	short pvy = m_stBrush[sFrame].pvy;

	short dX = static_cast<short>(sX + pvx);
	short dY = static_cast<short>(sY + pvy);

	auto& clip = m_pDDraw->m_rcClipArea;

	if (dX < clip.left) {
		short diff = clip.left - dX;
		sx += diff; szx -= diff;
		if (szx <= 0) return;
		dX = clip.left;
	}
	else if (dX + szx > clip.right) {
		short diff = (dX + szx) - clip.right;
		szx -= diff;
		if (szx <= 0) return;
	}

	if (dY < clip.top) {
		short diff = clip.top - dY;
		sy += diff; szy -= diff;
		if (szy <= 0) return;
		dY = clip.top;
	}
	else if (dY + szy > clip.bottom) {
		short diff = (dY + szy) - clip.bottom;
		szy -= diff;
		if (szy <= 0) return;
	}

	m_dwRefTime = dwTime;

	if (m_bIsSurfaceEmpty) {
		if (!_iOpenSprite()) return;
	}
	else if (m_bAlphaEffect && m_cAlphaDegree != G_cSpriteAlphaDegree) {
		if (G_cSpriteAlphaDegree == 2)
			_SetAlphaDegree();
		else {
			_iCloseSprite();
			if (!_iOpenSprite()) return;
		}
	}

	SetRect(&m_rcBound, dX, dY, dX + szx, dY + szy);

	WORD* pSrc = static_cast<WORD*>(m_pSurfaceAddr) + sx + (sy * m_sPitch);
	WORD* pDst = static_cast<WORD*>(m_pDDraw->m_pBackB4Addr) + dX + (dY * m_pDDraw->m_sBackB4Pitch);

	if (szx == 0 || szy == 0) return;

	switch (m_pDDraw->m_cPixelFormat) {
	case 1:
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				if (pSrc[ix] != m_wColorKey) {
					int iR = m_pDDraw->m_lFadeRB[(pDst[ix] & 0xF800) >> 11][((pSrc[ix] & 0xF800) >> 11) + alphaDepth];
					int iG = m_pDDraw->m_lFadeG[(pDst[ix] & 0x07E0) >> 5][((pSrc[ix] & 0x07E0) >> 5) + alphaDepth * 2];
					int iB = m_pDDraw->m_lFadeRB[(pDst[ix] & 0x001F)][((pSrc[ix] & 0x001F)) + alphaDepth];
					pDst[ix] = static_cast<WORD>((iR << 11) | (iG << 5) | iB);
				}
			}
			pSrc += m_sPitch;
			pDst += m_pDDraw->m_sBackB4Pitch;
		}
		break;

	case 2:
		for (int iy = 0; iy < szy; ++iy) {
			for (int ix = 0; ix < szx; ++ix) {
				if (pSrc[ix] != m_wColorKey) {
					int iR = m_pDDraw->m_lFadeRB[(pDst[ix] & 0x7C00) >> 10][((pSrc[ix] & 0x7C00) >> 10) + alphaDepth];
					int iG = m_pDDraw->m_lFadeG[(pDst[ix] & 0x03E0) >> 5][((pSrc[ix] & 0x03E0) >> 5) + alphaDepth];
					int iB = m_pDDraw->m_lFadeRB[(pDst[ix] & 0x001F)][((pSrc[ix] & 0x001F)) + alphaDepth];
					pDst[ix] = static_cast<WORD>((iR << 10) | (iG << 5) | iB);
				}
			}
			pSrc += m_sPitch;
			pDst += m_pDDraw->m_sBackB4Pitch;
		}
		break;
	}

	m_bOnCriticalSection = false;
}