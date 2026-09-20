//External includes
#include "SDL.h"
#include "SDL_surface.h"

// Standard includes
#include <iostream>

//Project includes
#include "Renderer.h"
#include "Camera.h"
#include "Texture.h"
#include "FlatEffect.h"

#include "Utils.h"

#include <execution>

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	InitializeSharedResources();
	InitializeSoftwareResources();
	InitializeHardwareResources();
}

void dae::Renderer::InitializeSharedResources()
{
	m_pCamera = std::make_unique<Camera>();
	m_pCamera->Initialize(Vector3(0.0f, 0.0f, 0.0f), 45.0f, 0.1f, 100.0f);
	m_pCamera->CalculateProjectionMatrix(static_cast<float>(m_Width) / static_cast<float>(m_Height));
}

void dae::Renderer::InitializeHardwareResources()
{
	const HRESULT result = InitializeDirectX();
	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}

	std::vector<Vertex_In> vertices;
	std::vector<uint32_t> indices;

	if (Utils::ParseOBJ("resources/vehicle.obj", vertices, indices))
	{
		std::cout << "Successfully loaded vehicle mesh" << std::endl;
		std::cout << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;

		m_pVehicleMesh = std::make_unique<Mesh>(m_pDevice, vertices, indices);
		m_pDiffuseTextureHW = std::make_unique<Texture_HARDWARE>(m_pDevice, "resources/vehicle_diffuse.png");
		m_pNormalTextureHW = std::make_unique<Texture_HARDWARE>(m_pDevice, "resources/vehicle_normal.png");
		m_pSpecularTextureHW = std::make_unique<Texture_HARDWARE>(m_pDevice, "resources/vehicle_specular.png");
		m_pGlossinessTextureHW = std::make_unique<Texture_HARDWARE>(m_pDevice, "resources/vehicle_gloss.png");

		// Set all textures to mesh
		m_pVehicleMesh->SetDiffuseMap(m_pDiffuseTextureHW.get());
		m_pVehicleMesh->SetNormalMap(m_pNormalTextureHW.get());
		m_pVehicleMesh->SetSpecularMap(m_pSpecularTextureHW.get());
		m_pVehicleMesh->SetGlossinessMap(m_pGlossinessTextureHW.get());
	}
	else
	{
		std::cout << "Failed to load vehicle mesh!" << std::endl;
	}

	vertices.clear();
	indices.clear();

	if (Utils::ParseOBJ("resources/fireFX.obj", vertices, indices))
	{
		std::cout << "Successfully loaded fire mesh" << std::endl;
		std::cout << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;

		m_pFireMesh = std::make_unique<Mesh>(m_pDevice, vertices, indices, EffectType::Flat);
		m_pFireTexture = std::make_unique<Texture_HARDWARE>(m_pDevice, "resources/fireFX_diffuse.png");
		m_pFireMesh->SetDiffuseMap(m_pFireTexture.get());
	}
	else
	{
		std::cout << "Failed to load fire mesh!" << std::endl;
	}
}

HRESULT Renderer::InitializeDirectX()
{
	// 1. Create DXGI Factory to enumerate adapters
	IDXGIFactory1* pDXGIFactory{};
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDXGIFactory));
	if (FAILED(result)) return result;

	IDXGIAdapter* adapter = nullptr;
	IDXGIAdapter* selectedAdapter = nullptr;

	for (UINT i = 0; pDXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC desc{};
		adapter->GetDesc(&desc);
		std::wcout << L"Adapter " << i << L": " << desc.Description << L"\n";

		if (desc.VendorId != 0x8086) // Not Intel
		{
			selectedAdapter = adapter;
			break;
		}
		else
		{
			adapter->Release();
		}
	}

	if (selectedAdapter == nullptr)
	{
		if (pDXGIFactory) pDXGIFactory->Release();
		return E_FAIL;
	}

	// 2. Create device and device context with selected adapter
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	result = D3D11CreateDevice(
		selectedAdapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		0,
		createDeviceFlags,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&m_pDevice,
		nullptr,
		&m_pDeviceContext);

	if (FAILED(result))
	{
		if (selectedAdapter) selectedAdapter->Release();
		if (pDXGIFactory) pDXGIFactory->Release();
		return result;
	}

	// 3. Create swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Get the handle (HWND) from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_GetVersion(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	// Create SwapChain
	result = pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
	{
		if (selectedAdapter) selectedAdapter->Release();
		if (pDXGIFactory) pDXGIFactory->Release();
		return result;
	}

	// 4. Create DepthStencil (DS) & DepthStencilView (DSV)
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result)) return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result)) return result;

	// 5. Create RenderTarget (RT) & RenderTargetView (RTV)
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result)) return result;

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result)) return result;

	// 6. Bind RTV & DSV to Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 7. Set Viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	if (selectedAdapter) selectedAdapter->Release();
	if (pDXGIFactory) pDXGIFactory->Release();

	// Rasterizer States for culling
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	// Back
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateBack);
	if (FAILED(result)) return result;

	// Front
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateFront);
	if (FAILED(result)) return result;

	// None
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateNone);
	if (FAILED(result)) return result;

	return S_OK;
}

void dae::Renderer::InitializeSoftwareResources()
{
	m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = std::make_unique<float[]>(m_Width * m_Height);

	m_pDiffuseTextureSW = Texture_SOFTWARE::LoadFromFile("resources/vehicle_diffuse.png");
	m_pNormalTextureSW = Texture_SOFTWARE::LoadFromFile("resources/vehicle_normal.png");
	m_pSpecularTextureSW = Texture_SOFTWARE::LoadFromFile("resources/vehicle_specular.png");
	m_pGlossinessTextureSW = Texture_SOFTWARE::LoadFromFile("resources/vehicle_gloss.png");
	Utils::ParseOBJ("resources/vehicle.obj", m_OriginalVertices, m_MeshIndices);
}


Renderer::~Renderer()
{
	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
	if (m_pDepthStencilView) m_pDepthStencilView->Release();
	if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pRasterizerStateNone) m_pRasterizerStateNone->Release();
	if (m_pRasterizerStateFront) m_pRasterizerStateFront->Release();
	if (m_pRasterizerStateBack) m_pRasterizerStateBack->Release();
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice) m_pDevice->Release();
}

void Renderer::Update(const Timer* pTimer)
{
	if (m_pCamera)
	{
		m_pCamera->Update(pTimer);
	}

	const float DEGREES_PER_SECOND{45.f};
	if (m_IsRotating)
	{
		m_MeshRotation += DEGREES_PER_SECOND * TO_RADIANS * pTimer->GetElapsed();
	}
}

void Renderer::Render()
{
	if (!m_IsInitialized)
		return;

	if (m_UseHardwareRenderer)
	{
		RenderHardware();
	}
	else
	{
		RenderSoftware();
	}
}

void dae::Renderer::RenderHardware()
{
	// 1. Clear RTV & DSV
	float color[4];
	if (m_UseUniformClearColor)
	{
		color[0] = 0.1f;
		color[1] = 0.1f;
		color[2] = 0.1f;
		color[3] = 1.0f;
	}
	else
	{
		color[0] = 0.39f;
		color[1] = 0.59f;
		color[2] = 0.93f;
		color[3] = 1.0f;
	}

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RasterizerState* currentRasterizerState = nullptr;
	switch (m_CurrentCullMode)
	{
	case CullMode::Back:
		currentRasterizerState = m_pRasterizerStateBack;
		break;
	case CullMode::Front:
		currentRasterizerState = m_pRasterizerStateFront;
		break;
	case CullMode::None:
		currentRasterizerState = m_pRasterizerStateNone;
		break;
	}
	m_pDeviceContext->RSSetState(currentRasterizerState);

	// 2. Set pipeline + draw calls

	// Vehicle
	if (m_pVehicleMesh && m_pCamera)
	{
		Matrix worldMatrix = Matrix::CreateRotationY(m_MeshRotation) * Matrix::CreateTranslation(0.f, 0.f, 50.f);
		Matrix viewProjectionMatrix = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();
		Matrix worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;

		m_pVehicleMesh->SetWorldMatrix(worldMatrix);
		m_pVehicleMesh->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);

		BaseEffect* pBaseEffect = m_pVehicleMesh->GetEffect();
		if (pBaseEffect)
		{
			Effect* pEffect = static_cast<Effect*>(pBaseEffect);
			pEffect->SetCameraPosition(m_pCamera->GetPosition());
		}

		m_pVehicleMesh->Render(m_pDeviceContext);
		m_pDeviceContext->RSSetState(currentRasterizerState);
	}

	// Fire
	if (m_ShowFireMesh && m_pFireMesh && m_pCamera)
	{
		Matrix worldMatrix = Matrix::CreateRotationY(m_MeshRotation) * Matrix::CreateTranslation(0.f, 0.f, 50.f);
		Matrix viewProjectionMatrix = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();
		Matrix worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
		m_pFireMesh->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);

		m_pFireMesh->Render(m_pDeviceContext);
		m_pDeviceContext->RSSetState(currentRasterizerState);
	}

	// 3. Present backbuffer (swap)
	m_pSwapChain->Present(0, 0);
}

void dae::Renderer::RenderSoftware()
{
	if (m_UseUniformClearColor)
	{
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 25, 25, 25));
	}
	else
	{
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 70, 70, 70));
	}

	ResetDepthBuffer();

	std::vector<Mesh_SOFTWARE> meshes_world
	{
		Mesh_SOFTWARE{
			m_OriginalVertices,
			m_MeshIndices,
			PrimitiveTopology::TriangleList
		},
	};

	std::vector<std::vector<Vector4>> allVerticesNDC;
	std::vector<std::vector<Vector2>> allVerticesScreen;
	std::vector<std::vector<Vector3>> allTransformedNormals;
	std::vector<std::vector<Vector3>> allTransformedTangents;
	std::vector<BoundingBox> boxes;

	//////////////////////////////////////////////////////////////////////////////////////// PROJECTION STAGE - Transform all meshes one by one
	for (size_t meshIndex{}; meshIndex < meshes_world.size(); ++meshIndex)
	{
		const Mesh_SOFTWARE& CURRENT_MESH{ meshes_world[meshIndex] };

		std::vector<Vertex> currMeshVerticesWorld{ CURRENT_MESH.vertices };
		std::vector<Vector4> currMeshVerticesNDC(currMeshVerticesWorld.size());
		std::vector<Vector3> currMeshTransformedNormals(currMeshVerticesWorld.size());
		std::vector<Vector3> currMeshTransformedTangents(currMeshVerticesWorld.size());

		Matrix worldMatrix{ Matrix::CreateRotationY(m_MeshRotation) * Matrix::CreateTranslation(0.f, 0.f, 50.f) };
		VertexTransformationFunction(currMeshVerticesWorld, currMeshVerticesNDC, currMeshTransformedNormals, currMeshTransformedTangents, worldMatrix);

		allVerticesNDC.push_back(currMeshVerticesNDC);
		allTransformedNormals.push_back(currMeshTransformedNormals);
		allTransformedTangents.push_back(currMeshTransformedTangents);

		////////////////////////////////////////////////////////////////////////////////////// RASTERIZATION STAGE - Convert NDC to screen space for the current mesh
		std::vector<Vector2> currMeshVerticesScreen(currMeshVerticesWorld.size());
		for (size_t index{}; index < currMeshVerticesWorld.size(); ++index)
		{
			currMeshVerticesScreen[index] = NDCToScreen(Vector3{ currMeshVerticesNDC[index].x, currMeshVerticesNDC[index].y, currMeshVerticesNDC[index].z });
		}
		allVerticesScreen.push_back(currMeshVerticesScreen);

		// build bounding boxes based on primitive topology
		if (CURRENT_MESH.primitiveTopology == PrimitiveTopology::TriangleList)
		{
			RenderTriangleList(meshes_world, currMeshVerticesScreen, currMeshVerticesNDC, boxes, meshIndex);
		}
		else
		{
			RenderTriangleStrip(meshes_world, currMeshVerticesScreen, currMeshVerticesNDC, boxes, meshIndex);
		}
	}

	// PIXEL SHADING - Parallel execution
	std::for_each(std::execution::par, boxes.begin(), boxes.end(),
		[&](const BoundingBox& bx)
		{
			const std::vector<Vector2>& CURR_MESH_VERTICES_SCREEN{ allVerticesScreen[bx.meshIndex] };
			const std::vector<Vector4>& CURR_MESH_VERTICES_NDC{ allVerticesNDC[bx.meshIndex] };
			const std::vector<Vertex>& VERTICES_WORLD{ meshes_world[bx.meshIndex].vertices };
			const std::vector<Vector3>& TRANSFORMED_NORMALS{ allTransformedNormals[bx.meshIndex] };
			const std::vector<Vector3>& TRANSFORMED_TANGENTS{ allTransformedTangents[bx.meshIndex] };

			for (int PY{ bx.minY }; PY <= bx.maxY; ++PY)
			{
				for (int PX{ bx.minX }; PX <= bx.maxX; ++PX)
				{
					ProcessPixel(PX, PY, bx, CURR_MESH_VERTICES_SCREEN, CURR_MESH_VERTICES_NDC, VERTICES_WORLD, TRANSFORMED_NORMALS, TRANSFORMED_TANGENTS);
				}
			}
		});

	if (m_ShowBoundingBoxes)
	{
		for (const auto& box : boxes)
		{
			DrawBoundingBox(box);
		}
	}

	SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::ResetDepthBuffer()
{
	const size_t NUM_PIXELS{ static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height) };
	for (size_t index{}; index < NUM_PIXELS; ++index)
	{
		m_pDepthBufferPixels[index] = FLT_MAX;
	}
}

void dae::Renderer::VertexTransformationFunction(const std::vector<Vertex>& in, std::vector<Vector4>& out, std::vector<Vector3>& outNormals,
	std::vector<Vector3>& outTangents, const Matrix& worldMatrix) const
{
	const Matrix WORLD_VIEW_PROJECTION_MATRIX{ worldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix()};

	for (size_t index{}; index < in.size(); ++index)
	{
		const Vector4 CLIP_SPACE_POS{ WORLD_VIEW_PROJECTION_MATRIX.TransformPoint(Vector4{ in[index].position.x, in[index].position.y, in[index].position.z, 1.0f }) };
		const float W{ CLIP_SPACE_POS.w };

		const bool IS_OUTSSIDE_FRUSTUM{ (CLIP_SPACE_POS.x < -W || CLIP_SPACE_POS.x > W ||
										  CLIP_SPACE_POS.y < -W || CLIP_SPACE_POS.y > W ||
										  CLIP_SPACE_POS.z < 0.0f || CLIP_SPACE_POS.z > W) };

		if (IS_OUTSSIDE_FRUSTUM)
		{
			out[index] = Vector4{ 0.0f, 0.0f, 0.0f, -1.0f };
			outNormals[index] = Vector3::Zero;
			outTangents[index] = Vector3::Zero;
			continue;
		}

		// perspective divide = ndc coords
		Vector4 ndcPosition;
		ndcPosition.x = CLIP_SPACE_POS.x / W;
		ndcPosition.y = CLIP_SPACE_POS.y / W;
		ndcPosition.z = CLIP_SPACE_POS.z / W;
		ndcPosition.w = W;

		out[index] = ndcPosition;

		Vector3 transformedNormal{ worldMatrix.TransformVector(in[index].normal) };
		outNormals[index] = transformedNormal.Normalized();
		Vector3 transformedTangent{ worldMatrix.TransformVector(in[index].tangent) };
		outTangents[index] = transformedTangent.Normalized();
	}
}

Vector2 Renderer::NDCToScreen(const Vector3& ndc) const
{
	return Vector2{ (ndc.x + 1.0f) * 0.5f * static_cast<float>(m_Width),
					(1.0f - ndc.y) * 0.5f * static_cast<float>(m_Height) };
}

float dae::Renderer::Remap(float value, float min, float max) const
{
	return (value - min) / (max - min);
}

void dae::Renderer::RenderTriangleList(const std::vector<Mesh_SOFTWARE>& meshes_world, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
	std::vector<BoundingBox>& boxes, size_t meshIndex) 
{
	for (size_t t{}; t < meshes_world[meshIndex].indices.size() / 3; ++t)
	{
		int v0 = meshes_world[meshIndex].indices[t * 3 + 0];
		int v1 = meshes_world[meshIndex].indices[t * 3 + 1];
		int v2 = meshes_world[meshIndex].indices[t * 3 + 2];

		CreateBoundingBox(vertices_screen, vertices_ndc, v0, v1, v2, meshIndex, boxes);
	}
}

void dae::Renderer::RenderTriangleStrip(const std::vector<Mesh_SOFTWARE>& meshes_world, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
	std::vector<BoundingBox>& boxes, size_t meshIndex)
{
	for (size_t t{}; t < meshes_world[meshIndex].indices.size() - 2; ++t)
	{
		int v0;
		int v1;
		int v2;

		// handle winding
		if (t & 1) // odd
		{
			v0 = meshes_world[meshIndex].indices[t];
			v1 = meshes_world[meshIndex].indices[t + 2];
			v2 = meshes_world[meshIndex].indices[t + 1];
		}
		else // even
		{
			v0 = meshes_world[meshIndex].indices[t];
			v1 = meshes_world[meshIndex].indices[t + 1];
			v2 = meshes_world[meshIndex].indices[t + 2];
		}

		CreateBoundingBox(vertices_screen, vertices_ndc, v0, v1, v2, meshIndex, boxes);
	}
}

void dae::Renderer::CreateBoundingBox(const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
	int v0, int v1, int v2, size_t meshIndex, std::vector<BoundingBox>& boxes)
{
	// Skip if any vertex was culled (w < 0)
	if (vertices_ndc[v0].w < 0 || vertices_ndc[v1].w < 0 || vertices_ndc[v2].w < 0) return;

	const float TOTAL_AREA = std::abs(Vector2::Cross(vertices_screen[v1] - vertices_screen[v0], vertices_screen[v2] - vertices_screen[v0])) * 0.5f;

	// Skip degenerate triangles
	if (TOTAL_AREA < 0.001f) return;

	float minX = std::floor(std::min({ vertices_screen[v0].x, vertices_screen[v1].x, vertices_screen[v2].x }));
	float maxX = std::ceil(std::max({ vertices_screen[v0].x, vertices_screen[v1].x, vertices_screen[v2].x }));
	float minY = std::floor(std::min({ vertices_screen[v0].y, vertices_screen[v1].y, vertices_screen[v2].y }));
	float maxY = std::ceil(std::max({ vertices_screen[v0].y, vertices_screen[v1].y, vertices_screen[v2].y }));

	minX = std::clamp(minX, 0.f, static_cast<float>(m_Width - 1));
	maxX = std::clamp(maxX, 0.f, static_cast<float>(m_Width - 1));
	minY = std::clamp(minY, 0.f, static_cast<float>(m_Height - 1));
	maxY = std::clamp(maxY, 0.f, static_cast<float>(m_Height - 1));

	boxes.push_back({ static_cast<int>(minX), static_cast<int>(maxX),
					  static_cast<int>(minY), static_cast<int>(maxY),
					  v0, v1, v2, TOTAL_AREA, meshIndex });
}

bool dae::Renderer::ProcessPixel(int PX, int PY, const BoundingBox& bx, const std::vector<Vector2>& vertices_screen, const std::vector<Vector4>& vertices_ndc,
	const std::vector<Vertex>& vertices_world, const std::vector<Vector3>& transformed_normals,
	const std::vector<Vector3>& transformed_tangents)
{
	const Vector2 P{ PX + 0.5f, PY + 0.5f };

	// Calculate edge vectors for culling check
	Vector2 edge0 = vertices_screen[bx.v1] - vertices_screen[bx.v0];
	Vector2 edge1 = vertices_screen[bx.v2] - vertices_screen[bx.v1];
	Vector2 edge2 = vertices_screen[bx.v0] - vertices_screen[bx.v2];

	// Barycentric weights (signed for culling)
	const float w0 = Vector2::Cross(vertices_screen[bx.v1] - P, vertices_screen[bx.v2] - P);
	const float w1 = Vector2::Cross(vertices_screen[bx.v2] - P, vertices_screen[bx.v0] - P);
	const float w2 = Vector2::Cross(vertices_screen[bx.v0] - P, vertices_screen[bx.v1] - P);

	// F9 - Culling based on winding order
	const float triangleArea = Vector2::Cross(vertices_screen[bx.v1] - vertices_screen[bx.v0], vertices_screen[bx.v2] - vertices_screen[bx.v0]);

	if (m_CurrentCullMode == CullMode::Back && triangleArea < 0) return false;  // Back-facing
	if (m_CurrentCullMode == CullMode::Front && triangleArea > 0) return false; // Front-facing

	// Use absolute values for barycentric coordinates
	const float BARYCENTRIC_WEIGHT_0 = std::abs(w0) * 0.5f / bx.totalArea;
	const float BARYCENTRIC_WEIGHT_1 = std::abs(w1) * 0.5f / bx.totalArea;
	const float BARYCENTRIC_WEIGHT_2 = std::abs(w2) * 0.5f / bx.totalArea;

	// Inside-triangle test
	if (!(BARYCENTRIC_WEIGHT_0 >= 0 && BARYCENTRIC_WEIGHT_1 >= 0 && BARYCENTRIC_WEIGHT_2 >= 0 &&
		BARYCENTRIC_WEIGHT_0 <= 1 && BARYCENTRIC_WEIGHT_1 <= 1 && BARYCENTRIC_WEIGHT_2 <= 1 &&
		(BARYCENTRIC_WEIGHT_0 + BARYCENTRIC_WEIGHT_1 + BARYCENTRIC_WEIGHT_2 <= 1.001f)))
	{
		return false;
	}

	// Get interpolated depth and attributes
	const float VIEW_SPACE_DEPTH_W0{ vertices_ndc[bx.v0].w };
	const float VIEW_SPACE_DEPTH_W1{ vertices_ndc[bx.v1].w };
	const float VIEW_SPACE_DEPTH_W2{ vertices_ndc[bx.v2].w };

	const float NON_LINEAR_DEPTH_Z0{ vertices_ndc[bx.v0].z };
	const float NON_LINEAR_DEPTH_Z1{ vertices_ndc[bx.v1].z };
	const float NON_LINEAR_DEPTH_Z2{ vertices_ndc[bx.v2].z };

	const float DEPTH{ NON_LINEAR_DEPTH_Z0 * BARYCENTRIC_WEIGHT_0 + NON_LINEAR_DEPTH_Z1 * BARYCENTRIC_WEIGHT_1 + NON_LINEAR_DEPTH_Z2 * BARYCENTRIC_WEIGHT_2 };
	const int PIXEL_INDEX{ PX + PY * m_Width };

	// Depth test
	if (DEPTH >= m_pDepthBufferPixels[PIXEL_INDEX])
	{
		return false;
	}

	m_pDepthBufferPixels[PIXEL_INDEX] = DEPTH;

	// Perspective-correct interpolation
	const float INTERPOLATED_1_OVER_W{ (1.0f / VIEW_SPACE_DEPTH_W0) * BARYCENTRIC_WEIGHT_0 +
									   (1.0f / VIEW_SPACE_DEPTH_W1) * BARYCENTRIC_WEIGHT_1 +
									   (1.0f / VIEW_SPACE_DEPTH_W2) * BARYCENTRIC_WEIGHT_2 };
	const float W_INTERPOLATED{ 1.0f / INTERPOLATED_1_OVER_W };

	const Vector2 UV_INTERPOLATED{ (vertices_world[bx.v0].uv / VIEW_SPACE_DEPTH_W0 * BARYCENTRIC_WEIGHT_0 +
									vertices_world[bx.v1].uv / VIEW_SPACE_DEPTH_W1 * BARYCENTRIC_WEIGHT_1 +
									vertices_world[bx.v2].uv / VIEW_SPACE_DEPTH_W2 * BARYCENTRIC_WEIGHT_2) * W_INTERPOLATED };

	const Vector3 NORMAL_INTERPOLATED{ (transformed_normals[bx.v0] / VIEW_SPACE_DEPTH_W0 * BARYCENTRIC_WEIGHT_0 +
										transformed_normals[bx.v1] / VIEW_SPACE_DEPTH_W1 * BARYCENTRIC_WEIGHT_1 +
										transformed_normals[bx.v2] / VIEW_SPACE_DEPTH_W2 * BARYCENTRIC_WEIGHT_2) * W_INTERPOLATED };
	const Vector3 NORMAL_NORMALIZED{ NORMAL_INTERPOLATED.Normalized() };

	const Vector3 TANGENT_INTERPOLATED{ (transformed_tangents[bx.v0] / VIEW_SPACE_DEPTH_W0 * BARYCENTRIC_WEIGHT_0 +
									 transformed_tangents[bx.v1] / VIEW_SPACE_DEPTH_W1 * BARYCENTRIC_WEIGHT_1 +
									 transformed_tangents[bx.v2] / VIEW_SPACE_DEPTH_W2 * BARYCENTRIC_WEIGHT_2) * W_INTERPOLATED };
	const Vector3 TANGENT_NORMALIZED{ TANGENT_INTERPOLATED.Normalized() };

	const Vector3 WORLD_POSITION_INTERPOLATED{ (vertices_world[bx.v0].position / VIEW_SPACE_DEPTH_W0 * BARYCENTRIC_WEIGHT_0 +
												vertices_world[bx.v1].position / VIEW_SPACE_DEPTH_W1 * BARYCENTRIC_WEIGHT_1 +
												vertices_world[bx.v2].position / VIEW_SPACE_DEPTH_W2 * BARYCENTRIC_WEIGHT_2) * W_INTERPOLATED };

	ColorRGB finalColor;

	if (m_ShowDepthBuffer)
	{
		const float DEPTH_VISUALIZATION{ Remap(DEPTH, 0.985f, 1.0f) };
		finalColor = ColorRGB{ DEPTH_VISUALIZATION, DEPTH_VISUALIZATION, DEPTH_VISUALIZATION };
	}
	else
	{
		Vertex vertexOut;
		vertexOut.position = Vector4{ static_cast<float>(PX), static_cast<float>(PY), DEPTH, W_INTERPOLATED };
		vertexOut.uv = UV_INTERPOLATED;
		vertexOut.normal = NORMAL_NORMALIZED;
		vertexOut.tangent = TANGENT_NORMALIZED;
		vertexOut.viewDirection = WORLD_POSITION_INTERPOLATED;

		finalColor = PixelShading(vertexOut);
	}

	finalColor.MaxToOne();
	m_pBackBufferPixels[PIXEL_INDEX] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

	return true;
}

ColorRGB dae::Renderer::PixelShading(const Vertex& v)
{
	Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
	lightDirection = lightDirection.Normalized();

	// NORMALS
	Vector3 finalNormal{ v.normal };
	if (m_UseNormalMap)
	{
		finalNormal = CalculateFinalNormal(v);
	}

	//// Observed area
	const float OBSERVED_AREA{ std::max(0.f, Vector3::Dot(finalNormal, -lightDirection)) };
	const ColorRGB OBSERVED_AREA_COLOR{ OBSERVED_AREA, OBSERVED_AREA, OBSERVED_AREA };

	// Colors
	const ColorRGB LAMBERT_DIFFUSE{ CalculateDiffuseColor(v, OBSERVED_AREA) };
	const ColorRGB PHONG_SPECULAR{ CalculatePhongColor(v, lightDirection, finalNormal) };

	// return based on lighting mode
	const ColorRGB AMBIENT{ ColorRGB{ 0.03f, 0.03f, 0.03f } };
	switch (m_CurrentLightingMode)
	{
	case ShadingMode::ObservedArea:
		return OBSERVED_AREA_COLOR;

	case ShadingMode::Diffuse:
		return LAMBERT_DIFFUSE + AMBIENT;

	case ShadingMode::Specular:
		return PHONG_SPECULAR;

	case ShadingMode::Combined:
		return LAMBERT_DIFFUSE + PHONG_SPECULAR + AMBIENT;
	}

	return ColorRGB{ 0, 0, 0 };
}

Vector3 dae::Renderer::CalculateFinalNormal(const Vertex& v)
{
	const ColorRGB SAMPLED_NORMAL{ m_pNormalTextureSW->Sample(v.uv) };

	Vector3 tangentSpaceNormal{ 2.0f * SAMPLED_NORMAL.r - 1.0f,
								2.0f * SAMPLED_NORMAL.g - 1.0f,
								2.0f * SAMPLED_NORMAL.b - 1.0f };
	tangentSpaceNormal = tangentSpaceNormal.Normalized();

	// tangent space -> world space
	const Vector3 BINORMAL{ Vector3::Cross(v.normal, v.tangent).Normalized() };
	const Matrix TANGENT_SPACE_AXIS{ Matrix{ v.tangent, BINORMAL, v.normal, Vector3::Zero } };

	return TANGENT_SPACE_AXIS.TransformVector(tangentSpaceNormal).Normalized();
}

ColorRGB dae::Renderer::CalculateDiffuseColor(const Vertex& v, const float observedArea)
{
	const ColorRGB DIFFUSE_TEXTURE_COLOR{ m_pDiffuseTextureSW->Sample(v.uv) };
	const float KD{ 7.f };

	return ColorRGB{ (DIFFUSE_TEXTURE_COLOR * KD) / PI * observedArea };
}

ColorRGB dae::Renderer::CalculatePhongColor(const Vertex& v, const Vector3& lightDirection, const Vector3& finalNormal)
{
	//// Phong
	const Vector3 VIEW_DIRECTION{ (m_pCamera->GetPosition() - v.viewDirection).Normalized()};
	const Vector3 REFLECT{ lightDirection - 2.f * Vector3::Dot(lightDirection, finalNormal) * finalNormal };
	const ColorRGB SPECULAR_COLOR{ m_pSpecularTextureSW->Sample(v.uv) };
	const float GLOSSINESS_VALUE{ m_pGlossinessTextureSW->Sample(v.uv).r };
	const float SHININESS{ 25.f };
	const float PHONG_EXPONENT{ GLOSSINESS_VALUE * SHININESS };
	// Calculate specular reflection
	const float COS_ALPHA{ std::max(0.f, Vector3::Dot(REFLECT, VIEW_DIRECTION)) };
	const float PHONG_SPECULAR_REFLECTION{ SPECULAR_COLOR.r * std::pow(COS_ALPHA, PHONG_EXPONENT) };

	return ColorRGB{ PHONG_SPECULAR_REFLECTION, PHONG_SPECULAR_REFLECTION, PHONG_SPECULAR_REFLECTION };
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////// INOPUT FUNCTIONS

void Renderer::ToggleRenderMode()
{
	m_UseHardwareRenderer = !m_UseHardwareRenderer;
	std::cout << "Render Mode: " << (m_UseHardwareRenderer ? "HARDWARE (DirectX)" : "SOFTWARE (Rasterizer)") << std::endl;
}

void Renderer::ToggleRotation()
{
	m_IsRotating = !m_IsRotating;
	std::cout << "Rotation: " << (m_IsRotating ? "ON" : "OFF") << std::endl;
}

void Renderer::ToggleFireMesh()
{
	m_ShowFireMesh = !m_ShowFireMesh;
	std::cout << "FireFX Mesh: " << (m_ShowFireMesh ? "ON" : "OFF") << std::endl;
}

void Renderer::CycleSamplingState()
{
	SamplingMethod next{};
	switch (m_CurrentSamplingMethod)
	{
	case SamplingMethod::Point:
		next = SamplingMethod::Linear;
		std::cout << "Texture Sampling: LINEAR\n";
		break;
	case SamplingMethod::Linear:
		next = SamplingMethod::Anisotropic;
		std::cout << "Texture Sampling: ANISOTROPIC\n";
		break;
	case SamplingMethod::Anisotropic:
		next = SamplingMethod::Point;
		std::cout << "Texture Sampling: POINT\n";
		break;
	}

	m_CurrentSamplingMethod = next;

	if (m_pVehicleMesh)
	{
		BaseEffect* pEffect = m_pVehicleMesh->GetEffect();
		if (pEffect)
		{
			pEffect->SetFilteringMethod(next);
		}
	}

	if (m_pFireMesh)
	{
		BaseEffect* pEffect = m_pFireMesh->GetEffect();
		if (pEffect)
		{
			pEffect->SetFilteringMethod(next);
		}
	}
}

void dae::Renderer::CycleShadingMode()
{
	const size_t NUM_MODES{ 4 };
	int current{ static_cast<int>(m_CurrentLightingMode) };
	current = (current + 1) % NUM_MODES;
	m_CurrentLightingMode = static_cast<ShadingMode>(current);

	const char* MODE_NAMES[] = { "ObservedArea", "Diffuse", "Specular", "Combined" };
	std::cout << "Lighting Mode: " << MODE_NAMES[current] << std::endl;
}

void dae::Renderer::ToggleNormalMap()
{
	m_UseNormalMap = !m_UseNormalMap;
	std::cout << "Normal Map: " << (m_UseNormalMap ? "ON" : "OFF") << std::endl;
}

void dae::Renderer::ToggleDepthBuffer()
{
	m_ShowDepthBuffer = !m_ShowDepthBuffer;
	std::cout << "Depth Buffer Visualization: " << (m_ShowDepthBuffer ? "ON" : "OFF") << std::endl;
}

void Renderer::ToggleBoundingBoxVisualization()
{
	m_ShowBoundingBoxes = !m_ShowBoundingBoxes;
	std::cout << "BoundingBox Visualization: " << (m_ShowBoundingBoxes ? "ON" : "OFF") << std::endl;
}

void Renderer::DrawBoundingBox(const BoundingBox& box)
{
	const uint32_t WHITE = SDL_MapRGB(m_pBackBuffer->format, 255, 255, 255);

	for (int y = box.minY; y <= box.maxY; ++y)
	{
		for (int x = box.minX; x <= box.maxX; ++x)
		{
			if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
			{
				m_pBackBufferPixels[x + y * m_Width] = WHITE;
			}
		}
	}
}

void Renderer::CycleCullMode()
{
	switch (m_CurrentCullMode)
	{
	case CullMode::Back:
		m_CurrentCullMode = CullMode::Front;
		std::cout << "Cull Mode: FRONT" << std::endl;
		break;
	case CullMode::Front:
		m_CurrentCullMode = CullMode::None;
		std::cout << "Cull Mode: NONE" << std::endl;
		break;
	case CullMode::None:
		m_CurrentCullMode = CullMode::Back;
		std::cout << "Cull Mode: BACK" << std::endl;
		break;
	}
}

void Renderer::ToggleUniformClearColor()
{
	m_UseUniformClearColor = !m_UseUniformClearColor;
	std::cout << "Uniform Clear Color: " << (m_UseUniformClearColor ? "ON" : "OFF") << std::endl;
}