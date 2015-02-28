//--------------------------------------------------------------------------------------
//	AmbientLight.h
//
//	The light class collects together a the ambient lights data
//
//--------------------------------------------------------------------------------------
#ifndef AMBIENT_LIGHT_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define AMBIENT_LIGHT_H_INCLUDED

#include "Defines.h"

class CAmbientLight
{
private:
	D3DXVECTOR3 m_Colour;
	static ID3D10EffectVectorVariable* m_ColourVar;

public:
	static void SetColourVar(ID3D10EffectVectorVariable* colourVar);

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

	void LightRender();

};

#endif