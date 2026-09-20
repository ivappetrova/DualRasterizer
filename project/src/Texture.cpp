#include "Texture.h"
#include <SDL_image.h>
#include <cassert>
#include <iostream>

#include "Vector2.h"

using namespace dae;

//////////////////////////////////////////////////////////// HARDWARE TEXTURE

Texture_HARDWARE::Texture_HARDWARE(ID3D11Device* pDevice, const std::string& filePath)
{
	SDL_Surface* pSurface = IMG_Load(filePath.c_str());
	if (!pSurface)
	{
		std::cout << "Failed to load texture: " << filePath << std::endl;
		std::cout << "SDL_Image Error: " << IMG_GetError() << std::endl;
		assert(false && "Failed to load texture");
		return;
	}

	// Use linear format for all textures - we'll handle gamma in the shader
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Create texture description
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// Fill subresource data
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	// Create texture
	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if (FAILED(hr))
	{
		assert(false && "Failed to create texture");
		SDL_FreeSurface(pSurface);
		return;
	}

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);
	if (FAILED(hr))
	{
		assert(false && "Failed to create shader resource view");
	}

	// Free SDL surface
	SDL_FreeSurface(pSurface);
}

Texture_HARDWARE::~Texture_HARDWARE()
{
	if (m_pShaderResourceView)
		m_pShaderResourceView->Release();
	if (m_pResource)
		m_pResource->Release();
}

//////////////////////////////////////////////////////////// SOFTWARE TEXTURE

Texture_SOFTWARE::Texture_SOFTWARE(SDL_Surface* pSurface) :
	m_pSurface{ pSurface },
	m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
{
}

Texture_SOFTWARE::~Texture_SOFTWARE()
{
	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}

std::unique_ptr<Texture_SOFTWARE> Texture_SOFTWARE::LoadFromFile(const std::string& path)
{
	SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
	if (!pSurface) return nullptr;

	return std::make_unique<Texture_SOFTWARE>(pSurface);
}


ColorRGB Texture_SOFTWARE::Sample(const Vector2& uv) const
{
	const float U{ uv.x * m_pSurface->w };
	const float V{ uv.y * m_pSurface->h };
	int x{ static_cast<int>(U) };
	int y{ static_cast<int>(V) };
	x = std::clamp(x, 0, m_pSurface->w - 1);
	y = std::clamp(y, 0, m_pSurface->h - 1);
	const int PIXEL_INDEX{ x + (y * m_pSurface->w) };
	const uint32_t PIXEL{ m_pSurfacePixels[PIXEL_INDEX] };
	uint8_t r;
	uint8_t g;
	uint8_t b;
	SDL_GetRGB(PIXEL, m_pSurface->format, &r, &g, &b);

	return ColorRGB{ r / 255.f, g / 255.f, b / 255.f };
}
