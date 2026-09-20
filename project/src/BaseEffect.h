#pragma once
#include <d3d11.h>
#include <d3dx11effect.h>
#include <string>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	class BaseEffect
	{
	public:
		BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~BaseEffect();

		BaseEffect(const BaseEffect&) = delete;
		BaseEffect(BaseEffect&&) noexcept = delete;
		BaseEffect& operator=(const BaseEffect&) = delete;
		BaseEffect& operator=(BaseEffect&&) noexcept = delete;

		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pCurrentTechnique; }

		void SetWorldViewProjectionMatrix(const Matrix& wvp);
		void SetFilteringMethod(SamplingMethod method);
		SamplingMethod GetFilteringMethod() const { return m_CurrentSamplingMethod; }

	protected:
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pCurrentTechnique{};
		ID3DX11EffectTechnique* m_pPointTechnique{};
		ID3DX11EffectTechnique* m_pLinearTechnique{};
		ID3DX11EffectTechnique* m_pAnisotropicTechnique{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		SamplingMethod m_CurrentSamplingMethod{ SamplingMethod::Point };
	};
}