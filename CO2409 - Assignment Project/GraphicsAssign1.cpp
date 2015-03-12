// **%** is the symbol of code that needs to be redone/undone before submission that might be easily missed - check entire solution - including .fx file


//--------------------------------------------------------------------------------------
//	GraphicsAssign1.cpp
//
//	Shaders Graphics Assignment
//	Add further models using different shader techniques
//	See assignment specification for details
//--------------------------------------------------------------------------------------

//***|  INFO  |****************************************************************
// Lights:
//   The initial project shows models for a couple of point lights, but the
//   initial shaders don't actually apply any lighting. Part of the assignment
//   is to add a shader that uses this information to actually light a model.
//   Refer to lab work to determine how best to do this.
// 
// Textures:
//   The models in the initial scene have textures loaded but the initial
//   technique/shaders doesn't show them. Part of the assignment is to add 
//   techniques to show a textured model
//
// Shaders:
//   The initial shaders render plain coloured objects with no lighting:
//   - The vertex shader performs basic transformation of 3D vertices to 2D
//   - The pixel shader colours every pixel the same colour
//   A few shader variables are set up to communicate between C++ and HLSL
//   You will usually need to add further variables to add new techniques
//*****************************************************************************

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <atlbase.h>
#include "resource.h"

#include <vector>

#include "Defines.h"			// General definitions shared by all source files
#include "Model.h"				// Model class - encapsulates working with vertex/index data and world matrix
#include "PositionalLight.h"	// Light class - encapsulates lighting colour/position etc
#include "AmbientLight.h"
#include "Camera.h"				// Camera class - encapsulates the camera's view and projection matrix
#include "Input.h"				// Input functions - not DirectX
#include "Material.h"
#include "ColourConversion.h"
#include "Technique.h"
//--------------------------------------------------------------------------------------
// Global Scene Variables
//--------------------------------------------------------------------------------------
const unsigned int NO_OF_LIGHTS = 3;

// Models and cameras encapsulated in classes for flexibity and convenience
// The CModel class collects together geometry and world matrix, and provides functions to control the model and render it
// The CCamera class handles the view and projections matrice, and provides functions to control the camera

vector<CModel*> g_Models;

CCamera* Camera = NULL; 

// Additional Light data
D3DXVECTOR3 PulsingLightColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

// Light Class and orbit data
CPositionalLight* Lights[NO_OF_LIGHTS];
CAmbientLight* AmbientLight = NULL;
const float LightOrbitRadius = 20.0f;
const float LightOrbitSpeed  = 0.5f;

//Misc values
float Wiggle = 0.0f;
float PulseTime = 0.0f;

// Note: There are move & rotation speed constants in Defines.h

//--------------------------------------------------------------------------------------
// Shader Variables
//--------------------------------------------------------------------------------------
// Variables to connect C++ code to HLSL shaders

// Effects / techniques
ID3D10Effect* Effect = NULL;

CTechnique* PlainColourTechnique = NULL;
CTechnique* DiffuseTexturedTechnique = NULL;
CTechnique* WiggleAndScrollTechnique = NULL;
CTechnique* PixelLightingTechnique = NULL;
CTechnique* NormalMapTechnique = NULL;
CTechnique* ParallaxMapTechnique = NULL;
CTechnique* CelShadingTechnique = NULL;
CTechnique* AdditiveTexTintTechnique = NULL;
CTechnique* ParallaxCelShadeTechnique = NULL;
CTechnique* AlphaCutoutTechnique = NULL;
CTechnique* ParallaxOutlinedTechnique = NULL;
CTechnique* PixelLitOutlinedTechnique = NULL;

// Matrices
ID3D10EffectMatrixVariable* ViewMatrixVar = NULL;
ID3D10EffectMatrixVariable* ProjMatrixVar = NULL;
ID3D10EffectMatrixVariable* ViewProjMatrixVar = NULL;

// Lighting
ID3D10EffectVectorVariable* CameraPositionVar = NULL;

// Miscellaneous
ID3D10EffectScalarVariable* WiggleVar = NULL;


//--------------------------------------------------------------------------------------
// DirectX Variables
//--------------------------------------------------------------------------------------

// The main D3D interface, this pointer is used to access most D3D functions (and is shared across all cpp files through Defines.h)
ID3D10Device* g_pd3dDevice = NULL;

// Width and height of the window viewport
int g_ViewportWidth;
int g_ViewportHeight;

// Variables used to setup D3D
IDXGISwapChain*         SwapChain = NULL;
ID3D10Texture2D*        DepthStencil = NULL;
ID3D10DepthStencilView* DepthStencilView = NULL;
ID3D10RenderTargetView* RenderTargetView = NULL;

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
bool InitDevice(HWND hWnd)
{
	// Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
	// the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
	HRESULT hr = S_OK;


	////////////////////////////////a
	// Initialise Direct3D

	// Calculate the visible area the window we are using - the "client rectangle" refered to in the first function is the 
	// size of the interior of the window, i.e. excluding the frame and title
	RECT rc;
	GetClientRect(hWnd, &rc);
	g_ViewportWidth = rc.right - rc.left;
	g_ViewportHeight = rc.bottom - rc.top;

	// Create a Direct3D device (i.e. initialise D3D), and create a swap-chain (create a back buffer to render to)
	DXGI_SWAP_CHAIN_DESC sd;         // Structure to contain all the information needed
	ZeroMemory( &sd, sizeof( sd ) ); // Clear the structure to 0 - common Microsoft practice, not really good style
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_ViewportWidth;             // Target window size
	sd.BufferDesc.Height = g_ViewportHeight;           // --"--
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
	sd.BufferDesc.RefreshRate.Numerator = 60;          // Refresh rate of monitor
	sd.BufferDesc.RefreshRate.Denominator = 1;         // --"--
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.OutputWindow = hWnd;                            // Target window
	sd.Windowed = TRUE;                                 // Whether to render in a window (TRUE) or go fullscreen (FALSE)
	hr = D3D10CreateDeviceAndSwapChain( NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0,
										D3D10_SDK_VERSION, &sd, &SwapChain, &g_pd3dDevice );
	if( FAILED( hr ) ) return false;


	// Specify the render target as the back-buffer - this is an advanced topic. This code almost always occurs in the standard D3D setup
	ID3D10Texture2D* pBackBuffer;
	hr = SwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBackBuffer );
	if( FAILED( hr ) ) return false;
	hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &RenderTargetView );
	pBackBuffer->Release();
	if( FAILED( hr ) ) return false;


	// Create a texture (bitmap) to use for a depth buffer
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width = g_ViewportWidth;
	descDepth.Height = g_ViewportHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &DepthStencil );
	if( FAILED( hr ) ) return false;

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView( DepthStencil, &descDSV, &DepthStencilView );
	if( FAILED( hr ) ) return false;

	// Select the back buffer and depth buffer to use for rendering now
	g_pd3dDevice->OMSetRenderTargets( 1, &RenderTargetView, DepthStencilView );


	// Setup the viewport - defines which part of the window we will render to, almost always the whole window
	D3D10_VIEWPORT vp;
	vp.Width  = g_ViewportWidth;
	vp.Height = g_ViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports( 1, &vp );

	return true;
}

//--------------------------------------------------------------------------------------
// Load and compile Effect file (.fx file containing shaders)
//--------------------------------------------------------------------------------------
// An effect file contains a set of "Techniques". A technique is a combination of vertex, geometry and pixel shaders (and some states) used for
// rendering in a particular way. We load the effect file at runtime (it's written in HLSL and has the extension ".fx"). The effect code is compiled
// *at runtime* into low-level GPU language. When rendering a particular model we specify which technique from the effect file that it will use
//
bool LoadEffectFile()
{
	// Compile and link the effect file
	ID3D10Blob* pErrors; // This strangely typed variable collects any errors when compiling the effect file
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS; // These "flags" are used to set the compiler options

	// Load and compile the effect file
	HRESULT hr = D3DX10CreateEffectFromFile( L"GraphicsAssign1.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0, g_pd3dDevice, NULL, NULL, &Effect, &pErrors, NULL );
	if( FAILED( hr ) )
	{
		if (pErrors != 0)  MessageBox( NULL, CA2CT(reinterpret_cast<char*>(pErrors->GetBufferPointer())), L"Error", MB_OK ); // Compiler error: display error message
		else               MessageBox( NULL, L"Error loading FX file. Ensure your FX file is in the same folder as this executable.", L"Error", MB_OK );  // No error message - probably file not found
		return false;
	}

	// Select techniques from compiled effect file
	PlainColourTechnique =		new CTechnique(Effect->GetTechniqueByName("PlainColour"), false, false, false);
	DiffuseTexturedTechnique =	new CTechnique(Effect->GetTechniqueByName("DiffuseTex"), true, false, false);
	WiggleAndScrollTechnique =	new CTechnique(Effect->GetTechniqueByName("WiggleAndScroll"), true, false, false);
	PixelLightingTechnique =	new CTechnique(Effect->GetTechniqueByName("PixDiffSpec"), true, false, false);
	NormalMapTechnique =		new CTechnique(Effect->GetTechniqueByName("NormalMapping"), true, true, false);
	ParallaxMapTechnique =		new CTechnique(Effect->GetTechniqueByName("ParallaxMapping"), true, true, false);
	CelShadingTechnique =		new CTechnique(Effect->GetTechniqueByName("CelShading"), false, false, true);
	AdditiveTexTintTechnique =  new CTechnique(Effect->GetTechniqueByName("AdditiveTexTint"), true, false, false);
	ParallaxCelShadeTechnique = new CTechnique(Effect->GetTechniqueByName("ParallaxCelShaded"), true, true, true);
	AlphaCutoutTechnique =		new CTechnique(Effect->GetTechniqueByName("AlphaCutout"), true, false, false);
	ParallaxOutlinedTechnique = new CTechnique(Effect->GetTechniqueByName("ParallaxOutlined"), true, true, false);
	PixelLitOutlinedTechnique = new CTechnique(Effect->GetTechniqueByName("PixelLitOutlined"), true, false, false);


	// Push techniques onto the model technique list
	CModel::m_TechniqueList.push_back(PlainColourTechnique);
	CModel::m_TechniqueList.push_back(DiffuseTexturedTechnique);
	CModel::m_TechniqueList.push_back(WiggleAndScrollTechnique);
	CModel::m_TechniqueList.push_back(PixelLightingTechnique);
	CModel::m_TechniqueList.push_back(NormalMapTechnique);
	CModel::m_TechniqueList.push_back(ParallaxMapTechnique);
	CModel::m_TechniqueList.push_back(CelShadingTechnique);
	CModel::m_TechniqueList.push_back(AdditiveTexTintTechnique);
	CModel::m_TechniqueList.push_back(ParallaxCelShadeTechnique);
	CModel::m_TechniqueList.push_back(AlphaCutoutTechnique);
	CModel::m_TechniqueList.push_back(ParallaxOutlinedTechnique);
	CModel::m_TechniqueList.push_back(PixelLitOutlinedTechnique);

	//--------------------------------------------
	// Create links to effect file globals
	//--------------------------------------------
	
	// Matrix data
	CModel::SetMatrixShaderVariable(Effect->GetVariableByName( "WorldMatrix" )->AsMatrix());
	ViewProjMatrixVar =				Effect->GetVariableByName("ViewProjMatrix")->AsMatrix();
	ViewMatrixVar =					Effect->GetVariableByName("ViewMatrix")->AsMatrix();
	ProjMatrixVar =					Effect->GetVariableByName("ProjMatrix")->AsMatrix();

	// Material data
	CMaterial::SetDiffuseSpecularShaderVariable(	Effect->GetVariableByName("DiffuseMap")->AsShaderResource());
	CMaterial::SetSpecularPowerShaderVariable(		Effect->GetVariableByName("SpecularPower")->AsScalar());
	CMaterial::SetNormalMapShaderVariable(			Effect->GetVariableByName("NormalMap")->AsShaderResource());
	CMaterial::SetParallaxDepthShaderVariable(		Effect->GetVariableByName("ParallaxDepth")->AsScalar());
	CMaterial::SetCelGradientShaderVariable(		Effect->GetVariableByName("CelGradient")->AsShaderResource());
	CMaterial::SetOutlineThicknessShaderVariable(	Effect->GetVariableByName("OutlineThickness")->AsScalar());

	// Camera data
	CameraPositionVar = Effect->GetVariableByName("CameraPos")->AsVector();
	
	// Main Lighting data is Assigned in InitScene (So the lighting objects can first be created) - due to there being multiple lighting
	
	// Ambient light data
	CAmbientLight::SetColourVar(Effect->GetVariableByName("AmbientColour")->AsVector());

	// Model data
	CModel::SetColourShaderVariable(Effect->GetVariableByName("ModelColour")->AsVector());
	
	// Other shader variables
	WiggleVar = Effect->GetVariableByName("Wiggle")->AsScalar();

	return true;
}

//--------------------------------------------------------------------------------------
// Scene Setup / Update / Rendering
//--------------------------------------------------------------------------------------

// Create / load the camera, models and Materials for the scene
bool InitScene()
{
	// Create camera
	Camera = new CCamera();
	Camera->SetPosition(D3DXVECTOR3(-15.0f, 20.0f, -40.0f));
	Camera->SetRotation( D3DXVECTOR3(ToRadians(13.0f), ToRadians(18.0f), 0.0f) ); // ToRadians is a new helper function to convert degrees to radians

	// Material initialisation

	// Create Material objects
	CMaterial* StoneMaterial		= new CMaterial();
	CMaterial* WoodMaterial			= new CMaterial();
	CMaterial* GrassMaterial		= new CMaterial();
	CMaterial* BrainMaterial		= new CMaterial();
	CMaterial* PatternMaterial		= new CMaterial();
	CMaterial* CobbleMaterial		= new CMaterial();
	CMaterial* TechMaterial			= new CMaterial();
	CMaterial* WallMaterial			= new CMaterial();
	CMaterial* Troll1Material		= new CMaterial();
	CMaterial* LightMaterial		= new CMaterial();
	CMaterial* ThunderboltMaterial	= new CMaterial();
	CMaterial* FlamesMaterial		= new CMaterial();
		

	// Push Material objects onto Material list
	CModel::m_MaterialList.push_back(StoneMaterial);
	CModel::m_MaterialList.push_back(WoodMaterial);
	CModel::m_MaterialList.push_back(GrassMaterial);
	CModel::m_MaterialList.push_back(BrainMaterial);
	CModel::m_MaterialList.push_back(PatternMaterial);
	CModel::m_MaterialList.push_back(CobbleMaterial);
	CModel::m_MaterialList.push_back(TechMaterial);
	CModel::m_MaterialList.push_back(WallMaterial);
	CModel::m_MaterialList.push_back(Troll1Material);
	CModel::m_MaterialList.push_back(LightMaterial);
	CModel::m_MaterialList.push_back(ThunderboltMaterial);
	CModel::m_MaterialList.push_back(FlamesMaterial);

	
	// Load Texture maps from file and set other material variables
	if (!StoneMaterial->LoadDiffSpecMap(TEXT("StoneDiffuseSpecular.dds")))		return false;
	StoneMaterial->SetSpecularPower(64.0f);
	if (!StoneMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	StoneMaterial->SetOutlineThickness(0.035f);

	if (!WoodMaterial->LoadDiffSpecMap(TEXT("WoodDiffuseSpecular.dds")))		return false;
	WoodMaterial->SetSpecularPower(64.0f);
	if (!WoodMaterial->LoadNormalMap(TEXT("WoodNormal.dds")))					return false;
	WoodMaterial->SetParallaxDepth(0.08f);
	if (!WoodMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	WoodMaterial->SetOutlineThickness(0.035f);

	if (!GrassMaterial->LoadDiffSpecMap(TEXT("GrassDiffuseSpecular.dds")))		return false;
	GrassMaterial->SetSpecularPower(64.0f);
	if (!GrassMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	GrassMaterial->SetOutlineThickness(0.035f);

	if (!BrainMaterial->LoadDiffSpecMap(TEXT("BrainDiffuseSpecular.dds")))		return false;
	BrainMaterial->SetSpecularPower(16.0f);
	if (!BrainMaterial->LoadNormalMap(TEXT("BrainNormalDepth.dds")))			return false;
	BrainMaterial->SetParallaxDepth(0.08f);
	if (!BrainMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	BrainMaterial->SetOutlineThickness(0.035f);

	if (!PatternMaterial->LoadDiffSpecMap(TEXT("PatternDiffuseSpecular.dds")))	return false;
	PatternMaterial->SetSpecularPower(8.0f);
	if (!PatternMaterial->LoadNormalMap(TEXT("PatternNormalDepth.dds")))		return false;
	PatternMaterial->SetParallaxDepth(0.08f);
	if (!PatternMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	PatternMaterial->SetOutlineThickness(0.035f);

	if (!CobbleMaterial->LoadDiffSpecMap(TEXT("CobbleDiffuseSpecular.dds")))	return false;
	CobbleMaterial->SetSpecularPower(64.0f);
	if (!CobbleMaterial->LoadNormalMap(TEXT("CobbleNormalDepth.dds")))			return false;
	CobbleMaterial->SetParallaxDepth(0.08f);
	if (!CobbleMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	CobbleMaterial->SetOutlineThickness(0.035f);

	if (!TechMaterial->LoadDiffSpecMap(TEXT("TechDiffuseSpecular.dds")))		return false;
	TechMaterial->SetSpecularPower(64.0f);
	if (!TechMaterial->LoadNormalMap(TEXT("TechNormalDepth.dds")))				return false;
	TechMaterial->SetParallaxDepth(0.08f);
	if (!TechMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	TechMaterial->SetOutlineThickness(0.035f);

	if (!WallMaterial->LoadDiffSpecMap(TEXT("WallDiffuseSpecular.dds")))		return false;
	WallMaterial->SetSpecularPower(128.0f);
	if (!WallMaterial->LoadNormalMap(TEXT("WallNormalDepth.dds")))				return false;
	WallMaterial->SetParallaxDepth(0.08f);
	if (!WallMaterial->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	WallMaterial->SetOutlineThickness(0.035f);

	if (!Troll1Material->LoadDiffSpecMap(TEXT("Troll3DiffuseSpecular.dds")))	return false;
	Troll1Material->SetSpecularPower(16.0f);
	if (!Troll1Material->LoadCelGradient(TEXT("CelGradient.png")))				return false;
	Troll1Material->SetOutlineThickness(0.035f);
		
	if (!ThunderboltMaterial->LoadDiffSpecMap(TEXT("thdbolt.jpg")))				return false;
	ThunderboltMaterial->SetSpecularPower(2.0f);
	if (!ThunderboltMaterial->LoadCelGradient(TEXT("CelGradient.png")))			return false;

	if (!LightMaterial->LoadDiffSpecMap(TEXT("Flare.jpg")))						return false;
	LightMaterial->SetSpecularPower(64.0f);

	if (!FlamesMaterial->LoadDiffSpecMap(TEXT("flames4.png")))					return false;
	FlamesMaterial->SetSpecularPower(64.0f);

	
	// Model initialisation

	// Load/Create models
	for (int i = 0; i < 7; i++)
	{
		g_Models.push_back(new CModel);
	}

	// Set Model Materials 

	g_Models[0]->SetMaterial(WallMaterial);
	g_Models[1]->SetMaterial(PatternMaterial);
	g_Models[2]->SetMaterial(WoodMaterial);
	g_Models[3]->SetMaterial(StoneMaterial);
	g_Models[4]->SetMaterial(Troll1Material);
	g_Models[5]->SetMaterial(BrainMaterial);
	g_Models[6]->SetMaterial(ThunderboltMaterial);
		
	
	// Set Model Colours
	
	// Constant colours used for models in initial shaders
	D3DXVECTOR3 Black(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 Blue(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 Yellow(1.0f, 1.0f, 0.0f);
	D3DXVECTOR3 Red(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 Green(0.0f, 1.0f, 0.0f);

	g_Models[0]->SetColour(Red);
	g_Models[1]->SetColour(Green);
	g_Models[2]->SetColour(Blue);
	g_Models[3]->SetColour(Yellow);
	g_Models[4]->SetColour(Black);
	g_Models[5]->SetColour(Red);
	g_Models[6]->SetColour(Yellow);	

	
	// Set Render Techniques and load models

	// The model class can load ".X" files. It encapsulates (i.e. hides away from this code) the file loading/parsing and creation of vertex/index buffers
	// We must pass an example technique used for each model. We can then only render models with techniques that uses matching vertex input data
	if (!g_Models[0]->Load(	"Cube.x",	ParallaxMapTechnique		))	return false;
	if (!g_Models[1]->Load(	"Teapot.x", ParallaxMapTechnique		))	return false;
	if (!g_Models[2]->Load(	"Floor.x",	ParallaxMapTechnique		))	return false;
	if (!g_Models[3]->Load(	"Sphere.x", WiggleAndScrollTechnique	))	return false;		
	if (!g_Models[4]->Load(	"Troll.x",	CelShadingTechnique			))	return false;
	if (!g_Models[5]->Load(	"Hills.x",	ParallaxOutlinedTechnique	))	return false;
	if (!g_Models[6]->Load("A10Thunderbolt.x", PixelLitOutlinedTechnique)) return false;
	
	
	// Set Initial Positions/Scales of models
	g_Models[0]->SetPosition(D3DXVECTOR3(0.0f, 10.0f, 0.0f));

	g_Models[1]->SetPosition(D3DXVECTOR3(10.0f, 20.0f, 50.0f));

	g_Models[2]->SetPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	
	g_Models[3]->SetPosition(D3DXVECTOR3(-40.0f, 15.0f, 20.0f));
	g_Models[3]->SetScale(0.7f);

	g_Models[4]->SetPosition(D3DXVECTOR3(-25.0f, 1.0f, 80.0f));
	g_Models[4]->SetScale(10.0f);

	g_Models[5]->SetPosition(D3DXVECTOR3(500.0f, 0.0f, 0.0f));

	g_Models[6]->SetPosition(D3DXVECTOR3(-128.0f, 32.5f, 45.0f));
	g_Models[6]->SetScale(5.0f);
	g_Models[6]->SetRotation(D3DXVECTOR3(ToRadians(15.0f), ToRadians(120.0f), 0.0f));
	

	// Load/Create lights and light models

	AmbientLight =	new CAmbientLight(D3DXVECTOR3(0.2f, 0.2f, 0.2f));
	
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		Lights[i] = new CPositionalLight;
	}

	// Set light shader variables

	//Light Colours
	Lights[0]->SetDiffuseColourVar(Effect->GetVariableByName("Light1DiffuseCol")->AsVector());
	Lights[1]->SetDiffuseColourVar(Effect->GetVariableByName("Light2DiffuseCol")->AsVector());
	Lights[2]->SetDiffuseColourVar(Effect->GetVariableByName("Light3DiffuseCol")->AsVector());
	Lights[0]->SetSpecularColourVar(Effect->GetVariableByName("Light1SpecularCol")->AsVector());
	Lights[1]->SetSpecularColourVar(Effect->GetVariableByName("Light2SpecularCol")->AsVector());
	Lights[2]->SetSpecularColourVar(Effect->GetVariableByName("Light3SpecularCol")->AsVector());
	//Light Positions
	Lights[0]->SetPositionVar(Effect->GetVariableByName("Light1Position")->AsVector());
	Lights[1]->SetPositionVar(Effect->GetVariableByName("Light2Position")->AsVector());
	Lights[2]->SetPositionVar(Effect->GetVariableByName("Light3Position")->AsVector());
	
	// Light Positioning, colour data etc
	Lights[0]->SetPosition( D3DXVECTOR3(30.0f, 10.0f, 0.0f) );
	Lights[0]->SetScale( 4.0f ); // Nice if size of light reflects its brightness
	Lights[0]->SetIsStationary(false);
	Lights[0]->SetDiffuseColour(  D3DXVECTOR3(1.0f, 0.0f, 0.0f) * 10.0f);
	Lights[0]->SetSpecularColour( D3DXVECTOR3(1.0f, 0.0f, 0.0f) * 1.5f);

	Lights[1]->SetPosition( D3DXVECTOR3(-20.0f, 30.0f, 50.0f) );			
	Lights[1]->SetScale( 4.0f );
	Lights[1]->SetIsStationary(true);
	Lights[1]->SetSpecularColour( D3DXVECTOR3(1.0f, 0.0f, 0.7f) * 15.0f);
	Lights[1]->SetDiffuseColour(  D3DXVECTOR3(1.0f, 0.0f, 0.7f) * 15.f);

	//Lights[2]->SetPosition(D3DXVECTOR3(-95.0f, 22.5f, 26.0f));
	Lights[2]->SetPosition(D3DXVECTOR3(-90.0f, 21.0f, 23.1f));
	Lights[2]->SetScale(4.0f);
	Lights[2]->SetRotation(D3DXVECTOR3(ToRadians(-15.0f), ToRadians(300.0f), 0.0f));
	Lights[2]->SetIsStationary(true);
	Lights[2]->SetDiffuseColour(D3DXVECTOR3(1.0f, 0.2f, 0.0f) * 20.0f);
	Lights[2]->SetSpecularColour(D3DXVECTOR3(1.0f, 0.2f, 0.0f) * 15.0f);
	// Update the matrices of lights that will be stationary (dont need to update them in update scene) 
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		if (Lights[i]->IsStationary())
		{
			Lights[i]->UpdateMatrix();
		}
	}

	// Create Light models
	for (unsigned int i = 0; i < 2; i++)
	{
		Lights[i]->SetMaterial(LightMaterial);
		if (!Lights[i]->LoadModel( "Light.x", AdditiveTexTintTechnique ))	return false;
	}

	Lights[2]->SetMaterial(FlamesMaterial);
	if (!Lights[2]->LoadModel("FlameShell.x", AlphaCutoutTechnique)) return false;


	return true;
}

void SwitchMaterialsAndRenderModes()
{
	if (KeyHit(Key_1))	//Set all to plain model colour
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(PlainColourTechnique);
		}
	}
	if (KeyHit(Key_2))	//Set all to basic Material (No lighting)
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(DiffuseTexturedTechnique);
		}
	}
	if (KeyHit(Key_3))	//Set all to wiggle and scroll texture
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(WiggleAndScrollTechnique);
		}
	}
	if (KeyHit(Key_4))	//Set all to have per pixel lighting (no bump maps)
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(PixelLightingTechnique);
		}
	}
	if (KeyHit(Key_5))	//Set all to have normal mapping (no parallax)
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(NormalMapTechnique);
		}
	}
	if (KeyHit(Key_6))	//Set all to have parallax mapping
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			g_Models[i]->SetRenderTechnique(ParallaxMapTechnique);
		}
	}
	if (KeyHit(Key_7))	//Set all to have Parallax mapping (if supported) and/or an outline
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			if (g_Models[i]->UseTangents())
			{
				g_Models[i]->SetRenderTechnique(ParallaxOutlinedTechnique);
			}
			else // does not use tangents
			{
				g_Models[i]->SetRenderTechnique(PixelLitOutlinedTechnique);

			}
		}
	}
	if (KeyHit(Key_8))	//Set all to have black and white noire style shading and an outline (with parallax mapping if supported)
	{
		for (unsigned int i = 0; i < g_Models.size(); i++)
		{
			if (g_Models[i]->UseTangents())
			{
				g_Models[i]->SetRenderTechnique(ParallaxCelShadeTechnique);
			}
			else // does not use tangents
			{
				g_Models[i]->SetRenderTechnique(CelShadingTechnique);
			}
		}
	}
}

// Update the scene - move/rotate each model and the camera, then update their matrices
void UpdateScene(float frameTime)
{
	SwitchMaterialsAndRenderModes();	//Call function to handle real time switching of Materials and rendering techniques for each model

	// Control camera position and update its matrices (view matrix, projection matrix) each frame
	// Don't be deceived into thinking that this is a new method to control models - the same code we used previously is in the camera class
	Camera->Control( frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );
	Camera->UpdateMatrices();
	
	// Control cube position and update its world matrix each frame
	g_Models[0]->Control( frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );
	
	// Update model matrices
	for (unsigned int i = 0; i < g_Models.size(); i++)
	{
		g_Models[i]->UpdateMatrix();
	}
	D3DMATRIX foo = g_Models[6]->GetWorldMatrix();

	// Update wiggle value
	Wiggle += 15.0f * frameTime;

	// Update changing light colours
	
	//Pulsing light (on-off-on)

	PulseTime += 2 * frameTime;
	float cosTime = cosf(PulseTime);

	PulsingLightColour = D3DXVECTOR3(
		Lights[1]->GetDiffuseColour().x * ((cosTime + 1.0f)/2), 
		Lights[1]->GetDiffuseColour().y * ((cosTime + 1.0f)/2),
		Lights[1]->GetDiffuseColour().z * ((cosTime + 1.0f)/2));

	// ColourChangeLight

	static float timer = 0.0f;
	timer += frameTime;
	if (timer > 0.04f)
	{
		D3DXVECTOR3 changingLightColour = Lights[0]->GetDiffuseColour();
		int h, s, l;
		RGBToHSL(changingLightColour.x, changingLightColour.y, changingLightColour.z, h, s, l);
		timer = 0.0f;
		h += 2;
		HSLToRGB(h, s, l, changingLightColour.x, changingLightColour.y, changingLightColour.z);

		Lights[0]->SetDiffuseColour(changingLightColour);
		Lights[0]->SetSpecularColour(changingLightColour);
	}
	
	// Update the orbiting light position - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float Rotate = 0.0f;
	Lights[0]->SetPosition(g_Models[0]->GetPosition() + D3DXVECTOR3(cos(Rotate)*LightOrbitRadius, 0.0f, sin(Rotate)*LightOrbitRadius));
	Rotate -= LightOrbitSpeed * frameTime;
	
	// Update non stationary matrices
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{	//Update matrices of lights that are not stationary
		if (!Lights[i]->IsStationary())
		{
			Lights[i]->UpdateMatrix();
		}
	}
	
}

// Render everything in the scene
void RenderScene()
{
	// Clear the back buffer - before drawing the geometry clear the entire window to a fixed colour
	float ClearColor[4] = { AmbientLight->GetColour().x, AmbientLight->GetColour().y, AmbientLight->GetColour().z, 1.0f }; // Good idea to match background to ambient colour
	g_pd3dDevice->ClearRenderTargetView( RenderTargetView, ClearColor );
	g_pd3dDevice->ClearDepthStencilView( DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 ); // Clear the depth buffer too

	// Common rendering settings

	// Camera data
	ViewMatrixVar->SetMatrix( (float*)&Camera->GetViewMatrix() );
	ProjMatrixVar->SetMatrix( (float*)&Camera->GetProjectionMatrix() );  
	ViewProjMatrixVar->SetMatrix((float*)&Camera->GetViewProjectionMatrix());
		
	CameraPositionVar->SetRawValue(Camera->GetPosition(), 0, sizeof(D3DXVECTOR3));

	// Lighting data
	D3DXVECTOR3 DiffuseLightColours[NO_OF_LIGHTS] = {  /***%***/ Lights[0]->GetDiffuseColour(), PulsingLightColour, Lights[2]->GetDiffuseColour() };
	D3DXVECTOR3 SpecularLightColours[NO_OF_LIGHTS] = { Lights[0]->GetSpecularColour(), PulsingLightColour, Lights[2]->GetSpecularColour() };

	for (int i = 0; i < NO_OF_LIGHTS; i++)
	{
		Lights[i]->LightRender(DiffuseLightColours[i], SpecularLightColours[i]);
	}
	
	AmbientLight->LightRender();

	//Misc
	WiggleVar->SetFloat(Wiggle);
	
	// Render each model - individial model data for shader (Materials etc) is encapsulated in the class
	
	// Render models in vector
	for (unsigned int i = 0; i < g_Models.size(); i++)
	{
		g_Models[i]->Render();
	}

	// Render light models
	
	// Light 0
	Lights[0]->ModelRender(Lights[0]->GetDiffuseColour());
	// Light 1
	Lights[1]->ModelRender(PulsingLightColour);
	// Light 2
	Lights[2]->ModelRender(Lights[2]->GetDiffuseColour());

	// Display the Scene

	// After we've finished drawing to the off-screen back buffer, we "present" it to the front buffer (the screen)
	SwapChain->Present( 0, 0 );
}

//--------------------------------------------------------------------------------------
// Release the memory held by all objects created
//--------------------------------------------------------------------------------------
void ReleaseResources()
{
	// The D3D setup and preparation of the geometry created several objects that use up memory (e.g. textures, vertex/index buffers etc.)
	// Each object that allocates memory (or hardware resources) needs to be "released" when we exit the program
	// There is similar code in every D3D program, but the list of objects that need to be released depends on what was created
	// Test each variable to see if it exists before deletion
	if( g_pd3dDevice )     g_pd3dDevice->ClearState();

	// Deallocate lighting data
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		if (Lights[i])	{ delete Lights[i];		Lights[i]		= NULL;}
	}
	if (AmbientLight )  { delete AmbientLight;	AmbientLight	= NULL;}

	// Deallocate model data
	while (!g_Models.empty())
	{
		if (g_Models.back()) { delete g_Models.back(); }
		g_Models.pop_back();
	}
	
	// Deallocate Material data
	while (!CModel::m_MaterialList.empty())
	{
		if (CModel::m_MaterialList.back()) { delete CModel::m_MaterialList.back(); }
		CModel::m_MaterialList.pop_back();
	}

	// Empty the technique list
	while (!CModel::m_TechniqueList.empty())
	{
		CModel::m_TechniqueList.pop_back();
	}

	// Deallocate the camera
	if (Camera)  {delete Camera; Camera = NULL;}

	// Deallocate other directx variables
	if( Effect )			Effect->Release();
	if( DepthStencilView )	DepthStencilView->Release();
	if( RenderTargetView )	RenderTargetView->Release();
	if( DepthStencil )		DepthStencil->Release();
	if( SwapChain )			SwapChain->Release();
	if( g_pd3dDevice )		g_pd3dDevice->Release();

}