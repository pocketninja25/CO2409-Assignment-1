//--------------------------------------------------------------------------------------
//	Model.h
//
//	The model class collects together a model's geometry (vertex and index data) and
//	also manages it's positioning with a world
//--------------------------------------------------------------------------------------

#ifndef MODEL_H_INCLUDED // Header guard - prevents file being included more than once (would cause errors)
#define MODEL_H_INCLUDED

#include <string>
using namespace std;

#include <d3d10.h>
#include <d3dx10.h>
#include "Input.h"
#include "Material.h"
#include "Technique.h"

#include <vector>

class CModel
{
/////////////////////////////
// Private member variables
private:
	//-----------------
	// Postioning

	// Positions, rotations and scaling for the model
	D3DXVECTOR3   m_Position;
	D3DXVECTOR3   m_Rotation;
	D3DXVECTOR3   m_Scale;

	// World matrix for the model - built from the above
	D3DXMATRIX m_WorldMatrix;

	
	//-----------------
	// Geometry data

	// Does this model have any geometry to render
	bool                     m_HasGeometry;

	// Vertex data for the model stored in a vertex buffer and the number of the vertices in the buffer
	ID3D10Buffer*            m_VertexBuffer;
	unsigned int             m_NumVertices;

	// Description of the elements in a single vertex (position, normal, UVs etc.)
	static const int         MAX_VERTEX_ELTS = 64;
	D3D10_INPUT_ELEMENT_DESC m_VertexElts[MAX_VERTEX_ELTS];
	ID3D10InputLayout*       m_VertexLayout; // Layout of a vertex (derived from above)
	unsigned int             m_VertexSize;   // Size of vertex calculated from contained elements

	// Index data for the model stored in a index buffer and the number of indices in the buffer
	ID3D10Buffer*            m_IndexBuffer;
	unsigned int             m_NumIndices;

	//---------------
	// Render data

	//The render technique for this model
	CTechnique* m_RenderTechnique;
	//Pointer to texture
	CMaterial* m_ModelMaterial;

	//Technique change variables
	string m_FileName;

	//--------------
	// Render effect variables
	
	//Effect variable to pass matrix to shader
	static ID3D10EffectMatrixVariable* m_MatrixVar;
	//Effect variable to pass colour to shader
	static ID3D10EffectVectorVariable* m_ColourVar;

	//Render technique for rendering shadow maps
	static CTechnique* m_ShadowRenderTechnique;

	D3DXVECTOR3 m_Colour;

	unsigned int m_CurrentMaterialIndex;
	unsigned int m_CurrentTechniqueIndex;

	unsigned int m_NumElements;

public:
	//Static data members
	static vector<CMaterial*> m_MaterialList;
	static vector<CTechnique*> m_TechniqueList;

/////////////////////////////
// Public member functions

	static void SetMatrixShaderVariable(ID3D10EffectMatrixVariable* matrixVar);
	static void SetColourShaderVariable(ID3D10EffectVectorVariable* colourVar);

	static void SetShadowRenderTechnique(CTechnique* shadowTechnique);

	///////////////////////////////
	// Constructors / Destructors

	// Constructor - initialise all settings, sensible defaults provided for everything.
	CModel( D3DXVECTOR3 position = D3DXVECTOR3(0,0,0), D3DXVECTOR3 rotation = D3DXVECTOR3(0,0,0), float scale = 1.0f, D3DXVECTOR3 colour = D3DXVECTOR3(0.0f, 0.0f, 0.0f ));

	// Destructor
	~CModel();

	// Release resources used by model
	void ReleaseResources();

	/////////////////////////////
	// Data access

	// Getters
	D3DXVECTOR3 GetPosition()
	{
		return m_Position;
	}
	D3DXVECTOR3 GetFacingVector()
	{
		D3DXVECTOR3 facing;
		D3DXVec3Normalize(&facing, &D3DXVECTOR3(&m_WorldMatrix(2, 0)));
		return facing;
	}
	D3DXVECTOR3 GetRotation()
	{
		return m_Rotation;
	}
	D3DXVECTOR3 GetScale()
	{
		return m_Scale;
	}
	D3DXMATRIX GetWorldMatrix()
	{
		return m_WorldMatrix;
	}


	// Setters
	void SetPosition( D3DXVECTOR3 position )
	{
		m_Position = position;
	}
	void SetRotation( D3DXVECTOR3 rotation )
	{
		m_Rotation = rotation;
	}
	void SetScale( D3DXVECTOR3 scale ) // Overloaded setter, two versions: this one sets x,y,z scale separately, the next sets all to the same value
	{
		m_Scale = scale;
	}
	void SetScale( float scale )
	{
		m_Scale = D3DXVECTOR3( scale, scale, scale );
	}
	void SetMaterial(CMaterial* material)
	{
		m_ModelMaterial = material;
	}
	bool SetRenderTechnique(CTechnique* renderTechnique)
	{
		if (renderTechnique->IsCompatible(m_ModelMaterial))	//First check that the new technique and this models texture are compatible
		{
			// Given the vertex element list, pass it to DirectX to create a vertex layout. We also need to pass an example of a technique that will
			// render this model. We will only be able to render this model with techniques that have the same vertex input as the example we use here
			D3D10_PASS_DESC PassDesc;
			renderTechnique->GetTechnique()->GetPassByIndex(0)->GetDesc(&PassDesc);
			g_pd3dDevice->CreateInputLayout(m_VertexElts, m_NumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_VertexLayout);

			m_RenderTechnique = renderTechnique;
			return true;
		}
		// Have not returned yet must have failed the if statement
		return false;
	}
	void SetColour(D3DXVECTOR3 colour)
	{
		m_Colour = colour;
	}
	
	/////////////////////////////
	// Model Loading

	// Load the model geometry from a file. This function only reads the geometry using the first material in the file, so multi-material
	// models will load but will have parts missing. May optionally request for tangents to be created for the model (for normal or parallax mapping)
	// We need to pass an example technique that the model will use to help DirectX understand how to connect this data with the vertex shaders
	// Returns true if the load was successful
	bool Load( const string& fileName, CTechnique* shaderCode );


	/////////////////////////////
	// Model Usage

	// Check if the models texture supports normals (and therefore needs tangents)
	bool UseTangents();

	// Update the world matrix of the model from its position, rotation and scaling
	void UpdateMatrix();
	
	// Make the model face a certain point in world space
	void FacePoint(D3DXVECTOR3 point);

	// Control the model's position and rotation using keys provided. Amount of motion performed depends on frame time
	void Control( float frameTime, EKeyCode turnUp, EKeyCode turnDown, EKeyCode turnLeft, EKeyCode turnRight,  
				  EKeyCode turnCW, EKeyCode turnCCW, EKeyCode moveForward, EKeyCode moveBackward );

	// Render the model with the given technique. Assumes any shader variables for the technique have already been set up (e.g. matrices and textures)
	void Render();

	void ShadowRender();
};


#endif // End of header guard - see top of file
