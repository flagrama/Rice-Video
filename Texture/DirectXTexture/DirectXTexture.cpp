/*
Copyright (C) 2003-2009 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "DirectXTexture.h"
#include "../Texture.h"
#include "../TextureManager.h"
#include "../../Config.h"
#include "../../typedefs.h"
#include "../../Debugger/Debugger.h"
#include "../../Device/GraphicsContext.h"
#include "../../Render/DirectX/D3DRender.h"

CDirectXTexture::CDirectXTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage) :
	CTexture(dwWidth,dwHeight,usage)
{
	LPDIRECT3DTEXTURE9 pTxt;

	if (dwWidth < 1)	
		dwWidth = 1;

	if (dwHeight < 1)
		dwHeight = 1;

	if (dwWidth*dwHeight > 256*256 && usage == AS_NORMAL )
		TRACE2("Large texture: width (%d) , height (%d)", dwWidth, dwHeight);

	pTxt = CreateTexture(dwWidth, dwHeight, usage);

	// Copy from old surface to new surface
	if (m_pTexture != NULL)
	{
		m_pTexture->Release();
	}

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
	m_pTexture = pTxt;
}
	
CDirectXTexture::~CDirectXTexture()
{
	m_pTexture->Release();
	m_pTexture = NULL;
	m_dwWidth = 0;
	m_dwHeight = 0;
}


//////////////////////////////////////////////////
// Get information about the DIBitmap
// This locks the bitmap (and stops 
// it from being resized). Must be matched by a
// call to EndUpdate();
bool CDirectXTexture::StartUpdate(DrawInfo *di)
{
	if (m_pTexture == NULL)
		return false;

	D3DLOCKED_RECT d3d_lr;
	HRESULT hr = m_pTexture->LockRect(0, &d3d_lr, NULL, D3DLOCK_NOSYSLOCK);
	if (SUCCEEDED(hr))
	{
		di->dwHeight = (uint16)m_dwHeight;
		di->dwWidth = (uint16)m_dwWidth;
		di->dwCreatedHeight = m_dwCreatedTextureHeight;
		di->dwCreatedWidth = m_dwCreatedTextureWidth;
		di->lpSurface = d3d_lr.pBits;
		di->lPitch    = d3d_lr.Pitch;
		return true;
	}
	else
	{
		return false;
	}
}

///////////////////////////////////////////////////
// This releases the DIB information, allowing it
// to be resized again
void CDirectXTexture::EndUpdate(DrawInfo *di)
{
	if (m_pTexture == NULL)
		return;

	m_pTexture->UnlockRect( 0 );
}


LPDIRECT3DTEXTURE9 CDirectXTexture::CreateTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage)
{
	HRESULT hr;
	LPDIRECT3DTEXTURE9 lpSurf = NULL;
	unsigned int dwNumMaps = 1;

	D3DFORMAT pf = D3DFMT_A8R8G8B8;
	switch(usage)
	{
	case AS_BACK_BUFFER_SAVE:
		pf = D3DFMT_X8R8G8B8;
		break;
	case AS_RENDER_TARGET:
		pf = D3DFMT_A8R8G8B8;
		break;
	default:
		pf = D3DFMT_A8R8G8B8;
		break;
	}

	m_dwCreatedTextureWidth = dwWidth;
	m_dwCreatedTextureHeight = dwHeight;

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	if( m_Usage == AS_RENDER_TARGET)
	{
		D3DXCheckTextureRequirements(g_pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps,D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, &pf, D3DPOOL_DEFAULT);
		if(options.bMipMaps)
		{
			hr = g_pD3DDev->CreateTexture(m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 0 , D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET, pf, D3DPOOL_DEFAULT, &lpSurf,NULL);
			lpSurf->GenerateMipSubLevels(); // Hint to the GPU to generate the mip maps
		}
		else
		{
			hr = g_pD3DDev->CreateTexture(m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 0 , D3DUSAGE_RENDERTARGET, pf, D3DPOOL_DEFAULT, &lpSurf,NULL);
		}
	}
	else
	{
		D3DXCheckTextureRequirements(g_pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps, D3DUSAGE_AUTOGENMIPMAP, &pf, D3DPOOL_MANAGED);
		if(options.bMipMaps)
		{
			hr = g_pD3DDev->CreateTexture(m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 0, D3DUSAGE_AUTOGENMIPMAP, pf, D3DPOOL_MANAGED, &lpSurf,NULL);
			lpSurf->GenerateMipSubLevels(); // Hint to the GPU to generate the mip maps
		}
		else
		{
			hr = g_pD3DDev->CreateTexture(m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 1, 0, pf, D3DPOOL_MANAGED, &lpSurf,NULL);
		}
	}

#ifdef _DEBUG
	if( m_dwCreatedTextureWidth<dwWidth || m_dwCreatedTextureHeight<dwHeight)
	{
		TRACE0("!!Check me, created texture is smaller");
	}
#endif
	
	m_fYScale = (float)m_dwCreatedTextureHeight/(float)m_dwHeight;
	m_fXScale = (float)m_dwCreatedTextureWidth/(float)m_dwWidth;

	if( !CDXGraphicsContext::IsResultGood(hr))
	{
		TRACE2("!!Unable to create surface!! %d x %d", dwWidth, dwHeight);
		return NULL;
	}
	return lpSurf;		
}