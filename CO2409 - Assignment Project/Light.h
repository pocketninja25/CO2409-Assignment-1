//--------------------------------------------------------------------------------------
//	Light.h
//
//	The light class collects together a lights's model data and colour/specular power
//	values
//--------------------------------------------------------------------------------------

#ifndef LIGHT_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define LIGHT_H_INCLUDED

//Ambient light
//Point light
//Spot light
//Directional light
#include "Defines.h"

	//float m_SpecularPower;

#include "Model.h"

class CPositionalLight
{
private:
	CModel m_Model;
	D3DXVECTOR3 m_DiffuseColour;
	D3DXVECTOR3 m_SpecularColour;
	float m_SpecularPower;
	bool m_IsStationary;

public:
	CPositionalLight(D3DXVECTOR3 diffuseColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3 specularColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f), 
		D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f), float scale = 0.0f, bool isStationary = false, float specularPower = 0.0f);
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
	float GetSpecularPower()
	{
		return m_SpecularPower;
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
	void SetSpecularPower(float specularPower)
	{
		m_SpecularPower = specularPower;
	}
	void SetScale(float scale)
	{
		m_Model.SetScale(scale);
	}

	// Other
	bool LoadModel(const string& fileName, ID3D10EffectTechnique* shaderCode, bool tangents = false);

	void UpdateMatrix();								//Call model update matrix

	void Render(D3DXVECTOR3 colour = D3DXVECTOR3(0.0f, 0.0f, 0.0f));		//Call model render
};

//************************************************
// CAmbientLight
//************************************************

class CAmbientLight
{
private:
	D3DXVECTOR3 m_Colour;

public:
	CAmbientLight(D3DXVECTOR3 colour = D3DXVECTOR3(0.0f, 0.0f, 0.0f));

	// Getters

	D3DXVECTOR3 GetColour()
	{
		return m_Colour;
	}

	// Setters
	void SetColour(D3DXVECTOR3 colour)
	{
		m_Colour = colour;
	}
};

#endif
