#pragma once
#include "BaseEffect.h"

namespace dae
{
	class Texture_HARDWARE;

	class FlatEffect final : public BaseEffect
	{
	public:
		FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~FlatEffect();

		FlatEffect(const FlatEffect&) = delete;
		FlatEffect(FlatEffect&&) noexcept = delete;
		FlatEffect& operator=(const FlatEffect&) = delete;
		FlatEffect& operator=(FlatEffect&&) noexcept = delete;

		void SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture);

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	};
}
