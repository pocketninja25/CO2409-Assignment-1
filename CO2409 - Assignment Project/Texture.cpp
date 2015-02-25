#include "Texture.h"

ID3D10EffectShaderResourceVariable* CTexture::m_MapVar = NULL;


CTexture::CTexture(ID3D10ShaderResourceView* map)
{
	m_Map = map;
}


CTexture::~CTexture()
{
	Release();
}

bool CTexture::LoadTexture(wchar_t* textureName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, textureName, NULL, NULL, &m_Map, NULL));
}

void CTexture::Release()
{
	if (m_Map)
	{
		m_Map->Release();
		m_Map = NULL;
	}
}

ID3D10ShaderResourceView* CTexture::GetTexture()
{
	return m_Map;
}

void CTexture::SendToShader()
{
	if (m_Map && m_MapVar)
	{
		m_MapVar->SetResource(m_Map);
	}
}

void CTexture::SetShaderVariable(ID3D10EffectShaderResourceVariable* mapVar)
{
	m_MapVar = mapVar;
}