#ifndef SPOT_LIGHT_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define SPOT_LIGHT_H_INCLUDED

#include "PositionalLight.h"

class CSpotLight : public CPositionalLight
{
private:
	float m_ConeAngle;
	ID3D10EffectScalarVariable* m_ConeAngleVar;
	ID3D10EffectVectorVariable* m_FacingVectorVar;
	ID3D10EffectMatrixVariable* m_ViewMatrixVar;
	ID3D10EffectMatrixVariable* m_ProjMatrixVar;
	ID3D10EffectMatrixVariable* m_ViewProjMatrixVar;
	ID3D10EffectShaderResourceVariable* m_ShadowMapVar;



	static unsigned int m_ShadowMapSize;

	ID3D10Texture2D*          m_ShadowMapTexture;
	ID3D10DepthStencilView*   m_ShadowMapDepthView;
	ID3D10ShaderResourceView* m_ShadowMap;

public:
	CSpotLight(D3DXVECTOR3 diffuseColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3 specularColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f),
		D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f), float coneAngle = 90.0f, float scale = 0.0f);
	~CSpotLight();

	bool CreateShadowMap(ID3D10Device* Device);
	
	void RenderShadowMap(ID3D10Device* Device, vector<CModel*> &models, ID3D10EffectMatrixVariable* viewProjMatrixVar, bool useNewRenderTarget = true);

	void SetConeAngleVar(ID3D10EffectScalarVariable* coneAngleVar);
	void SetFacingVectorVar(ID3D10EffectVectorVariable* facingVectorVar);
	void SetViewMatrixVar(ID3D10EffectMatrixVariable* viewMatrixVar);
	void SetProjMatrixVar(ID3D10EffectMatrixVariable* projMatrixVar);
	void SetViewProjMatrixVar(ID3D10EffectMatrixVariable* viewProjMatrixVar);
	void SetShadowMapVar(ID3D10EffectShaderResourceVariable* shadowMapVar);

	void SetConeAngle(float coneAngle = 90.0f);

	void FacePoint(D3DXVECTOR3 point);

	D3DXMATRIX GetViewMatrix();
	D3DXMATRIX GetProjectionMatrix();

	void LightRender();
	void LightRender(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour);


};

#endif