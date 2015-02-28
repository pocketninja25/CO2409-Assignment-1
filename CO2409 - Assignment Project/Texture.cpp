#include "Texture.h"

ID3D10EffectShaderResourceVariable* CTexture::m_DiffSpecMapVar = NULL;
ID3D10EffectShaderResourceVariable* CTexture::m_NormalMapVar = NULL;
ID3D10EffectScalarVariable*			CTexture::m_ParallaxDepthVar = NULL;
ID3D10EffectShaderResourceVariable*	CTexture::m_CelGradientVar = NULL;
ID3D10EffectScalarVariable*			CTexture::m_OutlineThicknessVar = NULL;

CTexture::CTexture(ID3D10ShaderResourceView* diffSpecMap,
	ID3D10ShaderResourceView* normalMap, float parallaxDepth,
	ID3D10ShaderResourceView* CelGradient, float outlineThickness) :
	m_DiffSpecMap(diffSpecMap),
	m_NormalMap(normalMap),
	m_ParallaxDepth(parallaxDepth),
	m_CelGradient(CelGradient),
	m_OutlineThickness(outlineThickness)
{
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

bool CTexture::LoadCelGradient(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_CelGradient, NULL));
}

void CTexture::SetParallaxDepth(float parallaxDepth)
{
	m_ParallaxDepth = parallaxDepth;
}

void CTexture::SetOutlineThickness(float outlineThickness)
{
	m_OutlineThickness = outlineThickness;
}

bool CTexture::HasNormals()
{
	return m_NormalMap;
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
		m_NormalMap = NULL;
	}
	if (m_CelGradient)
	{
		m_CelGradient->Release();
		m_CelGradient = NULL;
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

ID3D10ShaderResourceView* CTexture::GetCelGradient()
{
	return m_CelGradient;
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
	if (m_ParallaxDepthVar)
	{
		m_ParallaxDepthVar->SetFloat(m_ParallaxDepth);
	}
	if (m_CelGradientVar)
	{
		m_CelGradientVar->SetResource(m_CelGradient);
	}
	if (m_OutlineThicknessVar)
	{
		m_OutlineThicknessVar->SetFloat(m_OutlineThickness);
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

void CTexture::SetParallaxDepthShaderVariable(ID3D10EffectScalarVariable* parallaxDepthVar)
{
	m_ParallaxDepthVar = parallaxDepthVar;
}

void CTexture::SetCelGradientShaderVariable(ID3D10EffectShaderResourceVariable* CelGradientVar)
{
	m_CelGradientVar = CelGradientVar;
}

void CTexture::SetOutlineThicknessShaderVariable(ID3D10EffectScalarVariable* outlineThicknessVar)
{
	m_OutlineThicknessVar = outlineThicknessVar;
}