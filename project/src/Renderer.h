#pragma once

// SDL Headers
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

// DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

// Framework Headers
#include "Timer.h"
#include "Effect.h"
#include "Mesh.h"
#include <memory>
#include "DataTypes.h"

namespace dae
{
	class Camera;
	class Texture_HARDWARE;
	class Texture_SOFTWARE;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();

		void ToggleRenderMode();
		void ToggleRotation();
		void ToggleFireMesh();
		void CycleSamplingState();
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBoxVisualization();
		void CycleCullMode();
		void ToggleUniformClearColor();

		bool m_UseHardwareRenderer{ true };

	private:
		SDL_Window* m_pWindow{};
		int m_Width{};
		int m_Height{};
		bool m_IsInitialized{ false };

		//////////////////////////////////////////////////////////////////////////// SHARED
		// Data
		std::unique_ptr<Camera> m_pCamera;

		bool m_IsRotating{ true };
		float m_MeshRotation{ 0.0f };
		CullMode m_CurrentCullMode{ CullMode::Back };
		bool m_UseUniformClearColor{ false };

		// Functions
		void InitializeSharedResources();
	

		//////////////////////////////////////////////////////////////////////////// HARDWARE
		// Data
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Texture2D* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		ID3D11RasterizerState* m_pRasterizerStateBack{};
		ID3D11RasterizerState* m_pRasterizerStateFront{};
		ID3D11RasterizerState* m_pRasterizerStateNone{};

		std::unique_ptr<Mesh> m_pVehicleMesh;
		std::unique_ptr<Texture_HARDWARE> m_pDiffuseTextureHW;
		std::unique_ptr<Texture_HARDWARE> m_pNormalTextureHW;
		std::unique_ptr<Texture_HARDWARE> m_pSpecularTextureHW;
		std::unique_ptr<Texture_HARDWARE> m_pGlossinessTextureHW;

		bool m_ShowFireMesh{ true };
		std::unique_ptr<Mesh> m_pFireMesh;
		std::unique_ptr<Texture_HARDWARE> m_pFireTexture;

		SamplingMethod m_CurrentSamplingMethod{ SamplingMethod::Point };

		// Functions
		void InitializeHardwareResources();
		void RenderHardware();
		HRESULT InitializeDirectX();

		//////////////////////////////////////////////////////////////////////////// SOFTWARE
		// Data
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		std::unique_ptr<float[]> m_pDepthBufferPixels;

		std::unique_ptr<Texture_SOFTWARE> m_pDiffuseTextureSW;
		std::unique_ptr<Texture_SOFTWARE> m_pNormalTextureSW;
		std::unique_ptr<Texture_SOFTWARE> m_pSpecularTextureSW;
		std::unique_ptr<Texture_SOFTWARE> m_pGlossinessTextureSW;
		std::vector<Vertex> m_OriginalVertices;
		std::vector<uint32_t> m_MeshIndices;

		ShadingMode m_CurrentLightingMode{ ShadingMode::ObservedArea };

		bool m_ShowDepthBuffer{ false };
		bool m_UseNormalMap{ true };
		bool m_ShowBoundingBoxes{ false };

		// Functions
		void InitializeSoftwareResources();

		void RenderSoftware();

		// Helper functions
		void VertexTransformationFunction(const std::vector<Vertex>& in, std::vector<Vector4>& out, std::vector<Vector3>& outNormals,
			std::vector<Vector3>& outTangents, const Matrix& worldMatrix) const;
		Vector2 NDCToScreen(const Vector3& ndc) const;
		void ResetDepthBuffer();
		float Remap(float value, float min, float max) const;

		void RenderTriangleList(const std::vector<Mesh_SOFTWARE>& meshes_world, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
			std::vector<BoundingBox>& boxes, size_t meshIndex);

		void RenderTriangleStrip(const std::vector<Mesh_SOFTWARE>& meshes_world, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
			std::vector<BoundingBox>& boxes, size_t meshIndex);

		void CreateBoundingBox(const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
			int v0, int v1, int v2, size_t meshIndex, std::vector<BoundingBox>& boxes);

		bool ProcessPixel(int PX, int PY, const BoundingBox& bx, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
			const std::vector<Vertex>& vertices_world, const std::vector<Vector3>& transformed_normals,
			const std::vector<Vector3>& transformed_tangents);

		ColorRGB PixelShading(const Vertex& v);
		Vector3 CalculateFinalNormal(const Vertex& v);
		ColorRGB CalculateDiffuseColor(const Vertex& v, float observedArea);
		ColorRGB CalculatePhongColor(const Vertex& v, const Vector3& lightDirection, const Vector3& finalNormal);

		void DrawBoundingBox(const BoundingBox& box);
	};
}