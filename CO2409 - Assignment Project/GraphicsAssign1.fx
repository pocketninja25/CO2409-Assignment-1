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

//Normal Mapping vertex shader output
struct VS_NORMALMAP_INPUT
{
	float3 Pos     : POSITION;
	float3 Normal  : NORMAL;
	float2 UV      : TEXCOORD0;
	float3 Tangent : TANGENT;
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
	float3 diffuseColour;
	float3 specularColour;
	float3 position;
};

struct SPOT_LIGHT_DATA
{
	float3 diffuseColour;
	float3 specularColour;
	float3 position;
	float3 facingVector;
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 viewProjMatrix;
	float cosHalfAngle;
	Texture2D shadowMap;
};

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// All these variables are created & manipulated in the C++ code and passed into the shader here

// The matrices (4x4 matrix of floats) for transforming from 3D model to 2D projection (used in vertex shader)
float4x4 WorldMatrix;
float4x4 ViewMatrix;
float4x4 ProjMatrix;
float4x4 ViewProjMatrix;

// Misc
float Wiggle;

//--------------------------
// Model Material Data

// A single colour for an entire model - used for light models and the intial basic shader
float3		ModelColour;
// Diffuse texture map (the main texture colour) - may contain specular map in alpha channel
Texture2D	DiffuseMap;
float		SpecularPower;
// Normal map - may contain parallax map in alpha channel
Texture2D	NormalMap;
float		ParallaxDepth;
//Gradient for clamping Cel shading colours
Texture2D CelGradient;
float OutlineThickness;

//-------------------------
// Camera Data

float3 CameraPos;

//-------------------------
// Lighting Data
static const unsigned int NO_OF_LIGHTS = 3;	

float3 Light1DiffuseCol;
float3 Light2DiffuseCol;
float3 Light3DiffuseCol;
float3 Light1SpecularCol;
float3 Light2SpecularCol;
float3 Light3SpecularCol;
float3 Light1Position;
float3 Light2Position;
float3 Light3Position;

static const unsigned int NO_OF_SPOT_LIGHTS = 1;

float3		SpotLight1DiffuseCol;
float3		SpotLight1SpecularCol;
float3		SpotLight1Position;
float3		SpotLight1Facing;
float4x4	SpotLight1ViewMatrix;
float4x4	SpotLight1ProjMatrix;
float4x4	SpotLight1ViewProjMatrix;
float		SpotLight1CosHalfAngle;
Texture2D	SpotLight1ShadowMap;


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

SamplerState PointSampleClamp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
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
	worldPos.x += cos(modelPos.z + Wiggle) * 0.15f;
	worldPos.y += cos(modelPos.x + Wiggle) * 0.15f;
	worldPos.z += cos(modelPos.y + Wiggle) * 0.15f;
	//*****************************

	vOut.ProjPos = mul(worldPos, ViewProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	return vOut;
}

VS_LIGHTING_OUTPUT PixelLightingTransform(VS_BASIC_INPUT vIn)
{
	VS_LIGHTING_OUTPUT vOut;

	float4 modelPos = float4(vIn.Pos, 1.0f); 
	float4 worldPos = mul(modelPos, WorldMatrix);
	vOut.WorldPos = worldPos.xyz;

	vOut.ProjPos = mul(worldPos, ViewProjMatrix);

	float4 modelNormal = float4(vIn.Normal, 0.0f);
	vOut.WorldNormal = normalize(mul(modelNormal, WorldMatrix)).xyz;

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

VS_BASIC_OUTPUT ExpandOutline(VS_BASIC_INPUT vIn)
{
	VS_BASIC_OUTPUT vOut;

	// Transform model-space vertex position to world-space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);

	// Next the usual transform from world space to camera space - but we don't go any further here - this will be used to help expand the outline
	// The result "viewPos" is the xyz position of the vertex as seen from the camera. The z component is the distance from the camera - that's useful...
	float4 viewPos = mul(worldPos, ViewMatrix);

	// Transform model normal to world space, using the normal to expand the geometry, not for lighting
	float4 modelNormal = float4(vIn.Normal, 0.0f); // Set 4th element to 0.0 this time as normals are vectors
	float4 worldNormal = normalize(mul(modelNormal, WorldMatrix)); // Normalise in case of world matrix scaling

	// Expand the vertex outwards
	// World position is modified (use a general Thickness, modify it by the square root of the distance to the camera then scale the normal by that value 
	// this makes the new world position of the dark version bigger than the 'main version' of the model
	worldPos += OutlineThickness * sqrt(viewPos.z) * worldNormal;

	// Transform new expanded world-space vertex position to viewport-space and output
	viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	return vOut;
}

//-------------------------------------------------------------------------------------
// Pixel Shader Component Functions
//-------------------------------------------------------------------------------------

void AssembleLightData(out LIGHT_DATA Lights[NO_OF_LIGHTS])
{
	//Initialise Lighting Array
	Lights[0].diffuseColour =	Light1DiffuseCol;
	Lights[0].specularColour =	Light1SpecularCol;
	Lights[0].position =		Light1Position;
	Lights[1].diffuseColour =	Light2DiffuseCol;
	Lights[1].specularColour =	Light2SpecularCol;
	Lights[1].position =		Light2Position;
	Lights[2].diffuseColour =	Light3DiffuseCol;
	Lights[2].specularColour =	Light3SpecularCol;
	Lights[2].position =		Light3Position;

}

void AssembleSpotLightData(out SPOT_LIGHT_DATA SpotLights[NO_OF_SPOT_LIGHTS])
{
	SpotLights[0].diffuseColour =	SpotLight1DiffuseCol;
	SpotLights[0].specularColour =	SpotLight1SpecularCol;
	SpotLights[0].position =		SpotLight1Position;
	SpotLights[0].facingVector =	SpotLight1Facing;
	SpotLights[0].viewMatrix =		SpotLight1ViewMatrix;
	SpotLights[0].projMatrix =		SpotLight1ProjMatrix;
	SpotLights[0].viewProjMatrix =	SpotLight1ViewProjMatrix;
	SpotLights[0].cosHalfAngle =	SpotLight1CosHalfAngle;
	SpotLights[0].shadowMap =		SpotLight1ShadowMap;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------

// A pixel shader that just outputs a single fixed colour
//
float4 OneColour(VS_BASIC_OUTPUT vOut) : SV_Target
{
	return float4( ModelColour, 1.0f ); // Set alpha channel to 1.0 (opaque)
}

float4 CelShadeOneColour(VS_BASIC_OUTPUT vOut) : SV_Target
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f); // Set alpha channel to 1.0 (opaque)
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

float4 PixelLighting(VS_LIGHTING_OUTPUT vOut) : SV_Target
{
	LIGHT_DATA Lights[NO_OF_LIGHTS];
	AssembleLightData(Lights);

	//*********************************************************************************************
	// Calculate direction of light and camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)
	
	float3 LightDirs[NO_OF_LIGHTS];
	float3 LightDist[NO_OF_LIGHTS];

	float3 diffuseLight = AmbientColour;	//Begin with just ambient light
	
	float3 halfwayNormal = 0.0f;
	float3 specularLight = 0.0f;
	

	//Perform Lighting Equations for the main lights
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)	//Perform calculations on lights, one light at a time
	{
		LightDirs[i] = Lights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		LightDist[i] = length(LightDirs[i]);
		LightDirs[i] = normalize(LightDirs[i]); //Can now normalise

		//Calculate Diffuse Light
		diffuseLight += Lights[i].diffuseColour * saturate(dot(vOut.WorldNormal.xyz, LightDirs[i])) / LightDist[i];	//Add each light's diffuse value one at a time
		//Calculate specular light
		halfwayNormal = normalize(LightDirs[i] + CameraDir);	//Calculate halfway normal for this light
		specularLight += Lights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower) / LightDist[i];	//Add Specular light for this light onto the current total 

	}
	
	//Perform Lighting Equations for the spot lights

	SPOT_LIGHT_DATA SpotLights[NO_OF_SPOT_LIGHTS];
	AssembleSpotLightData(SpotLights);

	float3 SpotLightDirs[NO_OF_SPOT_LIGHTS];
	float3 SpotLightDist[NO_OF_SPOT_LIGHTS];

	for (unsigned int i = 0; i < NO_OF_SPOT_LIGHTS; i++)
	{
		SpotLightDirs[i] = SpotLights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		SpotLightDist[i] = length(SpotLightDirs[i]);
		SpotLightDirs[i] = normalize(SpotLightDirs[i]); //Can now normalise		
		SpotLights[i].facingVector = normalize(SpotLights[i].facingVector);

		if (SpotLights[i].cosHalfAngle < dot(SpotLights[i].facingVector, -SpotLightDirs[i]))
		{
			//Calculate Diffuse Light
			diffuseLight += SpotLights[i].diffuseColour * saturate(dot(vOut.WorldNormal.xyz, SpotLightDirs[i])) / SpotLightDist[i];	//Add each light's diffuse value one at a time
			//Calculate specular light
			halfwayNormal = normalize(SpotLightDirs[i] + CameraDir);	//Calculate halfway normal for this ligh
			specularLight += SpotLights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower) / SpotLightDist[i];	//Add Specular light for this light onto the current total 
		}
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
	
	// Create a VS_LIGHTING_OUTPUT to send to PixelLighting to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = vOut.UV;
	return PixelLighting(diffSpecOut);	//Return the value given by the PixelLighting pixel shader
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

	// Create a VS_LIGHTING_OUTPUT to send to PixelLighting to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = offsetTexCoord;
	return PixelLighting(diffSpecOut);	//Return the value given by the PixelLighting pixel shader
}

float4 NoireShade(VS_LIGHTING_OUTPUT vOut) : SV_Target  // The ": SV_Target" bit just indicates that the returned float4 colour goes to the render target (i.e. it's a colour to render)
{
	// Can't guarantee the normals are length 1, interpolation from vertex shader to pixel shader will rescale normals
	float3 worldNormal = normalize(vOut.WorldNormal);

	//Initialise Lighting Array
	LIGHT_DATA Lights[NO_OF_LIGHTS];
	AssembleLightData(Lights);

	//*********************************************************************************************
	// Calculate direction of light and camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)

	float3 LightDirs[NO_OF_LIGHTS];
	float3 LightDist[NO_OF_LIGHTS];

	float3 DiffuseLights[NO_OF_LIGHTS];

	float3 combinedDiffuse = AmbientColour;	//Begin with just ambient light

	float3 halfwayNormal = 0.0f;
	float3 specularLight = 0.0f;

	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)	//Perform calculations on lights, one light at a time
	{
		LightDirs[i] = Lights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		LightDist[i] = length(LightDirs[i]);
		LightDirs[i] = normalize(LightDirs[i]); //Can now normalise
		
		//Calculate Diffuse Light
		DiffuseLights[i] = Lights[i].diffuseColour *																//Take the light colour and multiply it by
			CelGradient.Sample(PointSampleClamp, saturate(dot(vOut.WorldNormal.xyz, LightDirs[i]))).r		//the light level for this point (clamped based on the Celgradient texture)
			/ LightDist[i];																					//Divided by the distance of the light (for attenuation)
		
		//Calculate specular light
		halfwayNormal = normalize(LightDirs[i] + CameraDir);	//Calculate halfway normal for this light
				
		//specularLight += Lights[i].specularColour * CelGradient.Sample(PointSampleClamp, pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower)).r / LightDist[i];	//Add Specular light for this light onto the current total 

		specularLight += 30.0f * DiffuseLights[i] * 
			CelGradient.Sample(PointSampleClamp, Lights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower)).r / LightDist[i];	//Add Specular light for this light onto the current total 

	}

	//Perform Lighting Equations for the spot lights

	SPOT_LIGHT_DATA SpotLights[NO_OF_SPOT_LIGHTS];
	AssembleSpotLightData(SpotLights);

	float3 SpotLightDirs[NO_OF_SPOT_LIGHTS];
	float3 SpotLightDist[NO_OF_SPOT_LIGHTS];
	
	float3 SpotDiffuseLights[NO_OF_SPOT_LIGHTS];

	for (unsigned int i = 0; i < NO_OF_SPOT_LIGHTS; i++)
	{
		SpotLightDirs[i] = SpotLights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		SpotLightDist[i] = length(SpotLightDirs[i]);
		SpotLightDirs[i] = normalize(SpotLightDirs[i]); //Can now normalise
		SpotLights[i].facingVector = normalize(SpotLights[i].facingVector);

		if (SpotLights[i].cosHalfAngle < dot(SpotLights[i].facingVector, -SpotLightDirs[i]))
		{
			//Calculate Diffuse Light
			SpotDiffuseLights[i] = SpotLights[i].diffuseColour * 
				CelGradient.Sample(PointSampleClamp, saturate(dot(vOut.WorldNormal.xyz, SpotLightDirs[i]))).r / SpotLightDist[i];	//Add each light's diffuse value one at a time
			//Calculate specular light
			halfwayNormal = normalize(SpotLightDirs[i] + CameraDir);	//Calculate halfway normal for this ligh
			specularLight += 30.0f * DiffuseLights[i] *
				CelGradient.Sample(PointSampleClamp, SpotLights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower)).r / SpotLightDist[i];	//Add Specular light for this light onto the current total 
		}
	}

	for (int i = 0; i < NO_OF_LIGHTS; i++)
	{
		combinedDiffuse += DiffuseLights[i];
	}

	//combinedDiffuse = 0.0f;	//Set Diffuse to 0 (DEBUGGING ONLY)
	//specularLight = 0.0f; //Set Specular to 0 (DEBUGGING ONLY)

	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV) * CelGradient.Sample(PointSampleClamp, DiffuseMap.Sample(TrilinearWrap, vOut.UV)).r;

	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * combinedDiffuse + SpecularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 CelShade(VS_LIGHTING_OUTPUT vOut) : SV_Target  // The ": SV_Target" bit just indicates that the returned float4 colour goes to the render target (i.e. it's a colour to render)
{
	// Can't guarantee the normals are length 1, interpolation from vertex shader to pixel shader will rescale normals
	float3 worldNormal = normalize(vOut.WorldNormal);

	//Initialise Lighting Array
	LIGHT_DATA Lights[NO_OF_LIGHTS];
	AssembleLightData(Lights);

	//*********************************************************************************************
	// Calculate direction of light and camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)

	float3 LightDirs[NO_OF_LIGHTS];
	float3 LightDist[NO_OF_LIGHTS];

	float3 DiffuseLight;

	float3 combinedDiffuse = AmbientColour;	//Begin with just ambient light

	float3 halfwayNormal = 0.0f;
	float3 specularLight = 0.0f;

	float DiffuseLevel;
	float CelDiffuseLevel;
	float SpecularLevel;
	float CelSpecularLevel;

	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)	//Perform calculations on lights, one light at a time
	{

		LightDirs[i] = Lights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		LightDist[i] = length(LightDirs[i]);
		LightDirs[i] = normalize(LightDirs[i]); //Can now normalise

		DiffuseLevel = saturate(dot(worldNormal.xyz, LightDirs[i]));
		CelDiffuseLevel = CelGradient.Sample(PointSampleClamp, DiffuseLevel).r;

		//Calculate Diffuse Light
		DiffuseLight = Lights[i].diffuseColour * CelDiffuseLevel / LightDist[i];					//Take the light colour and multiply it by
																									//the light level for this point (clamped based on the Celgradient texture)
																									//Divided by the distance of the light (for attenuation)

		//Calculate specular light
		halfwayNormal = normalize(LightDirs[i] + CameraDir);	//Calculate halfway normal for this light
		SpecularLevel = pow(saturate(dot(worldNormal.xyz, halfwayNormal)), SpecularPower);
		CelSpecularLevel = CelGradient.Sample(PointSampleClamp, SpecularLevel).r;
		specularLight += DiffuseLight * CelSpecularLevel;
		
		combinedDiffuse += DiffuseLight;
	}


	//Perform Lighting Equations for the spot lights

	SPOT_LIGHT_DATA SpotLights[NO_OF_SPOT_LIGHTS];
	AssembleSpotLightData(SpotLights);

	float3 SpotLightDirs[NO_OF_SPOT_LIGHTS];
	float3 SpotLightDist[NO_OF_SPOT_LIGHTS];

	float3 SpotDiffuseLights[NO_OF_SPOT_LIGHTS];

	for (unsigned int i = 0; i < NO_OF_SPOT_LIGHTS; i++)
	{
		SpotLightDirs[i] = SpotLights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		SpotLightDist[i] = length(SpotLightDirs[i]);
		SpotLightDirs[i] = normalize(SpotLightDirs[i]); //Can now normalise
		SpotLights[i].facingVector = normalize(SpotLights[i].facingVector);

		if (SpotLights[i].cosHalfAngle < dot(SpotLights[i].facingVector, -SpotLightDirs[i]))
		{
			DiffuseLevel = saturate(dot(worldNormal.xyz, SpotLightDirs[i]));
			CelDiffuseLevel = CelGradient.Sample(PointSampleClamp, DiffuseLevel).r;

			//Calculate Diffuse Light
			DiffuseLight = SpotLights[i].diffuseColour * CelDiffuseLevel / SpotLightDist[i];					//Take the light colour and multiply it by
			//the light level for this point (clamped based on the Celgradient texture)
			//Divided by the distance of the light (for attenuation)

			//Calculate specular light
			halfwayNormal = normalize(SpotLightDirs[i] + CameraDir);	//Calculate halfway normal for this light
			SpecularLevel = pow(saturate(dot(worldNormal.xyz, halfwayNormal)), SpecularPower);
			CelSpecularLevel = CelGradient.Sample(PointSampleClamp, SpecularLevel).r;
			specularLight += DiffuseLight * CelSpecularLevel;

			combinedDiffuse += DiffuseLight;
		}
	}

	//combinedDiffuse = 0.0f;	//Set Diffuse to 0 (DEBUGGING ONLY)
	//specularLight = 0.0f;		//Set Specular to 0 (DEBUGGING ONLY)

	////////////////////
	// Sample texture

	// Extract diffuse material colour for this pixel from a texture (using float3, so we get RGB - i.e. ignore any alpha in the texture)
	float4 DiffuseMaterial = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	// Assume specular material colour is white (i.e. highlights are a full, untinted reflection of light)
	float3 SpecularMaterial = DiffuseMaterial.a;


	////////////////////
	// Combine colours 

	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseMaterial * combinedDiffuse + SpecularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}

float4 ParallaxCelShade(VS_NORMALMAP_OUTPUT vOut) : SV_Target
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

	float3x3 invWorldMatrix = transpose(WorldMatrix);						// Flip matrix over its diagonal - need this to move camera vector from world space 'backwards' into the model space of this model
	float3 cameraModelDir = normalize(mul(CameraDir, invWorldMatrix));	// Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose(invTangentMatrix);
	float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix);

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

	// Create a VS_LIGHTING_OUTPUT to send to PixelLighting to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = offsetTexCoord;
	return CelShade(diffSpecOut);	//Return the value given by the PixelLighting pixel shader
}

float4 TintDiffuseMap(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// Sample the material colour for this pixel from the texture
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);

	// Tint the pixel colour by the colour of the model
	diffuseMapColour.rgb *= ModelColour;

	return diffuseMapColour;
}

float4 CutoutTextured(VS_LIGHTING_OUTPUT vOut) : SV_Target
{
	float4 colour = DiffuseMap.Sample(Anisotropic16, vOut.UV);	//Return the texture colour of this pixel

	if (colour.a < 0.5f)
		discard;

	return colour * 0.5f;
}

float4 NoireParallaxMapLighting(VS_NORMALMAP_OUTPUT vOut) : SV_Target
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

	float3x3 invWorldMatrix = transpose(WorldMatrix);						// Flip matrix over its diagonal - need this to move camera vector from world space 'backwards' into the model space of this model
	float3 cameraModelDir = normalize(mul(CameraDir, invWorldMatrix));	// Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose(invTangentMatrix);
	float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix);

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

	// Create a VS_LIGHTING_OUTPUT to send to PixelLighting to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = offsetTexCoord;
	return NoireShade(diffSpecOut);	//Return the value given by the PixelLighting pixel shader
}

// Shader used when rendering the shadow map depths. In fact a pixel shader isn't needed, we are
// only writing to the depth buffer. However, needed to display what's in a shadow map
float4 PixelDepth(VS_BASIC_OUTPUT vOut) : SV_Target
{
	// Output the value that would go in the depth puffer to the pixel colour (greyscale)
	return vOut.ProjPos.z / vOut.ProjPos.w;
}

float4 ShadowMapPix(VS_LIGHTING_OUTPUT vOut) : SV_Target
{
	//Calculate lighting for non-directional lights
	LIGHT_DATA Lights[NO_OF_LIGHTS];
	AssembleLightData(Lights);

	// Can't guarantee the normals are length 1, so re-normalise
	float3 worldNormal = normalize(vOut.WorldNormal);

	//*********************************************************************************************
	// Calculate direction of light and camera
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)

	float3 LightDirs[NO_OF_LIGHTS];
	float3 LightDist[NO_OF_LIGHTS];

	float3 diffuseLight = AmbientColour;	//Begin with just ambient light

	float3 halfwayNormal = 0.0f;
	float3 specularLight = 0.0f;


	//Perform Lighting Equations for the main lights
	for (unsigned int i = 0; i < NO_OF_LIGHTS; i++)	//Perform calculations on lights, one light at a time
	{
		LightDirs[i] = Lights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		LightDist[i] = length(LightDirs[i]);
		LightDirs[i] = normalize(LightDirs[i]); //Can now normalise
	
		//Calculate Diffuse Light
		diffuseLight += Lights[i].diffuseColour * saturate(dot(vOut.WorldNormal.xyz, LightDirs[i])) / LightDist[i];	//Add each light's diffuse value one at a time
		//Calculate specular light
		halfwayNormal = normalize(LightDirs[i] + CameraDir);	//Calculate halfway normal for this light
		specularLight += Lights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower) / LightDist[i];	//Add Specular light for this light onto the current total 
	
	}

	//Calculate lighting for directional lights (This is where the shadows come in)

	// Slight adjustment to calculated depth of pixels so they don't shadow themselves
	const float DepthAdjust = 0.0005f;

	SPOT_LIGHT_DATA SpotLights[NO_OF_SPOT_LIGHTS];
	AssembleSpotLightData(SpotLights);

	float3 SpotLightDirection;
	float3 SpotLightDistance;
	float4 LightViewPos;
	float4 LightProjPos;
	float2 shadowUV;
	float depthFromLight;
	float depthFromShadowMap;

	for (unsigned int i = 0; i < NO_OF_SPOT_LIGHTS; i++)
	{
		// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		SpotLightDirection = SpotLights[i].position - vOut.WorldPos.xyz;	//Dont normalise yet (need to calculate length for attenuated light first)
		SpotLightDistance = length(SpotLightDirection);
		SpotLightDirection = normalize(SpotLightDirection); //Can now normalise
		SpotLights[i].facingVector = normalize(SpotLights[i].facingVector);

		if (SpotLights[i].cosHalfAngle < dot(SpotLights[i].facingVector, -SpotLightDirection))
		{ 
			LightViewPos = mul(float4(vOut.WorldPos, 1.0f), SpotLights[i].viewMatrix);
			LightProjPos = mul(LightViewPos, SpotLights[i].projMatrix);

			// Convert 2D pixel position as viewed from light into texture coordinates for shadow map
			// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
			shadowUV = 0.5f * LightProjPos.xy / (LightProjPos.w + float2(0.5f, 0.5f));
			shadowUV.y = 1.0f - shadowUV.y;
			
			// Get depth of this pixel if it were visible from the light
			depthFromLight = LightProjPos.z / LightProjPos.w - DepthAdjust;	// Adjustment so polygons don't shadow themselves
			depthFromShadowMap = SpotLights[i].shadowMap.Sample(PointSampleClamp, shadowUV).r;
			// Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less then something is nearer
			// to the light than this pixel - so the pixel gets no effect from this light

			if (depthFromLight < depthFromShadowMap)
			{
				//Calculate Diffuse Light
				diffuseLight += SpotLights[i].diffuseColour * saturate(dot(vOut.WorldNormal.xyz, SpotLightDirection)) / SpotLightDistance;	//Add each light's diffuse value one at a time
				//Calculate specular light
				halfwayNormal = normalize(SpotLightDirection + CameraDir);	//Calculate halfway normal for this light
				specularLight += SpotLights[i].specularColour * pow(saturate(dot(vOut.WorldNormal.xyz, halfwayNormal)), SpecularPower) / SpotLightDistance;	//Add Specular light for this light onto the current total 
			}
		}
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

	//Debug tool - view shadow map on uv's
	//combinedColour.rgb = SpotLights[0].shadowMap.Sample(TrilinearWrap, vOut.UV).r;
	return combinedColour; 
}

float4 ShadowMapParallax(VS_NORMALMAP_OUTPUT vOut) : SV_Target
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

	float3x3 invWorldMatrix = transpose(WorldMatrix);						// Flip matrix over its diagonal - need this to move camera vector from world space 'backwards' into the model space of this model
	float3 cameraModelDir = normalize(mul(CameraDir, invWorldMatrix));	// Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose(invTangentMatrix);
	float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix);

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

	// Create a VS_LIGHTING_OUTPUT to send to PixelLighting to perform the relevant lighting calculations
	VS_LIGHTING_OUTPUT diffSpecOut;
	diffSpecOut.ProjPos = vOut.ProjPos;
	diffSpecOut.WorldPos = vOut.WorldPos;
	diffSpecOut.WorldNormal = worldNormal;
	diffSpecOut.UV = offsetTexCoord;

	return ShadowMapPix(diffSpecOut);
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
BlendState MultiplicativeBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = DEST_COLOR;
	DestBlend = ZERO;
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
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PixelLighting()));

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

technique10 NoireShading
{
	pass P0		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1		//Draw the model itself - with a colour range clamp
	{
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, NoireShade()));

		// Return to standard culling (draw only the outside of the model)
		SetRasterizerState(CullBack);
	}
}

technique10 ParallaxNoireShaded
{
	pass P0		//Draw the model itself
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, NoireParallaxMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 CelShading
{
	pass P0		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1		//Draw the model itself - with a colour range clamp
	{
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShade()));

		// Return to standard culling (draw only the outside of the model)
		SetRasterizerState(CullBack);
	}
};

technique10 ParallaxCelShading
{
	pass P0		//Draw the model itself - with a colour range clamp
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ParallaxCelShade()));

		// Return to standard culling (draw only the outside of the model)
		SetRasterizerState(CullBack);
	}
	pass P1		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
};

technique10 ParallaxOutlined
{
	pass P0		//Draw the model itself
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ParallaxMapLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 PixelLitOutlined
{
	pass P0		//Draw the model itself
	{
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PixelLighting()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
	pass P1		//Draw the darkened outline of the 'Cel shading'
	{
		SetVertexShader(CompileShader(vs_4_0, ExpandOutline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CelShadeOneColour()));

		// Draw the inside of the model
		SetRasterizerState(CullFront);

		// Switch off other blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 AdditiveTexTint
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TintDiffuseMap()));

		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOff, 0);
	}
}

technique10 AlphaCutout
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, CutoutTextured()));

		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOff, 0);
	}
}

technique10 DepthOnly
{
	// Rendering a shadow map. Only outputs the depth of each pixel
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, BasicTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PixelDepth()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 ShadowMappingPixelLit
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, PixelLightingTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPix()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 ShadowMappingParallaxLit
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, NormalMapTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapParallax()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}