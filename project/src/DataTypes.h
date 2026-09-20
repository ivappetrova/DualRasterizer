#pragma once
#include "Math.h"
#include <vector>

using namespace dae;

struct Vertex
{
	Vector3 position{};
	ColorRGB color{ colors::White };
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector3 viewDirection{};
};

struct Vertex_In
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
};

struct BoundingBox
{
	int minX, maxX;
	int minY, maxY;
	int v0, v1, v2;
	float totalArea;
	size_t meshIndex;
};

enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};

struct Mesh_SOFTWARE
{
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

	std::vector<Vertex> vertices_out{};
	Matrix worldMatrix{};
};

enum class SamplingMethod
{
	Point,
	Linear,
	Anisotropic
};

enum class CullMode
{
	Back,
	Front,
	None
};

enum class EffectType
{
	Standard,
	Flat
};

enum class ShadingMode
{
	ObservedArea,
	Diffuse,
	Specular,
	Combined
};