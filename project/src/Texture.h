#pragma once

#include <d3d11.h>
#include <string>

#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include <memory>

namespace dae
{
	////////////////////////////////////////////////////////// HARDWARE TEXTURE

	class Texture_HARDWARE final
	{
	public:
		Texture_HARDWARE(ID3D11Device* pDevice, const std::string& filePath);
		~Texture_HARDWARE();

		Texture_HARDWARE(const Texture_HARDWARE&) = delete;
		Texture_HARDWARE(Texture_HARDWARE&&) noexcept = delete;
		Texture_HARDWARE& operator=(const Texture_HARDWARE&) = delete;
		Texture_HARDWARE& operator=(Texture_HARDWARE&&) noexcept = delete;

		ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; }

	private:
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pShaderResourceView{};
	};

////////////////////////////////////////////////////////// SOFTWARE TEXTURE

	struct Vector2;
	class Texture_SOFTWARE final
	{
	public:
		Texture_SOFTWARE(SDL_Surface* pSurface);
		~Texture_SOFTWARE();

		static std::unique_ptr<Texture_SOFTWARE> LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

	private:
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}