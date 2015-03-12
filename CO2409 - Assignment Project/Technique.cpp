#include "Technique.h"


CTechnique::CTechnique(ID3D10EffectTechnique* technique, bool diffuseMap, bool bumpMap, bool CelGradient) :
	m_Technique(technique),
	m_RequiresDiffuseMap(diffuseMap),
	m_RequiresBumpMap(bumpMap),
	m_RequiresCelGradient(CelGradient)
{
}


CTechnique::~CTechnique()
{
}

bool CTechnique::IsCompatible(CMaterial* material)
{
	//X kinds of path
	//RequireMap && have a map - fine
	//RequireMap && dont have a map - bad
	//Dont RequireMap && have a map - fine
	//Dont RequireMap && dont have a map - fine
	// There is only one path that does not return 'okay' - test for this path on each variable type
	
	if (!material)	//Check that the texture provided is not null
	{
		return false;
	}

	if (m_RequiresDiffuseMap && !material->GetDiffSpecMap())	//If the technique requires a diffuse map but the texture does not have a diffuseMap
	{
		return false;
	}
	if (m_RequiresBumpMap && !material->GetNormalMap())	//If the technique requires a bump map but the texture does not have a bump map
	{
		return false;
	}
	if (m_RequiresCelGradient && !material->GetCelGradient())	//If the technique requires a Cel shading gradient but the texture does not have one
	{
		return false;
	}

	return true; //The texture has passed all of the requirements
}