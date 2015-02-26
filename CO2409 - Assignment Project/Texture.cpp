#include "Texture.h"

ID3D10EffectShaderResourceVariable* CTexture::m_DiffSpecMapVar = NULL;
ID3D10EffectShaderResourceVariable* CTexture::m_NormalMapVar = NULL;

CTexture::CTexture(ID3D10ShaderResourceView* diffSpecMap, ID3D10ShaderResourceView* normalMap)
{
	m_DiffSpecMap = diffSpecMap;
	m_NormalMap = normalMap;
}


CTexture::~CTexture()
{
	Release();
}

bool CTexture::LoadDiffSpecMap(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_DiffSpecMap, NULL));
}

bool CTexture::LoadNormalMap(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_NormalMap, NULL));
}

void CTexture::Release()
{
	if (m_DiffSpecMap)
	{
		m_DiffSpecMap->Release();
		m_DiffSpecMap = NULL;
	}
	if (m_NormalMap)
	{
		m_NormalMap->Release();
		m_DiffSpecMap = NULL;
	}
}

ID3D10ShaderResourceView* CTexture::GetDiffSpecMap()
{
	return m_DiffSpecMap;
}

ID3D10ShaderResourceView* CTexture::GetNormalMap()
{
	return m_NormalMap;
}

void CTexture::SendToShader()
{
	if (m_DiffSpecMap && m_DiffSpecMapVar)
	{
		m_DiffSpecMapVar->SetResource(m_DiffSpecMap);
	}
	if (m_NormalMap && m_NormalMapVar)
	{
		m_NormalMapVar->SetResource(m_NormalMap);
	}
}

void CTexture::SetDiffuseSpecularShaderVariable(ID3D10EffectShaderResourceVariable* mapVar)
{
	m_DiffSpecMapVar = mapVar;
}

void CTexture::SetNormalMapShaderVariable(ID3D10EffectShaderResourceVariable* mapVar)
{
	m_NormalMapVar = mapVar;
}