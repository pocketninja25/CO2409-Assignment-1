// **%** is the symbol of code that needs to be redone/undone before submission that might be easily missed - check entire solution - including .fx file

//Need a texture class


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

#include "Defines.h" // General definitions shared by all source files
#include "Model.h"   // Model class - encapsulates working with vertex/index data and world matrix
#include "Light.h"	 // Light class - encapsulates lighting colour/position etc
#include "Camera.h"  // Camera class - encapsulates the camera's view and projection matrix
#include "Input.h"   // Input functions - not DirectX
#include "Texture.h"

//--------------------------------------------------------------------------------------
// Global Scene Variables
//--------------------------------------------------------------------------------------
const unsigned int NO_OF_LIGHTS = 2;

// Models and cameras encapsulated in classes for flexibity and convenience
// The CModel class collects together geometry and world matrix, and provides functions to control the model and render it
// The CCamera class handles the view and projections matrice, and provides functions to control the camera
CModel* Cube;
CModel* Teapot;
CModel* Floor;
CModel* WiggleSphere;
CCamera* Camera;

// Textures
CTexture* StoneTexture;
CTexture* WoodTexture;

// Light data - stored manually as there is no light class
D3DXVECTOR3 PulsingLightColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
D3DXVECTOR3 ChangingLightColour = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

// Display models where the lights are. One of the lights will follow an orbit
CPositionalLight* Lights[NO_OF_LIGHTS];
CAmbientLight* AmbientLight;
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
ID3D10Effect*          Effect = NULL;
ID3D10EffectTechnique* PlainColourTechnique = NULL;
ID3D10EffectTechnique* DiffuseTexturedTechnique = NULL;
ID3D10EffectTechnique* WiggleAndScrollTechnique = NULL;
ID3D10EffectTechnique* PixDiffSpecTechnique = NULL;

// Matrices
ID3D10EffectMatrixVariable* ViewProjMatrixVar = NULL;

// Lighting
ID3D10EffectVectorVariable* CameraPositionVar = NULL;

ID3D10EffectVectorVariable* LightPositionsVar = NULL;
//Need two LightCOloursVar's one for diffuse and one for specular **%**
ID3D10EffectVectorVariable* LightColoursVar = NULL;
ID3D10EffectVectorVariable* AmbientColourVar = NULL;
ID3D10EffectScalarVariable* SpecularPowersVar = NULL;

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


// Release the memory held by all objects created
void ReleaseResources()
{
	// The D3D setup and preparation of the geometry created several objects that use up memory (e.g. textures, vertex/index buffers etc.)
	// Each object that allocates memory (or hardware resources) needs to be "released" when we exit the program
	// There is similar code in every D3D program, but the list of objects that need to be released depends on what was created
	// Test each variable to see if it exists before deletion
	if( g_pd3dDevice )     g_pd3dDevice->ClearState();

	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		delete Lights[i];
	}
	delete AmbientLight;
	delete Teapot;
	delete Floor;
	delete Cube;
	delete WiggleSphere;
	delete Camera;

	delete StoneTexture;
	delete WoodTexture;
	if( Effect )			Effect->Release();
	if( DepthStencilView )	DepthStencilView->Release();
	if( RenderTargetView )	RenderTargetView->Release();
	if( DepthStencil )		DepthStencil->Release();
	if( SwapChain )			SwapChain->Release();
	if( g_pd3dDevice )		g_pd3dDevice->Release();

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

	// Now we can select techniques from the compiled effect file
	PlainColourTechnique = Effect->GetTechniqueByName( "PlainColour" );
	DiffuseTexturedTechnique = Effect->GetTechniqueByName("DiffuseTex");
	WiggleAndScrollTechnique = Effect->GetTechniqueByName("WiggleAndScroll");
	PixDiffSpecTechnique = Effect->GetTechniqueByName("PixDiffSpec");

	// Create special variables to allow us to access global variables in the shaders from C++
	CModel::SetMatrixShaderVariable(Effect->GetVariableByName( "WorldMatrix" )->AsMatrix());
	ViewProjMatrixVar = Effect->GetVariableByName("ViewProjMatrix")->AsMatrix();

	// We access the texture variable in the shader in the same way as we have before for matrices, light data etc.
	// Only difference is that this variable is a "Shader Resource"
	CTexture::SetShaderVariable( Effect->GetVariableByName("DiffuseMap")->AsShaderResource());

	// Lighting
	CameraPositionVar	= Effect->GetVariableByName( "CameraPos" )->AsVector();

	LightColoursVar		= Effect->GetVariableByName( "LightColours" )->AsVector();
	LightPositionsVar	= Effect->GetVariableByName( "LightPositions" )->AsVector();
	AmbientColourVar	= Effect->GetVariableByName( "AmbientColour" )->AsVector();
	SpecularPowersVar	= Effect->GetVariableByName( "SpecularPowers" )->AsScalar();

	// Other shader variables
	CModel::SetColourShaderVariable(Effect->GetVariableByName( "ModelColour"  )->AsVector());
	WiggleVar		= Effect->GetVariableByName( "Wiggle"       )->AsScalar();
		
	return true;
}

//--------------------------------------------------------------------------------------
// Scene Setup / Update / Rendering
//--------------------------------------------------------------------------------------

// Create / load the camera, models and textures for the scene
bool InitScene()
{
	//////////////////
	// Create camera

	Camera = new CCamera();
	Camera->SetPosition(D3DXVECTOR3(-15.0f, 20.0f, -40.0f));
	Camera->SetRotation( D3DXVECTOR3(ToRadians(13.0f), ToRadians(18.0f), 0.0f) ); // ToRadians is a new helper function to convert degrees to radians

	//////////////////
	// Load textures

	StoneTexture = new CTexture();
	WoodTexture = new CTexture();

	if (!StoneTexture->LoadTexture(TEXT("StoneDiffuseSpecular.dds")))
		return false;
	if (!WoodTexture->LoadTexture(TEXT("WoodDiffuseSpecular.dds")))
		return false;

	///////////////////////
	// Load/Create models

	Cube =			new CModel;
	Teapot =		new CModel;
	Floor =			new CModel;
	WiggleSphere =	new CModel;
	AmbientLight =	new CAmbientLight;
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		Lights[i] = new CPositionalLight;
	}

	//**************************************
	// Set Render Techniques and load models

	// The model class can load ".X" files. It encapsulates (i.e. hides away from this code) the file loading/parsing and creation of vertex/index buffers
	// We must pass an example technique used for each model. We can then only render models with techniques that uses matching vertex input data
	if (!Cube->  Load(			"Cube.x",	PixDiffSpecTechnique /*DiffuseTexturedTechnique **%** */ )) return false;
	if (!Floor-> Load(			"Floor.x",	PixDiffSpecTechnique /*DiffuseTexturedTechnique **%** */ )) return false;
	if (!Teapot->Load(			"Teapot.x", PixDiffSpecTechnique))		return false;
	if (!WiggleSphere->Load(	"Sphere.x", WiggleAndScrollTechnique))	return false;
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		if (!Lights[i]->LoadModel(	"Sphere.x", PlainColourTechnique ))	return false;
	}

	// Set Initial values of models/lights
	Cube->SetPosition(D3DXVECTOR3(0.0f, 10.0f, 0.0f));
	Cube->SetTexture(StoneTexture);

	Teapot->SetPosition(D3DXVECTOR3(10.0f, 20.0f, 50.0f));
	Teapot->SetTexture(StoneTexture);

	WiggleSphere->SetPosition(D3DXVECTOR3(-40.0f, 15.0f, 20.0f));
	WiggleSphere->SetScale(0.7f);
	WiggleSphere->SetTexture(StoneTexture);

	Floor->SetTexture(WoodTexture);

	AmbientLight->SetColour(D3DXVECTOR3(0.2f, 0.2f, 0.2f));

	Lights[0]->SetPosition( D3DXVECTOR3(30.0f, 10.0f, 0.0f) );
	Lights[0]->SetScale( 0.1f ); // Nice if size of light reflects its brightness
	Lights[0]->SetIsStationary(true);
	Lights[0]->SetDiffuseColour(D3DXVECTOR3(1.0f, 0.0f, 0.7f));
	Lights[0]->SetSpecularColour(D3DXVECTOR3(1.0f, 0.0f, 0.7f));
	Lights[0]->SetSpecularPower(16.0f);

	//Lights[1]->SetPosition( D3DXVECTOR3(-20.0f, 30.0f, 50.0f) );			//Original position - commented out for debug purposes - need to reset **%**
	Lights[1]->SetPosition(D3DXVECTOR3(-100.0f, 10.0f, 100.0f));
	Lights[1]->SetScale( 0.2f );
	Lights[0]->SetIsStationary(false);
	Lights[1]->SetDiffuseColour(D3DXVECTOR3(1.0f, 0.8f, 0.2f));
	Lights[1]->SetSpecularColour(D3DXVECTOR3(1.0f, 0.8f, 0.2f));
	Lights[1]->SetSpecularPower(16.0f);

	//Update the matrices of lights that will be stationary (dont need to update them in update scene)
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{
		if (Lights[i]->IsStationary())
		{
			Lights[i]->UpdateMatrix();
		}
	}


	return true;
}


// Update the scene - move/rotate each model and the camera, then update their matrices
void UpdateScene( float frameTime )
{
	// Control camera position and update its matrices (view matrix, projection matrix) each frame
	// Don't be deceived into thinking that this is a new method to control models - the same code we used previously is in the camera class
	Camera->Control( frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );
	Camera->UpdateMatrices();
	
	// Control cube position and update its world matrix each frame
	Cube->Control( frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );
	Cube->UpdateMatrix();

	// Teapot
	Teapot->Control(frameTime, Key_Numpad8, Key_Numpad2, Key_Numpad4, Key_Numpad6, Key_Numpad9, Key_Numpad7, Key_Minus, Key_Plus);
	Teapot->UpdateMatrix();

	// Wiggle Sphere - doesnt have controls but does need matrix calculation
	Wiggle += 15.0f * frameTime;
	WiggleSphere->UpdateMatrix();

	// Update the orbiting light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float Rotate = 0.0f;
	Lights[0]->SetPosition(Cube->GetPosition() + D3DXVECTOR3(cos(Rotate)*LightOrbitRadius, 0.0f, sin(Rotate)*LightOrbitRadius));
	Rotate -= LightOrbitSpeed * frameTime;
	
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)
	{	//Update matrices of lights that are not stationary
		if (!Lights[i]->IsStationary())
		{
			Lights[i]->UpdateMatrix();
		}
	}
	// Second light doesn't move, but do need to make sure its matrix has been calculated - could do this in InitScene instead

	PulseTime += 2 * frameTime;
	float cosTime = cosf(PulseTime);
	float sinTime = sinf(PulseTime);
	// **%** need to find a way of doing this by directly influencing thevalues of the light
	PulsingLightColour = D3DXVECTOR3(Lights[0]->GetDiffuseColour().x * (cosTime + 1), Lights[0]->GetDiffuseColour().y * (cosTime + 1), Lights[0]->GetDiffuseColour().z * (cosTime + 1));
	
	ChangingLightColour = D3DXVECTOR3(Lights[1]->GetDiffuseColour().x * (cosTime + 1), Lights[1]->GetDiffuseColour().y * (sinTime + 1), Lights[1]->GetDiffuseColour().z * (cosTime + 2));
}

// Render everything in the scene
void RenderScene()
{
	// Clear the back buffer - before drawing the geometry clear the entire window to a fixed colour
	float ClearColor[4] = { 0.2f, 0.2f, 0.3f, 1.0f }; // Good idea to match background to ambient colour
	g_pd3dDevice->ClearRenderTargetView( RenderTargetView, ClearColor );
	g_pd3dDevice->ClearDepthStencilView( DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 ); // Clear the depth buffer too


	//---------------------------
	// Common rendering settings

	// Common features for all models, set these once only

	// Pass the camera's matrices to the vertex shader
	/*ViewMatrixVar->SetMatrix( (float*)&Camera->GetViewMatrix() );
	ProjMatrixVar->SetMatrix( (float*)&Camera->GetProjectionMatrix() ); - unneccessary - kept in incase later become neccessary again*/ 
	ViewProjMatrixVar->SetMatrix((float*)&Camera->GetViewProjectionMatrix());

	//Misc
	WiggleVar->SetFloat(Wiggle);

	//Lighting
	D3DXVECTOR3 LightPositions[NO_OF_LIGHTS]	= { Lights[0]->GetPosition(),										Lights[1]->GetPosition() };
	D3DXVECTOR3 LightColours[NO_OF_LIGHTS]		= { /*Lights[0]->GetDiffuseColour()*/ PulsingLightColour /***%***/,	ChangingLightColour/*Lights[1]->GetDiffuseColour()*/ };
	float SpecularPowers[NO_OF_LIGHTS]			= { Lights[0]->GetSpecularPower(),									Lights[1]->GetSpecularPower() };

	CameraPositionVar->SetRawValue(Camera->GetPosition(), 0, sizeof(D3DXVECTOR3));
	
	LightColoursVar->SetRawValue(LightColours, 0, sizeof(D3DXVECTOR3) * NO_OF_LIGHTS  );
	AmbientColourVar->SetRawValue(AmbientLight->GetColour(), 0, sizeof(D3DXVECTOR3));
	SpecularPowersVar->SetFloatArray(SpecularPowers, 0, NO_OF_LIGHTS);
	
	LightPositionsVar->SetRawValue(LightPositions, 0, sizeof(D3DXVECTOR3) * NO_OF_LIGHTS);	//Doesnt seem to send position of second light properly


	//---------------------------
	// Render each model
	
	// Constant colours used for models in initial shaders
	D3DXVECTOR3 Black( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 Blue( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 Yellow(1.0f, 1.0f, 0.0f);

	// Render cube
	//CModel::m_MatrixVar->SetMatrix((float*)Cube->GetWorldMatrix());	// Send the cube's world matrix to the shader
	//CTexture::m_MapVar->SetResource(StoneTexture->GetTexture());					// Send the cube's diffuse/specular map to the shader
	//CModel::m_ColourVar->SetRawValue( Blue, 0, sizeof(D3DXVECTOR3) );	// Set a single colour to render the model
	Cube->Render(Blue);							// Pass rendering technique to the model class

	// Floor
	//CModel::m_MatrixVar->SetMatrix((float*)Floor->GetWorldMatrix());		//Matrix
	//CTexture::m_MapVar->SetResource(WoodTexture->GetTexture());						//Map
	//CModel::m_ColourVar->SetRawValue(Black, 0, sizeof(D3DXVECTOR3));		//Colour
	Floor->Render(Black);								//Render

	// Wiggling Sphere
	//CModel::m_MatrixVar->SetMatrix((float*)WiggleSphere->GetWorldMatrix());	//Matrix
	//CTexture::m_MapVar->SetResource(StoneTexture->GetTexture());						//Map
	//CModel::m_ColourVar->SetRawValue(Yellow, 0, sizeof(D3DXVECTOR3));		//Colour
	WiggleSphere->Render(Yellow);						//Render
	
	// Teapot
	//CModel::m_MatrixVar->SetMatrix((float*)Teapot->GetWorldMatrix());		//Matrix
	//CTexture::m_MapVar->SetResource(StoneTexture->GetTexture());						//Map
	Teapot->Render();								//Render

	// Light 0
	//CModel::m_MatrixVar->SetMatrix((float*)Lights[0]->GetWorldMatrix());			//Matrix
	//CModel::m_ColourVar->SetRawValue(PulsingLightColour, 0, sizeof(D3DXVECTOR3));	//Colour
	Lights[0]->Render(PulsingLightColour);									//Render
	
	// Light 1
	//CModel::m_MatrixVar->SetMatrix((float*)Lights[1]->GetWorldMatrix());			//Matrix
	//CModel::m_ColourVar->SetRawValue(ChangingLightColour, 0, sizeof(D3DXVECTOR3));				//Colour
	Lights[1]->Render(ChangingLightColour);									//Render

	//---------------------------
	// Display the Scene

	// After we've finished drawing to the off-screen back buffer, we "present" it to the front buffer (the screen)
	SwapChain->Present( 0, 0 );
}
