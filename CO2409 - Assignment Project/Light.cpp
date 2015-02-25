#include "Light.h"



CPositionalLight::CPositionalLight(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour, D3DXVECTOR3 position, float scale, bool isStationary, float specularPower) :
		m_DiffuseColour(diffuseColour),
		m_SpecularColour(specularColour),
		m_Model(CModel(position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), scale)),
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

void CPositionalLight::Render(D3DXVECTOR3 colour)			//Call model render
{
	m_Model.Render(colour);
}

//************************************************
// CAmbientLight
//************************************************

CAmbientLight::CAmbientLight(D3DXVECTOR3 colour) :
	m_Colour(colour)
{
}