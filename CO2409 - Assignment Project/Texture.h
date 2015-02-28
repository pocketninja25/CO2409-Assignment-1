#ifndef TEXTURE_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define TEXTURE_H_INCLUDED

#include "Defines.h"

class CTexture
{
public:
	CTexture(ID3D10ShaderResourceView* diffSpecMap = NULL, 
		ID3D10ShaderResourceView* normalMap = NULL, 
		float parallaxDepth = 0.0f,
		ID3D10ShaderResourceView* CelGradient = NULL,
		float outlineThickness = 0.015f );

	~CTexture();

	bool LoadDiffSpecMap(wchar_t* mapName);

	bool LoadNormalMap(wchar_t* mapName);

	bool LoadCelGradient(wchar_t* mapName);

	void SetParallaxDepth(float parallaxDepth);

	void SetOutlineThickness(float outlineThickness);

	bool HasNormals();

	void Release();

	ID3D10ShaderResourceView* GetDiffSpecMap();

	ID3D10ShaderResourceView* GetNormalMap();

	ID3D10ShaderResourceView* GetCelGradient();

	void SendToShader();
	
	static void SetDiffuseSpecularShaderVariable(ID3D10EffectShaderResourceVariable* mapVar);

	static void SetNormalMapShaderVariable(ID3D10EffectShaderResourceVariable* mapVar);

	static void SetParallaxDepthShaderVariable(ID3D10EffectScalarVariable* parralaxDepthVar);

	static void SetCelGradientShaderVariable(ID3D10EffectShaderResourceVariable* CelGradientVar);

	static void SetOutlineThicknessShaderVariable(ID3D10EffectScalarVariable* outlineThicknessVar);


private:
	ID3D10ShaderResourceView* m_DiffSpecMap;
	ID3D10ShaderResourceView* m_NormalMap;
	ID3D10ShaderResourceView* m_CelGradient;
	float m_ParallaxDepth;
	float m_OutlineThickness;

	static ID3D10EffectShaderResourceVariable*	m_DiffSpecMapVar;		//Pointer to the shader variable to pass the m_DiffSpecMap to the shader
	static ID3D10EffectShaderResourceVariable*	m_NormalMapVar;			//Pointer to the shader variable to pass the m_NormalMap to the shader
	static ID3D10EffectScalarVariable*			m_ParallaxDepthVar;		//Pointer to the shader variable to pass the m_ParallaxDepth to the shader;
	static ID3D10EffectShaderResourceVariable*	m_CelGradientVar;		//Pointer to the shader variable to pass the m_CelGradient to the shader;
	static ID3D10EffectScalarVariable*			m_OutlineThicknessVar;	//Pointer to the shader variable to pass the m_OutlineThickness to the shader;
};

#endif