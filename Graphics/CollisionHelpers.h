////////////////////////////////////////////////////////////////////////////////
// Filename: CollisionHelpers.h
// Collision detection utilities including AABB and Ray intersection
////////////////////////////////////////////////////////////////////////////////
#ifndef _COLLISIONHELPERS_H_
#define _COLLISIONHELPERS_H_

#include <directxmath.h>
#include <algorithm>
#include <cfloat>

using namespace DirectX;

namespace CollisionHelpers
{
	// AABB 구조체
	struct AABB
	{
		XMFLOAT3 min;
		XMFLOAT3 max;

		AABB()
		{
			min = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
			max = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		}

		AABB(const XMFLOAT3& minPoint, const XMFLOAT3& maxPoint) 
		{
			min = minPoint;
			max = maxPoint;
		}

		// AABB를 월드 변환 행렬로 변환
		AABB Transform(const XMMATRIX& worldMatrix) const
		{
			AABB transformed;

			// AABB의 8개 코너 계산
			XMFLOAT3 corners[8] = {
				XMFLOAT3(min.x, min.y, min.z),
				XMFLOAT3(max.x, min.y, min.z),
				XMFLOAT3(min.x, max.y, min.z),
				XMFLOAT3(max.x, max.y, min.z),
				XMFLOAT3(min.x, min.y, max.z),
				XMFLOAT3(max.x, min.y, max.z),
				XMFLOAT3(min.x, max.y, max.z),
				XMFLOAT3(max.x, max.y, max.z)
			};

			// 각 코너를 변환하고 새로운 AABB 계산
			for (int i = 0; i < 8; i++)
			{
				XMVECTOR corner = XMLoadFloat3(&corners[i]);
				corner = XMVector3TransformCoord(corner, worldMatrix);
				XMFLOAT3 transformedCorner;
				XMStoreFloat3(&transformedCorner, corner);

				transformed.min.x = (::std::min)(transformed.min.x, transformedCorner.x);
				transformed.min.y = (::std::min)(transformed.min.y, transformedCorner.y);
				transformed.min.z = (::std::min)(transformed.min.z, transformedCorner.z);

				transformed.max.x = (::std::max)(transformed.max.x, transformedCorner.x);
				transformed.max.y = (::std::max)(transformed.max.y, transformedCorner.y);
				transformed.max.z = (::std::max)(transformed.max.z, transformedCorner.z);
			}

			return transformed;
		}

		// AABB 중심점 계산
		XMFLOAT3 GetCenter() const
		{
			return XMFLOAT3(
				(min.x + max.x) * 0.5f,
				(min.y + max.y) * 0.5f,
				(min.z + max.z) * 0.5f
			);
		}

		// AABB 크기 계산
		XMFLOAT3 GetExtents() const
		{
			return XMFLOAT3(
				(max.x - min.x) * 0.5f,
				(max.y - min.y) * 0.5f,
				(max.z - min.z) * 0.5f
			);
		}
	};

	// 버텍스 배열로부터 AABB 계산
	template<typename VertexType>
	inline AABB CalculateAABB(const VertexType* vertices, unsigned int vertexCount)
	{
		AABB aabb;

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			aabb.min.x = (::std::min)(aabb.min.x, vertices[i].position.x);
			aabb.min.y = (::std::min)(aabb.min.y, vertices[i].position.y);
			aabb.min.z = (::std::min)(aabb.min.z, vertices[i].position.z);

			aabb.max.x = (::std::max)(aabb.max.x, vertices[i].position.x);
			aabb.max.y = (::std::max)(aabb.max.y, vertices[i].position.y);
			aabb.max.z = (::std::max)(aabb.max.z, vertices[i].position.z);
		}

		return aabb;
	}

	// Ray-AABB 교차 테스트 (Slab method)
	inline bool RayAABBIntersect(
		const XMFLOAT3& rayOrigin,
		const XMFLOAT3& rayDirection,
		const AABB& aabb,
		float* outDistance = nullptr)
	{
		float tMin = 0.0f;
		float tMax = FLT_MAX;

		// X축 slab 테스트
		if (abs(rayDirection.x) < 1e-8f)
		{
			// Ray가 X축과 평행
			if (rayOrigin.x < aabb.min.x || rayOrigin.x > aabb.max.x)
				return false;
		}
		else
		{
			float invD = 1.0f / rayDirection.x;
			float t1 = (aabb.min.x - rayOrigin.x) * invD;
			float t2 = (aabb.max.x - rayOrigin.x) * invD;

			if (t1 > t2) ::std::swap(t1, t2);

			tMin = (::std::max)(tMin, t1);
			tMax = (::std::min)(tMax, t2);

			if (tMin > tMax) return false;
		}

		// Y축 slab 테스트
		if (abs(rayDirection.y) < 1e-8f)
		{
			if (rayOrigin.y < aabb.min.y || rayOrigin.y > aabb.max.y)
				return false;
		}
		else
		{
			float invD = 1.0f / rayDirection.y;
			float t1 = (aabb.min.y - rayOrigin.y) * invD;
			float t2 = (aabb.max.y - rayOrigin.y) * invD;

			if (t1 > t2) ::std::swap(t1, t2);

			tMin = (::std::max)(tMin, t1);
			tMax = (::std::min)(tMax, t2);

			if (tMin > tMax) return false;
		}

		// Z축 slab 테스트
		if (abs(rayDirection.z) < 1e-8f)
		{
			if (rayOrigin.z < aabb.min.z || rayOrigin.z > aabb.max.z)
				return false;
		}
		else
		{
			float invD = 1.0f / rayDirection.z;
			float t1 = (aabb.min.z - rayOrigin.z) * invD;
			float t2 = (aabb.max.z - rayOrigin.z) * invD;

			if (t1 > t2) ::std::swap(t1, t2);

			tMin = (::std::max)(tMin, t1);
			tMax = (::std::min)(tMax, t2);

			if (tMin > tMax) return false;
		}

		// 교차 거리 반환
		if (outDistance)
		{
			*outDistance = (tMin > 0.0f) ? tMin : tMax;
		}

		return tMax >= 0.0f; // Ray가 AABB와 교차
	}

	// Ray-Sphere 교차 테스트 (개선된 버전)
	inline bool RaySphereIntersect(
		const XMFLOAT3& rayOrigin,
		const XMFLOAT3& rayDirection,
		const XMFLOAT3& sphereCenter,
		float sphereRadius,
		float* outDistance = nullptr)
	{
		XMVECTOR origin = XMLoadFloat3(&rayOrigin);
		XMVECTOR direction = XMLoadFloat3(&rayDirection);
		XMVECTOR center = XMLoadFloat3(&sphereCenter);

		XMVECTOR oc = origin - center;

		float a = XMVectorGetX(XMVector3Dot(direction, direction));
		float b = 2.0f * XMVectorGetX(XMVector3Dot(oc, direction));
		float c = XMVectorGetX(XMVector3Dot(oc, oc)) - sphereRadius * sphereRadius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
		{
			return false;
		}

		if (outDistance)
		{
			float t = (-b - sqrtf(discriminant)) / (2.0f * a);
			if (t < 0.0f)
				t = (-b + sqrtf(discriminant)) / (2.0f * a);
			*outDistance = t;
		}

		return true;
	}
}

#endif
