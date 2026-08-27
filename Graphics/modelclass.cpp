#include "modelclass.h"

#include "DDSTextureLoader.h"

#include <algorithm>
#include <charconv>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace
{
	struct ObjVertexKey
	{
		int position = -1;
		int texture = -1;
		int normal = -1;

		bool operator==(const ObjVertexKey& other) const noexcept
		{
			return position == other.position &&
				texture == other.texture &&
				normal == other.normal;
		}
	};

	struct ObjVertexKeyHash
	{
		std::size_t operator()(const ObjVertexKey& key) const noexcept
		{
			std::size_t seed = static_cast<std::size_t>(key.position + 1);
			seed ^= static_cast<std::size_t>(key.texture + 1) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
			seed ^= static_cast<std::size_t>(key.normal + 1) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
			return seed;
		}
	};

	bool ParseInteger(std::string_view text, int& value) noexcept
	{
		if (text.empty())
		{
			return false;
		}

		const char* first = text.data();
		const char* last = first + text.size();
		const auto result = std::from_chars(first, last, value);
		return result.ec == std::errc() && result.ptr == last;
	}

	bool ResolveObjIndex(int sourceIndex, std::size_t elementCount, int& resolvedIndex) noexcept
	{
		if (sourceIndex == 0 || elementCount > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
		{
			return false;
		}

		const int count = static_cast<int>(elementCount);
		const int candidate = sourceIndex > 0 ? sourceIndex - 1 : count + sourceIndex;
		if (candidate < 0 || candidate >= count)
		{
			return false;
		}

		resolvedIndex = candidate;
		return true;
	}

	bool ParseObjVertexKey(
		std::string_view token,
		std::size_t positionCount,
		std::size_t textureCount,
		std::size_t normalCount,
		ObjVertexKey& key) noexcept
	{
		const std::size_t firstSlash = token.find('/');
		const std::size_t secondSlash = firstSlash == std::string_view::npos
			? std::string_view::npos
			: token.find('/', firstSlash + 1u);

		const std::string_view positionPart = token.substr(0u, firstSlash);
		const std::string_view texturePart = firstSlash == std::string_view::npos
			? std::string_view{}
			: token.substr(
				firstSlash + 1u,
				secondSlash == std::string_view::npos
					? std::string_view::npos
					: secondSlash - firstSlash - 1u);
		const std::string_view normalPart = secondSlash == std::string_view::npos
			? std::string_view{}
			: token.substr(secondSlash + 1u);

		int sourceIndex = 0;
		if (!ParseInteger(positionPart, sourceIndex) ||
			!ResolveObjIndex(sourceIndex, positionCount, key.position))
		{
			return false;
		}

		if (!texturePart.empty())
		{
			if (!ParseInteger(texturePart, sourceIndex) ||
				!ResolveObjIndex(sourceIndex, textureCount, key.texture))
			{
				return false;
			}
		}

		if (!normalPart.empty())
		{
			if (!ParseInteger(normalPart, sourceIndex) ||
				!ResolveObjIndex(sourceIndex, normalCount, key.normal))
			{
				return false;
			}
		}

		return true;
	}

	DirectX::XMFLOAT3 NormalizedOrDefault(const DirectX::XMFLOAT3& normal) noexcept
	{
		using namespace DirectX;

		const XMVECTOR value = XMLoadFloat3(&normal);
		if (XMVectorGetX(XMVector3LengthSq(value)) <= 1.0e-12f)
		{
			return XMFLOAT3(0.0f, 1.0f, 0.0f);
		}

		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(value));
		return result;
	}
}

bool ModelClass::Initialize(
	HWND window,
	ID3D11Device* device,
	const wchar_t* modelFilename,
	const wchar_t* textureFilename)
{
	(void)window;
	Shutdown();

	if (device == nullptr)
	{
		return false;
	}

	if (!LoadObj(modelFilename))
	{
		return false;
	}

	if (!InitializeBuffers(device))
	{
		Shutdown();
		return false;
	}

	if (!LoadTexture(device, textureFilename))
	{
		Shutdown();
		return false;
	}

	return true;
}

void ModelClass::Shutdown() noexcept
{
	m_texture.Reset();
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indices.clear();
	m_vertices.clear();
	m_localAABB = CollisionHelpers::AABB();
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext) const noexcept
{
	if (deviceContext == nullptr || !m_vertexBuffer || !m_indexBuffer)
	{
		return;
	}

	ID3D11Buffer* vertexBuffer = m_vertexBuffer.Get();
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0u;
	deviceContext->IASetVertexBuffers(0u, 1u, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int ModelClass::GetIndexCount() const noexcept
{
	return static_cast<int>(m_indices.size());
}

int ModelClass::GetVertexCount() const noexcept
{
	return static_cast<int>(m_vertices.size());
}

int ModelClass::GetPolygonCount() const noexcept
{
	return static_cast<int>(m_indices.size() / 3u);
}

int ModelClass::CountPolygons() const noexcept
{
	return GetPolygonCount();
}

int ModelClass::CountMeshes() const noexcept
{
	return m_indices.empty() ? 0 : 1;
}

ID3D11ShaderResourceView* ModelClass::GetTexture() const noexcept
{
	return m_texture.Get();
}

CollisionHelpers::AABB ModelClass::GetLocalAABB() const noexcept
{
	return m_localAABB;
}

bool ModelClass::LoadObj(const wchar_t* filename)
{
	using namespace DirectX;

	if (filename == nullptr || *filename == L'\0')
	{
		return false;
	}

	const std::filesystem::path path(filename);
	std::wstring extension = path.extension().wstring();
	std::transform(extension.begin(), extension.end(), extension.begin(), [](wchar_t value)
	{
		return static_cast<wchar_t>(std::towlower(value));
	});
	if (extension != L".obj")
	{
		return false;
	}

	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> textureCoordinates;
	std::vector<XMFLOAT3> normals;
	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<bool> needsGeneratedNormal;
	std::unordered_map<ObjVertexKey, std::uint32_t, ObjVertexKeyHash> vertexLookup;

	std::string line;
	while (std::getline(input, line))
	{
		std::istringstream lineStream(line);
		std::string command;
		lineStream >> command;
		if (command.empty() || command[0] == '#')
		{
			continue;
		}

		if (command == "v")
		{
			XMFLOAT3 position;
			if (lineStream >> position.x >> position.y >> position.z)
			{
				// Match the legacy Assimp aiProcess_ConvertToLeftHanded import.
				position.z = -position.z;
				positions.push_back(position);
			}
			continue;
		}

		if (command == "vt")
		{
			XMFLOAT2 texture(0.0f, 0.0f);
			if (lineStream >> texture.x)
			{
				lineStream >> texture.y;
				texture.y = 1.0f - texture.y;
				textureCoordinates.push_back(texture);
			}
			continue;
		}

		if (command == "vn")
		{
			XMFLOAT3 normal;
			if (lineStream >> normal.x >> normal.y >> normal.z)
			{
				normal.z = -normal.z;
				normals.push_back(NormalizedOrDefault(normal));
			}
			continue;
		}

		if (command != "f")
		{
			continue;
		}

		std::vector<ObjVertexKey> face;
		std::string token;
		bool validFace = true;
		while (lineStream >> token)
		{
			if (!token.empty() && token[0] == '#')
			{
				break;
			}

			ObjVertexKey key;
			if (!ParseObjVertexKey(
				token,
				positions.size(),
				textureCoordinates.size(),
				normals.size(),
				key))
			{
				validFace = false;
				break;
			}
			face.push_back(key);
		}

		if (!validFace || face.size() < 3u)
		{
			continue;
		}

		auto getVertexIndex = [&](const ObjVertexKey& key, std::uint32_t& index) -> bool
		{
			const auto found = vertexLookup.find(key);
			if (found != vertexLookup.end())
			{
				index = found->second;
				return true;
			}

			if (vertices.size() >= static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
			{
				return false;
			}

			Vertex vertex{};
			vertex.position = positions[static_cast<std::size_t>(key.position)];
			vertex.texture = key.texture >= 0
				? textureCoordinates[static_cast<std::size_t>(key.texture)]
				: XMFLOAT2(0.0f, 0.0f);
			vertex.normal = key.normal >= 0
				? normals[static_cast<std::size_t>(key.normal)]
				: XMFLOAT3(0.0f, 0.0f, 0.0f);

			index = static_cast<std::uint32_t>(vertices.size());
			vertexLookup.emplace(key, index);
			vertices.push_back(vertex);
			needsGeneratedNormal.push_back(key.normal < 0);
			return true;
		};

		for (std::size_t corner = 1u; corner + 1u < face.size(); ++corner)
		{
			std::uint32_t triangle[3]{};
			if (!getVertexIndex(face[0], triangle[0]) ||
				!getVertexIndex(face[corner], triangle[1]) ||
				!getVertexIndex(face[corner + 1u], triangle[2]))
			{
				return false;
			}

			// aiProcess_ConvertToLeftHanded also reverses face winding.
			indices.push_back(triangle[0]);
			indices.push_back(triangle[2]);
			indices.push_back(triangle[1]);
		}
	}

	if (vertices.empty() || indices.empty() || indices.size() % 3u != 0u ||
		vertices.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()) ||
		indices.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
	{
		return false;
	}

	m_vertices = std::move(vertices);
	m_indices = std::move(indices);
	GenerateMissingNormals(needsGeneratedNormal);
	RecalculateLocalAABB();
	return true;
}

void ModelClass::GenerateMissingNormals(const std::vector<bool>& needsGeneratedNormal)
{
	using namespace DirectX;

	if (needsGeneratedNormal.size() != m_vertices.size())
	{
		return;
	}

	std::vector<XMFLOAT3> accumulated(m_vertices.size(), XMFLOAT3(0.0f, 0.0f, 0.0f));
	for (std::size_t i = 0u; i + 2u < m_indices.size(); i += 3u)
	{
		const std::uint32_t index0 = m_indices[i];
		const std::uint32_t index1 = m_indices[i + 1u];
		const std::uint32_t index2 = m_indices[i + 2u];

		const XMVECTOR position0 = XMLoadFloat3(&m_vertices[index0].position);
		const XMVECTOR position1 = XMLoadFloat3(&m_vertices[index1].position);
		const XMVECTOR position2 = XMLoadFloat3(&m_vertices[index2].position);
		const XMVECTOR faceNormal = XMVector3Cross(position1 - position0, position2 - position0);

		XMFLOAT3 value;
		XMStoreFloat3(&value, faceNormal);
		for (const std::uint32_t index : {index0, index1, index2})
		{
			if (needsGeneratedNormal[index])
			{
				accumulated[index].x += value.x;
				accumulated[index].y += value.y;
				accumulated[index].z += value.z;
			}
		}
	}

	for (std::size_t i = 0u; i < m_vertices.size(); ++i)
	{
		if (needsGeneratedNormal[i])
		{
			m_vertices[i].normal = NormalizedOrDefault(accumulated[i]);
		}
	}
}

void ModelClass::RecalculateLocalAABB() noexcept
{
	m_localAABB = m_vertices.empty()
		? CollisionHelpers::AABB()
		: CollisionHelpers::CalculateAABB(m_vertices.data(), static_cast<unsigned int>(m_vertices.size()));
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	if (device == nullptr || m_vertices.empty() || m_indices.empty())
	{
		return false;
	}

	const std::size_t vertexByteCount = m_vertices.size() * sizeof(Vertex);
	const std::size_t indexByteCount = m_indices.size() * sizeof(std::uint32_t);
	if (vertexByteCount > static_cast<std::size_t>((std::numeric_limits<UINT>::max)()) ||
		indexByteCount > static_cast<std::size_t>((std::numeric_limits<UINT>::max)()))
	{
		return false;
	}

	D3D11_BUFFER_DESC vertexDescription{};
	vertexDescription.Usage = D3D11_USAGE_DEFAULT;
	vertexDescription.ByteWidth = static_cast<UINT>(vertexByteCount);
	vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = m_vertices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if (FAILED(device->CreateBuffer(&vertexDescription, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	D3D11_BUFFER_DESC indexDescription{};
	indexDescription.Usage = D3D11_USAGE_DEFAULT;
	indexDescription.ByteWidth = static_cast<UINT>(indexByteCount);
	indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = m_indices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	if (FAILED(device->CreateBuffer(&indexDescription, &indexData, indexBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_indexBuffer = std::move(indexBuffer);
	return true;
}

bool ModelClass::LoadTexture(ID3D11Device* device, const wchar_t* filename) noexcept
{
	m_texture.Reset();
	if (device == nullptr || filename == nullptr || *filename == L'\0')
	{
		return false;
	}

	return SUCCEEDED(DirectX::CreateDDSTextureFromFile(
		device,
		filename,
		nullptr,
		m_texture.GetAddressOf()));
}
