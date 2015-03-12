#ifndef TECHNIQUE_H_INCLUDED
#define TECHNIQUE_H_INCLUDED

#include "Defines.h"
#include "Material.h"

class CTechnique
{
private:
	ID3D10EffectTechnique* m_Technique;
	bool m_RequiresDiffuseMap;
	bool m_RequiresBumpMap;
	bool m_RequiresCelGradient;

public:
	CTechnique(ID3D10EffectTechnique* technique, bool diffuseMap = false, bool bumpMap = false, bool CelGradient = false);
	~CTechnique();

	ID3D10EffectTechnique* GetTechnique()
	{
		return m_Technique;
	}

	bool IsCompatible(CMaterial* material);
};

#endif