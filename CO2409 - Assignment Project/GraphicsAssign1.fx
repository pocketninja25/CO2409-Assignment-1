//--------------------------------------------------------------------------------------
// File: GraphicsAssign1.fx
//
//	Shaders Graphics Assignment
//	Add further models using different shader techniques
//	See assignment specification for details
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// Standard input geometry data, more complex techniques (e.g. normal mapping) may need more
struct VS_BASIC_INPUT
{
	float3 Pos    : POSITION;
	float3 Normal : NORMAL;
	float2 UV     : TEXCOORD0;
};

// Data output from vertex shader to pixel shader for simple techniques. Again different techniques have different requirements
struct VS_BASIC_OUTPUT
{
	float4 ProjPos : SV_POSITION;
	float2 UV      : TEXCOORD0;
};

//Alternate Vertex Shader Output - needed values for pixel Lighting
struct VS_LIGHTING_OUTPUT
{
	float4 ProjPos		: SV_POSITION;  // 2D "projected" position for vertex (required output for vertex shader)
	float3 WorldPos		: POSITION;
	float3 WorldNormal	: NORMAL;
	float2 UV			: TEXCOORD0;
};

//Normal Mapping vertex shader output
struct VS_NORMALMAP_INPUT
{
	float3 Pos     : POSITION;
	float3 Normal  : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV      : TEXCOORD0;
};

//Normal Mapping vertex shader output
struct VS_NORMALMAP_OUTPUT
{
	float4 ProjPos      : SV_POSITION;
	float3 WorldPos     : POSITION;
	float3 ModelNormal  : NORMAL;
	float3 ModelTangent : TANGENT;
	float2 UV           : TEXCOORD0;
};

struct LIGHT_DATA
{
	float3 colour;
	float3 position;
	float specularPower;
};

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// All these variables are created & manipulated in the C++ code and passed into the shader here

// The matrices (4x4 matrix of floats) for transforming from 3D model to 2D projection (used in vertex shader)
float4x4 WorldMatrix;
float4x4 ViewProjMatrix;

// Misc
float Wiggle;

// A single colour for an entire model - used for light models and the intial basic shader
float3 ModelColour;

// Diffuse texture map (the main texture colour) - may contain specular map in alpha channel
Texture2D DiffuseMap;
// Normal map - may contain parralax map in alpha channel
Texture2D NormalMap;

float ParallaxDepth;

//Lighting Data
float3 CameraPos;

static const unsigned int NO_OF_LIGHTS = 2;	
//float3 LightColours[NO_OF_LIGHTS];
//float3 LightPositions[NO_OF_LIGHTS];
//float SpecularPowers[NO_OF_LIGHTS];
float3 Light1Colour;
float3 Light2Colour;
float3 Light1Position;
float3 Light2Position;
float  Light1SpecularPower;
float  Light2SpecularPower;

float3 AmbientColour;

//--------------------------------------------------------------------------------------
// Texture Samplers
//--------------------------------------------------------------------------------------

SamplerState Anisotropic16
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
	MaxAnisotropy = 16;
};

SamplerState TrilinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

// Basic vertex shader to transform 3D model vertices to 2D and pass UVs to the pixel shader
//
VS_BASIC_OUTPUT BasicTransform( VS_BASIC_INPUT vIn )
{
	VS_BASIC_OUTPUT vOut;
	
	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul( modelPos, WorldMatrix );
	vOut.ProjPos    = mul( worldPos,  ViewProjMatrix );
	
	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	return vOut;
}

VS_BASIC_OUTPUT WiggleTransform(VS_BASIC_INPUT vIn)
{
	VS_BASIC_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);

	//****Make the vertices wiggle
	float4 normal = float4(vIn.Normal, 0.0f);	//Promote normal to a float4 for calculations
	float4 worldNormal = mul(normal, WorldMatrix);	//Put normal in world space
	worldNormal = normalize(worldNormal);			//Ensure the normal is normalised

	//Make the vertices wiggle
	worldPos.x += cos(modelPos.z + Wiggle) * 0.1f;
	worldPos.y += cos(modelPos.y + Wiggle) * 0.1f;
	worldPos.z += cos(modelPos.x + Wiggle) * 0.1f;
	//*****************************

	vOut.ProjPos = mul(worldPos, ViewProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	return vOut;
}

VS_LIGHTING_OUTPUT PixelDiffSpecTransform(VS_BASIC_INPUT vIn)
{
	VS_LIGHTING_OUTPUT vOut;

	float4 modelPos = float4(vIn.Pos, 1.0f); 
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = (float3)worldPos;

	vOut.ProjPos = mul(worldPos, ViewProjMatrix);

	float4 modelNormal = float4(vIn.Normal, 0.0f);
	float4 worldNormal = mul(modelNormal, WorldMatrix);
	vOut.WorldNormal = (float3)normalize(worldNormal);

	vOut.UV = vIn.UV;
	return vOut;
}

VS_NORMALMAP_OUTPUT NormalMapTransform(VS_NORMALMAP_INPUT vIn)
{
	VS_NORMALMAP_OUTPUT vOut;

	// Transform model position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f);
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	// Transform vertex from world space to view space
	vOut.ProjPos = mul(worldPos, ViewProjMatrix);

	// Send the model's normal and tangent in model space. (Pixel shader transforms them to world space)
	vOut.ModelNormal = vIn.Normal;
	vOut.ModelTangent = vIn.Tangent;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	return vOut;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------


// A pixel shader that just outputs a single fixed colour
//
float4 OneColour( VS_BASIC_OUTPUT vOut ) : SV_Target
{
	return float4( ModelColour, 1.0 ); // Set alpha channel to 1.0 (opaque)
}

float4 DiffuseTextured(VS_BASIC_OUTPUT vOut) : SV_Target
{
	return DiffuseMap.Sample(Anisotropic16, vOut.UV);	//Return the texture colour of this pixel
}

float4 ScrollAndTint(VS_BASIC_OUTPUT vOut) : SV_Target
{
	float4 diffuseMapColour = DiffuseMap.Sample(Anisotropic16, (vOut.UV + Wiggle / 200));	//set the map colour of this pixel to an offset UV based on the wiggle value passed from c++

	float4 combinedColour;
	combinedColour.rgb = diffuseMapColour.rgb * ModelColour;		//Combine the map colour with the model colour using multiplicative blending - this adds a tint to the map colour
	combinedColour.a = 1.0f;										//No alpha channel

	return combinedColour;
}

float4 DiffuseSpecular(VS_LIGHTING_OUTPUT vOut) : SV_Target
{
	//Initialise Lighting Array
	LIGHT_DATA Lights[NO_OF_LIGHTS];
	Lights[0].colour = Light1Colour;
	Lights[0].position = Light1Position;
	Lights[0].specularPower = Light1SpecularPower;
	Lights[1].colour = Light2Colour;
	Lights[1].position = Light2Position;
	Lights[1].specularPower = Light2SpecularPower;
	
	//*********************************************************************************************
	// Calculate direction of light and camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)
	
	float3 LightDirs[NO_OF_LIGHTS];
	float3 AttenuatedLights[NO_OF_LIGHTS];
	float3 LightDist[NO_OF_LIGHTS];

	float3 diffuseLight = AmbientColour;	//Begin with just ambient light
	
	float3 halfwayNormal = 0.0f;
	float3 specularLight = 0.0f;
	
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)	//Perform calculations on lights, one light at a time
	{
		LightDirs[i] = Lights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate attenuated light first)
		LightDist[i] = length(LightDirs[i]);
		//AttenuatedLights[i] = (Lights[i].colour / length(LightDirs[i]));	//Calculate Light attenuation
		LightDirs[i] = normalize(LightDirs[i]); //Can now normalise
		//Calculate Diffuse Light
		diffuseLight += (Lights[i].colour * saturate(dot(vOut.WorldNormal.xyz, LightDirs[i])) / LightDist[i]);	//Add each light's diffuse value one at a time
		//Calculate specular light
		halfwayNormal = normalize(LightDirs[i] + CameraDir);	//Calculate halfway normal for this ligh
		specularLight += (Lights[i].colour / LightDist[i]) * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), Lights[i].specularPower);	//Add Specular light for this light onto the current total 

	}
	

	//diffuseLight = 0.0f;	//Set Diffuse to 0 (DEBUGGING ONLY)
	//specularLight = 0.0f; //Set Specular to 0 (DEBUGGING ONLY)

	//*********************************************************************************************
	// Get colours from texture maps

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float3 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV).rgb;

	// Sample specular material colour from alpha channel of the diffusemap
	float SpecularMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV).a;
	//*********************************************************************************************
	// Combine colours (lighting, textures) for final pixel colour

	float4 combinedColour;
	//combinedColour.rgb = diffuseLight + specularLight;	//Light - without material
	combinedColour.rgb = (DiffuseMaterial * diffuseLight) + (SpecularMaterial * specularLight); //Light - with material
	
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 NormalMapLighting(VS_NORMALMAP_OUTPUT vOut) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	//Normalise interpolated model normal and tangent
	float3 modelNormal = normalize(vOut.ModelNormal);
	float3 modelTangent = normalize(vOut.ModelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space.
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	// Get the texture normal from the normal map and convert from rgb range to xyz range (colour of normal map to the normal itself)
	float3 textureNormal = 2.0f * NormalMap.Sample(TrilinearWrap, vOut.UV) - 1.0f; 	//range 0->1, to range 1->1.

	/*Exaggerate the normal provided by the texture*/
	textureNormal.z /= 5.0f;
	normalize(textureNormal);
	/*---------------------------------------------*/

	// Convert from texture space to model space using invTangentMatrix, then convert to world space using worldmatrix - then normalize (to accomodate world scaling etc..)
	float3 worldNormal = normalize(mul(mul(textureNormal, invTangentMatrix), WorldMatrix));

	// The world normal is now the version from the file rather than the automatically generated one
	//This means we can use the ordinary diffuse/specular lighting equations/shader to perform the lighting calculation
	
	// Create a VS_LIGHTING_OUTPUT to send to DiffuseSpecular to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = vOut.UV;
	return DiffuseSpecular(diffSpecOut);	//Return the value given by the DiffuseSpecular pixel shader
}

float4 ParallaxMapLighting(VS_NORMALMAP_OUTPUT vOut) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	//Normalise interpolated model normal and tangent
	float3 modelNormal = normalize(vOut.ModelNormal);
	float3 modelTangent = normalize(vOut.ModelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space.
	float3 modelBiTangent = cross(modelNormal, modelTangent);
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	//--------------------------------------------
	// Parallax Mapping - Calculate alternate texture coordinate for this pixel
	//--------------------------------------------

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz);

	float3x3 invWorldMatrix = transpose( WorldMatrix );						// Flip matrix over its diagonal - need this to move camera vector from world space 'backwards' into the model space of this model
	float3 cameraModelDir = normalize( mul( CameraDir, invWorldMatrix ) );	// Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose( invTangentMatrix ); 
	float2 textureOffsetDir = mul( cameraModelDir, tangentMatrix );

	// Get the depth info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
	float texDepth = ParallaxDepth * (NormalMap.Sample(TrilinearWrap, vOut.UV).a - 0.5f);

	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
	float2 offsetTexCoord = vOut.UV + texDepth * textureOffsetDir;

	//--------------------------------------------
	// End parallax mapping
	//--------------------------------------------

	// Get the texture normal from the normal map and convert from rgb range to xyz range (colour of normal map to the normal itself)
	float3 textureNormal = 2.0f * NormalMap.Sample(TrilinearWrap, offsetTexCoord) - 1.0f; 	//range 0->1, to range 1->1. // Using new offsetTexCoord (parallax effected texture coordinate) instead of standard UV

	// Calculate Lighting for specular/diffuse as usual

	// Convert from texture space to model space using invTangentMatrix, then convert to world space using worldmatrix - then normalize (to accomodate world scaling etc..)
	float3 worldNormal = normalize(mul(mul(textureNormal, invTangentMatrix), WorldMatrix));

	// The world normal is now the version from the file rather than the automatically generated one
	//This means we can use the ordinary diffuse/specular lighting equations/shader to perform the lighting calculation

	// Create a VS_LIGHTING_OUTPUT to send to DiffuseSpecular to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = offsetTexCoord;
	return DiffuseSpecular(diffSpecOut);	//Return the value given by the DiffuseSpecular pixel shader
}

//-------------------------------------------------------------------------------------
// Rasteriser States
//-------------------------------------------------------------------------------------

RasterizerState CullNone  // Cull none of the polygons, i.e. show both sides
{
	CullMode = None;
};
RasterizerState CullBack  // Cull back side of polygon - normal behaviour, only show front of polygons
{
	CullMode = Back;
};
RasterizerState CullFront	//Cull front side of polygon - allows you to see from an interior view (e.g. skyboxes)
{
	CullMode = Front;
};

//-------------------------------------------------------------------------------------
// Blending States
//-------------------------------------------------------------------------------------

BlendState NoBlending // Switch off blending - pixels will be opaque
{
	BlendEnable[0] = FALSE;
};
BlendState AdditiveBlending // Additive blending is used for lighting effects
{
	BlendEnable[0] = TRUE;
	SrcBlend = ONE;
	DestBlend = ONE;
	BlendOp = ADD;
};

//-------------------------------------------------------------------------------------
// Depth Buffer States
//-------------------------------------------------------------------------------------

DepthStencilState DepthWritesOff // Don't write to the depth buffer - polygons rendered will not obscure other polygons
{
	DepthWriteMask = ZERO;
};
DepthStencilState DepthWritesOn  // Write to the depth buffer - normal behaviour 
{
	DepthWriteMask = ALL;
};

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

// Techniques are used to render models in our scene. They select a combination of vertex, geometry and pixel shader from those provided above. Can also set states.

// Render models unlit in a single colour
technique10 PlainColour
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, BasicTransform() ) );
		SetGeometryShader( NULL );                                   
		SetPixelShader( CompileShader( ps_4_0, OneColour() ) );

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 DiffuseTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, DiffuseTextured()));

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 WiggleAndScroll
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, WiggleTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ScrollAndTint()));

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 PixDiffSpec
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, PixelDiffSpecTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, DiffuseSpecular()));

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 NormalMapping
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, NormalMapLighting()));

		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 ParallaxMapping
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ParallaxMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}