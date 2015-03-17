//--------------------------------------------------------------------------------------
//	PositionalLight.h
//
//	The light class collects together a lights's model data and colour/specular power
//	values
//--------------------------------------------------------------------------------------

#ifndef POSITIONAL_LIGHT_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define POSITIONAL_LIGHT_H_INCLUDED

#include "Defines.h"

#include "Model.h"
#include "Technique.h"

class CPositionalLight
{
private:
	D3DXVECTOR3 m_DiffuseColour;
	D3DXVECTOR3 m_SpecularColour;
	bool m_IsStationary;

	ID3D10EffectVectorVariable* m_DiffuseColourVar;
	ID3D10EffectVectorVariable* m_SpecularColourVar;
	ID3D10EffectVectorVariable* m_PositionVar;

protected:
	CModel m_Model;

public:
	void SetDiffuseColourVar(ID3D10EffectVectorVariable* colourVar);
	void SetSpecularColourVar(ID3D10EffectVectorVariable* colourVar);
	void SetPositionVar(ID3D10EffectVectorVariable* positionVar);

	CPositionalLight(D3DXVECTOR3 diffuseColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3 specularColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f), 
		D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f), float scale = 0.0f, bool isStationary = false);
	~CPositionalLight();

	// Getters
	D3DXVECTOR3 GetDiffuseColour()
	{
		return m_DiffuseColour;
	}
	D3DXVECTOR3 GetSpecularColour()
	{
		return m_SpecularColour;
	}
	D3DXVECTOR3 GetPosition()
	{
		return m_Model.GetPosition();
	}
	bool IsStationary()
	{
		return m_IsStationary;
	}
	D3DXVECTOR3 GetScale()
	{
		return m_Model.GetScale();
	}
	D3DXMATRIX GetWorldMatrix()
	{
		return m_Model.GetWorldMatrix();
	}

	// Setters
	void SetDiffuseColour(D3DXVECTOR3 diffuseColour)
	{
		m_DiffuseColour = diffuseColour;
	}
	void SetSpecularColour(D3DXVECTOR3 specularColour)
	{
		m_SpecularColour = specularColour;
	}
	void SetPosition(D3DXVECTOR3 position)
	{
		m_Model.SetPosition(position);
	}
	void SetIsStationary(bool isStationary)
	{
		m_IsStationary = isStationary;
	}
	void SetScale(float scale)
	{
		m_Model.SetScale(scale);
	}
	void SetRotation(D3DXVECTOR3 rotation)
	{
		m_Model.SetRotation(rotation);
	}

	// Other
	bool LoadModel(const string& fileName, CTechnique* shaderCode);

	void SetMaterial(CMaterial* texture);

	void UpdateMatrix();								//Call model update matrix

	void LightRender();	//Uses the member value of diffuse and specular light
	void LightRender(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour);		
	
	void ModelRender(D3DXVECTOR3 colour = D3DXVECTOR3(0.0f, 0.0f, 0.0f));		//Call model render

	void Control(float frameTime, EKeyCode turnUp, EKeyCode turnDown, EKeyCode turnLeft, EKeyCode turnRight,
		EKeyCode turnCW, EKeyCode turnCCW, EKeyCode moveForward, EKeyCode moveBackward);
};

#endif
