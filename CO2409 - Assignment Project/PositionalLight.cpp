#include "PositionalLight.h"

void CPositionalLight::SetDiffuseColourVar(ID3D10EffectVectorVariable* colourVar)
{
	m_DiffuseColourVar = colourVar;
}
void CPositionalLight::SetSpecularColourVar(ID3D10EffectVectorVariable* colourVar)
{
	m_SpecularColourVar = colourVar;
}
void CPositionalLight::SetPositionVar(ID3D10EffectVectorVariable* positionVar)
{
	m_PositionVar = positionVar;
}

CPositionalLight::CPositionalLight(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour, D3DXVECTOR3 position, float scale, bool isStationary) :
		m_DiffuseColour(diffuseColour),
		m_SpecularColour(specularColour),
		m_Model(CModel(position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), scale, m_DiffuseColour)),
		m_IsStationary(isStationary)
{
}

CPositionalLight::~CPositionalLight()
{
}

bool CPositionalLight::LoadModel(const string& fileName, CTechnique* shaderCode)
{
	return m_Model.Load(fileName, shaderCode);
}

void CPositionalLight::SetMaterial(CMaterial* material)
{
	m_Model.SetMaterial(material);
}

void CPositionalLight::UpdateMatrix()	//Call model update matrix
{
	m_Model.UpdateMatrix();
}

void CPositionalLight::LightRender()
{
	LightRender(m_DiffuseColour, m_SpecularColour);
}

void CPositionalLight::LightRender(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour)
{
	m_DiffuseColourVar->SetRawValue(diffuseColour, 0, sizeof(D3DXVECTOR3));
	m_SpecularColourVar->SetRawValue(specularColour, 0, sizeof(D3DXVECTOR3));
	m_PositionVar->SetRawValue(m_Model.GetPosition(), 0, sizeof(D3DXVECTOR3));
}

void CPositionalLight::ModelRender(D3DXVECTOR3 colour)			//Call model render
{
	m_Model.SetColour(colour);
	m_Model.Render();
}

void CPositionalLight::Control(float frameTime, EKeyCode turnUp, EKeyCode turnDown, EKeyCode turnLeft, EKeyCode turnRight,
	EKeyCode turnCW, EKeyCode turnCCW, EKeyCode moveForward, EKeyCode moveBackward)
{
	m_Model.Control(frameTime, turnUp, turnDown, turnLeft, turnRight, turnCW, turnCCW, moveForward, moveBackward);
}