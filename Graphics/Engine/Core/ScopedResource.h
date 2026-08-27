#pragma once

#include <memory>
#include <utility>

namespace Engine
{
	template <typename T>
	using ScopedResource = std::unique_ptr<T>;

	template <typename T, typename... Args>
	ScopedResource<T> MakeScopedResource(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}
