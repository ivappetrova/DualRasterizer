#include "BaseEffect.h"
#include <d3dcompiler.h>
#include <iostream>
#include <sstream>

using namespace dae;

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	if (m_pEffect)
	{
		m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
		m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
		m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");

		m_pCurrentTechnique = m_pPointTechnique;

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	}
}

BaseEffect::~BaseEffect()
{
	if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();
	if (m_pAnisotropicTechnique) m_pAnisotropicTechnique->Release();
	if (m_pLinearTechnique) m_pLinearTechnique->Release();
	if (m_pPointTechnique) m_pPointTechnique->Release();
	if (m_pEffect) m_pEffect->Release();
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(
		assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	return pEffect;
}

void BaseEffect::SetWorldViewProjectionMatrix(const Matrix& wvp)
{
	if (m_pMatWorldViewProjVariable && m_pMatWorldViewProjVariable->IsValid())
	{
		float matrixData[4][4];
		wvp.AsColMajArray(matrixData);
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<float*>(matrixData));
	}
}

void BaseEffect::SetFilteringMethod(SamplingMethod method)
{
	m_CurrentSamplingMethod = method;

	switch (method)
	{
	case SamplingMethod::Point:
		m_pCurrentTechnique = m_pPointTechnique;
		std::cout << "Switched to Point Filtering\n";
		break;
	case SamplingMethod::Linear:
		m_pCurrentTechnique = m_pLinearTechnique;
		std::cout << "Switched to Linear Filtering\n";
		break;
	case SamplingMethod::Anisotropic:
		m_pCurrentTechnique = m_pAnisotropicTechnique;
		std::cout << "Switched to Anisotropic Filtering\n";
		break;
	}
}