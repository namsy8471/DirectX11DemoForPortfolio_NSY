#pragma once

#include <type_traits>
#include <utility>

namespace Engine
{
	template <typename Function>
	class ScopeExit final
	{
	public:
		explicit ScopeExit(Function function) noexcept(std::is_nothrow_move_constructible_v<Function>)
			: m_function(std::move(function))
		{
		}

		~ScopeExit() noexcept
		{
			if (m_active)
			{
				m_function();
			}
		}

		ScopeExit(const ScopeExit&) = delete;
		ScopeExit& operator=(const ScopeExit&) = delete;
		ScopeExit(ScopeExit&&) = delete;
		ScopeExit& operator=(ScopeExit&&) = delete;

		void Release() noexcept
		{
			m_active = false;
		}

	private:
		Function m_function;
		bool m_active = true;
	};

	template <typename Function>
	ScopeExit<std::decay_t<Function>> MakeScopeExit(Function&& function)
	{
		return ScopeExit<std::decay_t<Function>>(std::forward<Function>(function));
	}
}
