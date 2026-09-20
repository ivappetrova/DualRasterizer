#pragma once
#include <d3d11.h>
#include <vector>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	class BaseEffect;
	class Texture_HARDWARE;

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices, EffectType effectType = EffectType::Standard);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void SetWorldViewProjectionMatrix(const Matrix& wvp);
		void SetWorldMatrix(const Matrix& world);

		void SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture);
		void SetNormalMap(Texture_HARDWARE* pNormalTexture);
		void SetSpecularMap(Texture_HARDWARE* pSpecularTexture);
		void SetGlossinessMap(Texture_HARDWARE* pGlossinessTexture);

		BaseEffect* GetEffect() const;

	private:
		BaseEffect* m_pEffect{};
		EffectType m_EffectType;
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};
		uint32_t m_NumIndices{};

		Matrix m_WorldViewProjectionMatrix{};
		Matrix m_WorldMatrix{};

		Texture_HARDWARE* m_pDiffuseTextureHW{};
		Texture_HARDWARE* m_pNormalTextureHW{};
		Texture_HARDWARE* m_pSpecularTextureHW{};
		Texture_HARDWARE* m_pGlossinessTextureHW{};
	};
}