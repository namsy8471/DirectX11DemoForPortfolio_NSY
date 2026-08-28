#pragma once

#include <cstdint>
#include <string>

namespace Engine
{
	enum class ErrorCode : std::uint32_t
	{
		InvalidArgument,
		InitializationFailed,
		PlatformFailure,
		UpdateFailed,
		RenderFailed,
		IoFailure,
		InvariantViolation,
		UnexpectedFailure
	};

	struct SourceLocation final
	{
		const char* file = "";
		const char* function = "";
		int line = 0;
	};

	class Error final
	{
	public:
		Error(
			ErrorCode code,
			std::string subsystem,
			std::string message,
			SourceLocation location = {});

		ErrorCode Code() const noexcept;
		const std::string& Subsystem() const noexcept;
		const std::string& Message() const noexcept;
		const SourceLocation& Location() const noexcept;
		std::string ToString() const;

	private:
		ErrorCode m_code;
		std::string m_subsystem;
		std::string m_message;
		SourceLocation m_location;
	};
}

#define ENGINE_SOURCE_LOCATION \
	::Engine::SourceLocation{ __FILE__, __func__, __LINE__ }

#define ENGINE_ERROR(code, subsystem, message) \
	::Engine::Error((code), (subsystem), (message), ENGINE_SOURCE_LOCATION)
