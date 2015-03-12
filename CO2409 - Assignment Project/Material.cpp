#include "Material.h"

ID3D10EffectShaderResourceVariable* CMaterial::m_DiffSpecMapVar = NULL;
ID3D10EffectScalarVariable*			CMaterial::m_SpecularPowerVar = NULL;
ID3D10EffectShaderResourceVariable* CMaterial::m_NormalMapVar = NULL;
ID3D10EffectScalarVariable*			CMaterial::m_ParallaxDepthVar = NULL;
ID3D10EffectShaderResourceVariable*	CMaterial::m_CelGradientVar = NULL;
ID3D10EffectScalarVariable*			CMaterial::m_OutlineThicknessVar = NULL;


CMaterial::CMaterial(ID3D10ShaderResourceView* diffSpecMap, float specularPower,
	ID3D10ShaderResourceView* normalMap, float parallaxDepth,
	ID3D10ShaderResourceView* CelGradient, float outlineThickness) :
	m_DiffSpecMap(diffSpecMap),
	m_SpecularPower(specularPower),
	m_NormalMap(normalMap),
	m_ParallaxDepth(parallaxDepth),
	m_CelGradient(CelGradient),
	m_OutlineThickness(outlineThickness)
{
}

CMaterial::~CMaterial()
{
	Release();
}

bool CMaterial::LoadDiffSpecMap(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_DiffSpecMap, NULL));
}

bool CMaterial::LoadNormalMap(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_NormalMap, NULL));
}

bool CMaterial::LoadCelGradient(wchar_t* mapName)
{
	return !FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, mapName, NULL, NULL, &m_CelGradient, NULL));
}

void CMaterial::SetSpecularPower(float specularPower)
{
	m_SpecularPower = specularPower;
}

void CMaterial::SetParallaxDepth(float parallaxDepth)
{
	m_ParallaxDepth = parallaxDepth;
}

void CMaterial::SetOutlineThickness(float outlineThickness)
{
	m_OutlineThickness = outlineThickness;
}

bool CMaterial::HasNormals()
{
	return m_NormalMap;
}

void CMaterial::Release()
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

ID3D10ShaderResourceView* CMaterial::GetDiffSpecMap()
{
	return m_DiffSpecMap;
}

ID3D10ShaderResourceView* CMaterial::GetNormalMap()
{
	return m_NormalMap;
}

ID3D10ShaderResourceView* CMaterial::GetCelGradient()
{
	return m_CelGradient;
}

void CMaterial::SendToShader()
{
	if (m_DiffSpecMap && m_DiffSpecMapVar)
	{
		m_DiffSpecMapVar->SetResource(m_DiffSpecMap);
	}
	if (m_SpecularPowerVar)
	{
		m_SpecularPowerVar->SetFloat(m_SpecularPower);
	}
	if (m_NormalMap && m_NormalMapVar)
	{
		m_NormalMapVar->SetResource(m_NormalMap);
	}
	if (m_ParallaxDepthVar)
	{
		m_ParallaxDepthVar->SetFloat(m_ParallaxDepth);
	}
	if (m_CelGradient && m_CelGradientVar)
	{
		m_CelGradientVar->SetResource(m_CelGradient);
	}
	if (m_OutlineThicknessVar)
	{
		m_OutlineThicknessVar->SetFloat(m_OutlineThickness);
	}
}

void CMaterial::SetDiffuseSpecularShaderVariable(ID3D10EffectShaderResourceVariable* mapVar)
{
	m_DiffSpecMapVar = mapVar;
}

void CMaterial::SetSpecularPowerShaderVariable(ID3D10EffectScalarVariable* specularPowerVar)
{
	m_SpecularPowerVar = specularPowerVar;
}

void CMaterial::SetNormalMapShaderVariable(ID3D10EffectShaderResourceVariable* mapVar)
{
	m_NormalMapVar = mapVar;
}

void CMaterial::SetParallaxDepthShaderVariable(ID3D10EffectScalarVariable* parallaxDepthVar)
{
	m_ParallaxDepthVar = parallaxDepthVar;
}

void CMaterial::SetCelGradientShaderVariable(ID3D10EffectShaderResourceVariable* CelGradientVar)
{
	m_CelGradientVar = CelGradientVar;
}

void CMaterial::SetOutlineThicknessShaderVariable(ID3D10EffectScalarVariable* outlineThicknessVar)
{
	m_OutlineThicknessVar = outlineThicknessVar;
}