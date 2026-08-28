#pragma once

#include "Engine/Core/Error.h"

#include <cassert>
#include <optional>
#include <utility>
#include <variant>

namespace Engine
{
	template <typename T>
	class [[nodiscard]] Result final
	{
	public:
		static Result Success(T value)
		{
			return Result(std::move(value));
		}

		static Result Failure(Error error)
		{
			return Result(std::move(error));
		}

		explicit operator bool() const noexcept
		{
			return std::holds_alternative<T>(m_value);
		}

		T& Value() &
		{
			assert(static_cast<bool>(*this));
			return std::get<T>(m_value);
		}

		const T& Value() const&
		{
			assert(static_cast<bool>(*this));
			return std::get<T>(m_value);
		}

		T&& Value() &&
		{
			assert(static_cast<bool>(*this));
			return std::get<T>(std::move(m_value));
		}

		const Error& GetError() const noexcept
		{
			assert(!static_cast<bool>(*this));
			return std::get<Error>(m_value);
		}

	private:
		explicit Result(T value)
			: m_value(std::move(value))
		{
		}

		explicit Result(Error error)
			: m_value(std::move(error))
		{
		}

		std::variant<T, Error> m_value;
	};

	template <>
	class [[nodiscard]] Result<void> final
	{
	public:
		static Result Success() noexcept
		{
			return Result();
		}

		static Result Failure(Error error)
		{
			return Result(std::move(error));
		}

		explicit operator bool() const noexcept
		{
			return !m_error.has_value();
		}

		const Error& GetError() const noexcept
		{
			assert(m_error.has_value());
			return *m_error;
		}

	private:
		Result() noexcept = default;

		explicit Result(Error error)
			: m_error(std::move(error))
		{
		}

		std::optional<Error> m_error;
	};
}
