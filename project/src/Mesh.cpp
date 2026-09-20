#include "Mesh.h"
#include "Effect.h"
#include "FlatEffect.h"
#include <cassert>

using namespace dae;

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices, EffectType effectType)
	: m_NumIndices(static_cast<uint32_t>(indices.size()))
	, m_EffectType(effectType)
{
	// Create Effect instance based on type
	if (effectType == EffectType::Flat)
	{
		m_pEffect = new FlatEffect(pDevice, L"resources/FlatShading.fx");
	}
	else
	{
		m_pEffect = new Effect(pDevice, L"resources/PosCol3D.fx");
	}

	// Set attris
	static constexpr uint32_t NUM_ELEMENTS{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[NUM_ELEMENTS]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		NUM_ELEMENTS,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
	{
		assert(false && "Failed to create Input Layout");
		return;
	}

	// Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_In) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		assert(false && "Failed to create Vertex Buffer");
		return;
	}

	// Index Buffer
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		assert(false && "Failed to create Index Buffer");
		return;
	}
}

Mesh::~Mesh()
{
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pEffect) delete m_pEffect;
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
	pDeviceContext->IASetInputLayout(m_pInputLayout);
	constexpr UINT STRIDE = sizeof(Vertex_In);
	constexpr UINT OFFSET = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &STRIDE, &OFFSET);
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::SetWorldViewProjectionMatrix(const Matrix& wvp)
{
	m_WorldViewProjectionMatrix = wvp;
	if (m_pEffect)
	{
		Matrix transposed = Matrix::Transpose(transposed);
		m_pEffect->SetWorldViewProjectionMatrix(wvp);
	}
}

void Mesh::SetWorldMatrix(const Matrix& world)
{
	m_WorldMatrix = world;
	if (m_pEffect && m_EffectType == EffectType::Standard)
	{
		Effect* pStandardEffect = static_cast<Effect*>(m_pEffect);
		pStandardEffect->SetWorldMatrix(world);
	}
}

void Mesh::SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture)
{
	m_pDiffuseTextureHW = pDiffuseTexture;
	if (m_pEffect && m_pDiffuseTextureHW)
	{
		if (m_EffectType == EffectType::Standard)
		{
			Effect* pStandardEffect = static_cast<Effect*>(m_pEffect);
			pStandardEffect->SetDiffuseMap(m_pDiffuseTextureHW);
		}
		else
		{
			FlatEffect* pFlatEffect = static_cast<FlatEffect*>(m_pEffect);
			pFlatEffect->SetDiffuseMap(m_pDiffuseTextureHW);
		}
	}
}

void Mesh::SetNormalMap(Texture_HARDWARE* pNormalTexture)
{
	m_pNormalTextureHW = pNormalTexture;
	if (m_pEffect && m_pNormalTextureHW && m_EffectType == EffectType::Standard)
	{
		Effect* pStandardEffect = static_cast<Effect*>(m_pEffect);
		pStandardEffect->SetNormalMap(m_pNormalTextureHW);
	}
}

void Mesh::SetSpecularMap(Texture_HARDWARE* pSpecularTexture)
{
	m_pSpecularTextureHW = pSpecularTexture;
	if (m_pEffect && m_pSpecularTextureHW && m_EffectType == EffectType::Standard)
	{
		Effect* pStandardEffect = static_cast<Effect*>(m_pEffect);
		pStandardEffect->SetSpecularMap(m_pSpecularTextureHW);
	}
}

void Mesh::SetGlossinessMap(Texture_HARDWARE* pGlossinessTexture)
{
	m_pGlossinessTextureHW = pGlossinessTexture;
	if (m_pEffect && m_pGlossinessTextureHW && m_EffectType == EffectType::Standard)
	{
		Effect* pStandardEffect = static_cast<Effect*>(m_pEffect);
		pStandardEffect->SetGlossinessMap(m_pGlossinessTextureHW);
	}
}

BaseEffect* Mesh::GetEffect() const
{
	return m_pEffect;
}