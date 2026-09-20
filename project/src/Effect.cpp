#include "Effect.h"
#include <iostream>
#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		m_pCameraPositionVariable = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	}
}

Effect::~Effect()
{
	if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->Release();
	if (m_pSpecularMapVariable) m_pSpecularMapVariable->Release();
	if (m_pNormalMapVariable) m_pNormalMapVariable->Release();
	if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();
	if (m_pCameraPositionVariable) m_pCameraPositionVariable->Release();
	if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
}

void Effect::SetWorldMatrix(const Matrix& world)
{
	if (m_pMatWorldVariable && m_pMatWorldVariable->IsValid())
	{
		Matrix transposed = Matrix::Transpose(world);
		float matrixData[4][4];
		transposed.AsColMajArray(matrixData);
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<float*>(matrixData));
	}
}

void Effect::SetCameraPosition(const Vector3& cameraPos)
{
	if (m_pCameraPositionVariable && m_pCameraPositionVariable->IsValid())
	{
		m_pCameraPositionVariable->SetFloatVector(&cameraPos.x);
	}
}

void Effect::SetDiffuseMap(Texture_HARDWARE* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable && pDiffuseTexture)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}

void Effect::SetNormalMap(Texture_HARDWARE* pNormalTexture)
{
	if (m_pNormalMapVariable && pNormalTexture)
	{
		m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
	}
}

void Effect::SetSpecularMap(Texture_HARDWARE* pSpecularTexture)
{
	if (m_pSpecularMapVariable && pSpecularTexture)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
	}
}

void Effect::SetGlossinessMap(Texture_HARDWARE* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable && pGlossinessTexture)
	{
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
	}
}