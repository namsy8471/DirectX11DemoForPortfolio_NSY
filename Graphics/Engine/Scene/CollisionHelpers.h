#pragma once

#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>

namespace CollisionHelpers
{
	struct AABB
	{
		DirectX::XMFLOAT3 min{FLT_MAX, FLT_MAX, FLT_MAX};
		DirectX::XMFLOAT3 max{-FLT_MAX, -FLT_MAX, -FLT_MAX};

		AABB() noexcept = default;
		AABB(const DirectX::XMFLOAT3& minimum, const DirectX::XMFLOAT3& maximum) noexcept
			: min(minimum), max(maximum)
		{
		}

		[[nodiscard]] bool IsValid() const noexcept
		{
			return min.x <= max.x && min.y <= max.y && min.z <= max.z;
		}

		[[nodiscard]] AABB Transform(const DirectX::XMMATRIX& worldMatrix) const noexcept
		{
			if (!IsValid())
			{
				return {};
			}

			const std::array<DirectX::XMFLOAT3, 8> corners{
				DirectX::XMFLOAT3{min.x, min.y, min.z},
				DirectX::XMFLOAT3{max.x, min.y, min.z},
				DirectX::XMFLOAT3{min.x, max.y, min.z},
				DirectX::XMFLOAT3{max.x, max.y, min.z},
				DirectX::XMFLOAT3{min.x, min.y, max.z},
				DirectX::XMFLOAT3{max.x, min.y, max.z},
				DirectX::XMFLOAT3{min.x, max.y, max.z},
				DirectX::XMFLOAT3{max.x, max.y, max.z}};

			AABB transformed;
			for (const auto& corner : corners)
			{
				DirectX::XMFLOAT3 value;
				DirectX::XMStoreFloat3(
					&value,
					DirectX::XMVector3TransformCoord(
						DirectX::XMLoadFloat3(&corner),
						worldMatrix));
				transformed.min.x = (std::min)(transformed.min.x, value.x);
				transformed.min.y = (std::min)(transformed.min.y, value.y);
				transformed.min.z = (std::min)(transformed.min.z, value.z);
				transformed.max.x = (std::max)(transformed.max.x, value.x);
				transformed.max.y = (std::max)(transformed.max.y, value.y);
				transformed.max.z = (std::max)(transformed.max.z, value.z);
			}
			return transformed;
		}

		[[nodiscard]] DirectX::XMFLOAT3 GetCenter() const noexcept
		{
			return IsValid()
				? DirectX::XMFLOAT3{
					(min.x + max.x) * 0.5f,
					(min.y + max.y) * 0.5f,
					(min.z + max.z) * 0.5f}
				: DirectX::XMFLOAT3{};
		}

		[[nodiscard]] DirectX::XMFLOAT3 GetExtents() const noexcept
		{
			return IsValid()
				? DirectX::XMFLOAT3{
					(max.x - min.x) * 0.5f,
					(max.y - min.y) * 0.5f,
					(max.z - min.z) * 0.5f}
				: DirectX::XMFLOAT3{};
		}
	};

	template<typename VertexType>
	[[nodiscard]] inline AABB CalculateAABB(
		const VertexType* vertices,
		unsigned int vertexCount) noexcept
	{
		AABB bounds;
		if (vertices == nullptr)
		{
			return bounds;
		}

		for (unsigned int index = 0; index < vertexCount; ++index)
		{
			bounds.min.x = (std::min)(bounds.min.x, vertices[index].position.x);
			bounds.min.y = (std::min)(bounds.min.y, vertices[index].position.y);
			bounds.min.z = (std::min)(bounds.min.z, vertices[index].position.z);
			bounds.max.x = (std::max)(bounds.max.x, vertices[index].position.x);
			bounds.max.y = (std::max)(bounds.max.y, vertices[index].position.y);
			bounds.max.z = (std::max)(bounds.max.z, vertices[index].position.z);
		}
		return bounds;
	}

	[[nodiscard]] inline bool RayAABBIntersect(
		const DirectX::XMFLOAT3& rayOrigin,
		const DirectX::XMFLOAT3& rayDirection,
		const AABB& bounds,
		float* outDistance = nullptr) noexcept
	{
		if (!bounds.IsValid())
		{
			return false;
		}

		float minimumDistance = 0.0f;
		float maximumDistance = FLT_MAX;
		const auto testAxis = [&minimumDistance, &maximumDistance](
			float origin,
			float direction,
			float minimum,
			float maximum) noexcept
		{
			constexpr float epsilon = 1.0e-8f;
			if (std::abs(direction) < epsilon)
			{
				return origin >= minimum && origin <= maximum;
			}

			const float inverseDirection = 1.0f / direction;
			float nearDistance = (minimum - origin) * inverseDirection;
			float farDistance = (maximum - origin) * inverseDirection;
			if (nearDistance > farDistance)
			{
				std::swap(nearDistance, farDistance);
			}
			minimumDistance = (std::max)(minimumDistance, nearDistance);
			maximumDistance = (std::min)(maximumDistance, farDistance);
			return minimumDistance <= maximumDistance;
		};

		if (!testAxis(rayOrigin.x, rayDirection.x, bounds.min.x, bounds.max.x) ||
			!testAxis(rayOrigin.y, rayDirection.y, bounds.min.y, bounds.max.y) ||
			!testAxis(rayOrigin.z, rayDirection.z, bounds.min.z, bounds.max.z) ||
			maximumDistance < 0.0f)
		{
			return false;
		}

		if (outDistance != nullptr)
		{
			*outDistance = minimumDistance >= 0.0f
				? minimumDistance
				: maximumDistance;
		}
		return true;
	}

	[[nodiscard]] inline bool RaySphereIntersect(
		const DirectX::XMFLOAT3& rayOrigin,
		const DirectX::XMFLOAT3& rayDirection,
		const DirectX::XMFLOAT3& sphereCenter,
		float sphereRadius,
		float* outDistance = nullptr) noexcept
	{
		if (sphereRadius < 0.0f)
		{
			return false;
		}

		const float offsetX = rayOrigin.x - sphereCenter.x;
		const float offsetY = rayOrigin.y - sphereCenter.y;
		const float offsetZ = rayOrigin.z - sphereCenter.z;
		const float a = rayDirection.x * rayDirection.x +
			rayDirection.y * rayDirection.y +
			rayDirection.z * rayDirection.z;
		if (a <= 1.0e-12f)
		{
			return false;
		}

		const float b = 2.0f * (
			offsetX * rayDirection.x +
			offsetY * rayDirection.y +
			offsetZ * rayDirection.z);
		const float c = offsetX * offsetX + offsetY * offsetY +
			offsetZ * offsetZ - sphereRadius * sphereRadius;
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			return false;
		}

		const float root = std::sqrt(discriminant);
		const float nearDistance = (-b - root) / (2.0f * a);
		const float farDistance = (-b + root) / (2.0f * a);
		if (farDistance < 0.0f)
		{
			return false;
		}
		if (outDistance != nullptr)
		{
			*outDistance = nearDistance >= 0.0f ? nearDistance : farDistance;
		}
		return true;
	}
}
