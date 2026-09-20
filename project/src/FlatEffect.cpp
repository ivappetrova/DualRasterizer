#include "FlatEffect.h"
#include <iostream>
#include "Texture.h"

using namespace dae;

FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	}
}

FlatEffect::~FlatEffect()
{
	if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();
}

void FlatEffect::SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable && pDiffuseTexture)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}