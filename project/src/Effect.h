#pragma once
#include "BaseEffect.h"

namespace dae
{
	class Texture_HARDWARE;

	class Effect final : public BaseEffect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		void SetWorldMatrix(const Matrix& world);
		void SetCameraPosition(const Vector3& cameraPos);

		void SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture);
		void SetNormalMap(Texture_HARDWARE* pNormalTexture);
		void SetSpecularMap(Texture_HARDWARE* pSpecularTexture);
		void SetGlossinessMap(Texture_HARDWARE* pGlossinessTexture);

	private:
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectVectorVariable* m_pCameraPositionVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};
	};
}
