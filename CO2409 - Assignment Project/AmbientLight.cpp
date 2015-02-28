#include "AmbientLight.h"

ID3D10EffectVectorVariable* CAmbientLight::m_ColourVar = nullptr;

CAmbientLight::CAmbientLight(D3DXVECTOR3 colour) :
m_Colour(colour)
{
}

void CAmbientLight::SetColourVar(ID3D10EffectVectorVariable* colourVar)
{
	m_ColourVar = colourVar;
}

void CAmbientLight::LightRender()
{
	m_ColourVar->SetFloatVector(m_Colour);
}