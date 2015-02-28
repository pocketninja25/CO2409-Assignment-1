#include "PositionalLight.h"

void CPositionalLight::SetColourVar(ID3D10EffectVectorVariable* colourVar)
{
	m_ColourVar = colourVar;
}
void CPositionalLight::SetPositionVar(ID3D10EffectVectorVariable* positionVar)
{
	m_PositionVar = positionVar;
}
void CPositionalLight::SetSpecularPowerVar(ID3D10EffectScalarVariable* specularPowerVar)
{
	m_SpecularPowerVar = specularPowerVar;
}


CPositionalLight::CPositionalLight(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour, D3DXVECTOR3 position, float scale, bool isStationary, float specularPower) :
		m_DiffuseColour(diffuseColour),
		m_SpecularColour(specularColour),
		m_Model(CModel(position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), scale, m_DiffuseColour)),
		m_IsStationary(isStationary),
		m_SpecularPower(specularPower)
{
}

CPositionalLight::~CPositionalLight()
{
}

bool CPositionalLight::LoadModel(const string& fileName, ID3D10EffectTechnique* shaderCode, bool tangents)
{
	return m_Model.Load(fileName, shaderCode, tangents);
}

void CPositionalLight::UpdateMatrix()	//Call model update matrix
{
	m_Model.UpdateMatrix();
}

void CPositionalLight::LightRender(D3DXVECTOR3 colour)
{
	m_ColourVar->SetRawValue(colour, 0, sizeof(D3DXVECTOR3));
	m_PositionVar->SetRawValue(m_Model.GetPosition(), 0, sizeof(D3DXVECTOR3));
	m_SpecularPowerVar->SetFloat(m_SpecularPower);
}


void CPositionalLight::ModelRender(D3DXVECTOR3 colour)			//Call model render
{
	m_Model.SetColour(colour);
	m_Model.Render();
}