#ifndef TEXTURE_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define TEXTURE_H_INCLUDED

#include "Defines.h"

class CTexture
{
public:
	CTexture(ID3D10ShaderResourceView* map = NULL);
	~CTexture();

	bool LoadTexture(wchar_t* textureName);

	void Release();

	ID3D10ShaderResourceView* GetTexture();

	void SendToShader();
	
	static void SetShaderVariable(ID3D10EffectShaderResourceVariable* mapVar);

private:
	ID3D10ShaderResourceView* m_Map;

	static ID3D10EffectShaderResourceVariable* m_MapVar;	//Pointer to the shader variable to pass the m_Map to the shader
};

#endif