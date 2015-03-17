#include "SpotLight.h"

unsigned int CSpotLight::m_ShadowMapSize = 1024;

CSpotLight::CSpotLight(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour,
	D3DXVECTOR3 position, float coneAngle, float scale) :
	CPositionalLight(diffuseColour, specularColour, position, scale, false),
	m_ConeAngleVar(NULL),
	m_FacingVectorVar(NULL),
	m_ViewMatrixVar(NULL),
	m_ProjMatrixVar(NULL),
	m_ViewProjMatrixVar(NULL),
	m_ShadowMapVar(NULL),
	m_ShadowMapTexture(NULL),
	m_ShadowMapDepthView(NULL),
	m_ShadowMap(NULL)
{
}

void CSpotLight::SetConeAngleVar(ID3D10EffectScalarVariable* coneAngleVar)
{
	m_ConeAngleVar = coneAngleVar;
}
void CSpotLight::SetFacingVectorVar(ID3D10EffectVectorVariable* facingVectorVar)
{
	m_FacingVectorVar = facingVectorVar;
}
void CSpotLight::SetViewMatrixVar(ID3D10EffectMatrixVariable* viewMatrixVar)
{
	m_ViewMatrixVar = viewMatrixVar;
}
void CSpotLight::SetProjMatrixVar(ID3D10EffectMatrixVariable*  projMatrixVar)
{
	m_ProjMatrixVar = projMatrixVar;
}
void CSpotLight::SetViewProjMatrixVar(ID3D10EffectMatrixVariable* viewProjMatrixVar)
{
	m_ViewProjMatrixVar = viewProjMatrixVar;
}
void CSpotLight::SetShadowMapVar(ID3D10EffectShaderResourceVariable* shadowMapVar)
{
	m_ShadowMapVar = shadowMapVar;
}

CSpotLight::~CSpotLight()
{
	if (m_ShadowMap)			m_ShadowMap->Release();
	if (m_ShadowMapDepthView)	m_ShadowMapDepthView->Release();
	if (m_ShadowMapTexture)		m_ShadowMapTexture->Release();
}

bool CSpotLight::CreateShadowMap(ID3D10Device* Device)
{
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_ShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
	texDesc.Height = m_ShadowMapSize;
	texDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and texture see things slightly differently]
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as render target, and will also pass it to shaders
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	if (FAILED(Device->CreateTexture2D(&texDesc, NULL, &m_ShadowMapTexture))) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	if (FAILED(Device->CreateDepthStencilView(m_ShadowMapTexture, &descDSV, &m_ShadowMapDepthView))) return false;

	// We also need to send this texture (a GPU memory resource) to the shaders. To do that we must create a shader-resource "view"	
	D3D10_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above
	srDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED(Device->CreateShaderResourceView(m_ShadowMapTexture, &srDesc, &m_ShadowMap))) return false;

	//Not returned yet, return true
	return true;
}

void CSpotLight::RenderShadowMap(ID3D10Device* Device, vector<CModel*> &models, ID3D10EffectMatrixVariable* camViewProjMatrixVar, bool useNewRenderTarget)
{
	if (useNewRenderTarget)
	{
		// Setup the viewport - defines which part of the shadow map we will render to (usually all of it)
		D3D10_VIEWPORT vp;
		vp.Width = m_ShadowMapSize;
		vp.Height = m_ShadowMapSize;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Device->RSSetViewports(1, &vp);

		// Rendering a single shadow map for a light
		// 1. Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
		// 2. Clear the shadow map texture (as a depth buffer)
		// 3. Render everything from point of view of light 0
		Device->OMSetRenderTargets(0, 0, m_ShadowMapDepthView);
		Device->ClearDepthStencilView(m_ShadowMapDepthView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	}

	//Send the relevant values to the shader (common settings ViewProjMatrix and model matrices)

	//Send the viewProj matrix of the spotlight to the shader
	D3DXMATRIX matrix = m_Model.GetWorldMatrix();
	D3DXMATRIX viewMatrix;
	D3DXMatrixInverse(&viewMatrix, NULL, &matrix);
	D3DXMATRIX projMatrix;
	D3DXMatrixPerspectiveFovLH(&projMatrix, ToRadians(m_ConeAngle), 1, 1.0f, 1000.0f);
	D3DXMATRIX viewProjMatrix = viewMatrix * projMatrix;

	camViewProjMatrixVar->SetMatrix(viewProjMatrix);

	for (unsigned int i = 0; i < models.size(); i++)
	{
		models[i]->ShadowRender(/*DepthOnlyTechnique*/);
	}
}

void CSpotLight::SetConeAngle(float coneAngle)
{
	m_ConeAngle = coneAngle;
}

void CSpotLight::FacePoint(D3DXVECTOR3 point)
{
	if (!IsStationary())	//If the light is not stationary then move the model position
	{
		m_Model.FacePoint(point);
	}
}

D3DXMATRIX CSpotLight::GetViewMatrix()
{
	D3DXMATRIX viewMatrix;

	// Get the world matrix of the light model and invert it to get the view matrix (that is more-or-less the definition of a view matrix)
	// We don't always have a physical model for a light, in which case we would need to store this data along with the light colour etc.
	D3DXMATRIXA16 worldMatrix = m_Model.GetWorldMatrix();
	D3DXMatrixInverse(&viewMatrix, NULL, &worldMatrix);

	return viewMatrix;
}

D3DXMATRIX CSpotLight::GetProjectionMatrix()
{
	D3DXMATRIX projMatrix;

	// Create a projection matrix for the light. Use the spotlight cone angle as an FOV, just set default values for everything else.
	D3DXMatrixPerspectiveFovLH(&projMatrix, ToRadians(m_ConeAngle), 1, 0.1f, 1000.0f);

	return projMatrix;
}

void CSpotLight::LightRender()
{
	LightRender(GetDiffuseColour(), GetSpecularColour());
}

void CSpotLight::LightRender(D3DXVECTOR3 diffuseColour, D3DXVECTOR3 specularColour)
{
	//Call parent light render (to send the typical values)
	CPositionalLight::LightRender(diffuseColour, specularColour);

	//Send the cone angle to the shader
	m_ConeAngleVar->SetFloat(cosf(ToRadians(m_ConeAngle * 0.5f)));

	//Send the facing vector of the spotlight to the shader
	m_FacingVectorVar->SetRawValue(m_Model.GetFacingVector(), 0, sizeof(D3DXVECTOR3));

	//Send the viewProj matrix of the spotlight to the shader
	D3DXMATRIX matrix = m_Model.GetWorldMatrix();
	D3DXMATRIX viewMatrix;
	D3DXMatrixInverse(&viewMatrix, NULL, &matrix);
	D3DXMATRIX projMatrix;
	D3DXMatrixPerspectiveFovLH(&projMatrix, ToRadians(m_ConeAngle), 1, 0.1f, 1000.0f);
	D3DXMATRIX viewProjMatrix = viewMatrix * projMatrix;

	m_ViewMatrixVar->SetMatrix(viewMatrix);
	m_ProjMatrixVar->SetMatrix(projMatrix);
	m_ViewProjMatrixVar->SetMatrix(viewProjMatrix);


	//Send the shadow map to the shader
	m_ShadowMapVar->SetResource(m_ShadowMap);

}

