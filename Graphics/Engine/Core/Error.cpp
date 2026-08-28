#include "Engine/Core/Error.h"

#include <sstream>
#include <utility>

namespace
{
	const char* ErrorCodeName(Engine::ErrorCode code) noexcept
	{
		switch (code)
		{
		case Engine::ErrorCode::InvalidArgument:
			return "InvalidArgument";
		case Engine::ErrorCode::InitializationFailed:
			return "InitializationFailed";
		case Engine::ErrorCode::PlatformFailure:
			return "PlatformFailure";
		case Engine::ErrorCode::UpdateFailed:
			return "UpdateFailed";
		case Engine::ErrorCode::RenderFailed:
			return "RenderFailed";
		case Engine::ErrorCode::IoFailure:
			return "IoFailure";
		case Engine::ErrorCode::InvariantViolation:
			return "InvariantViolation";
		case Engine::ErrorCode::UnexpectedFailure:
			return "UnexpectedFailure";
		default:
			return "Unknown";
		}
	}
}

namespace Engine
{
	Error::Error(
		ErrorCode code,
		std::string subsystem,
		std::string message,
		SourceLocation location)
		: m_code(code),
		  m_subsystem(std::move(subsystem)),
		  m_message(std::move(message)),
		  m_location(location)
	{
	}

	ErrorCode Error::Code() const noexcept
	{
		return m_code;
	}

	const std::string& Error::Subsystem() const noexcept
	{
		return m_subsystem;
	}

	const std::string& Error::Message() const noexcept
	{
		return m_message;
	}

	const SourceLocation& Error::Location() const noexcept
	{
		return m_location;
	}

	std::string Error::ToString() const
	{
		std::ostringstream stream;
		stream << '[' << ErrorCodeName(m_code) << ']';
		if (!m_subsystem.empty())
		{
			stream << '[' << m_subsystem << ']';
		}
		stream << ' ' << m_message;

		if (m_location.file != nullptr && m_location.file[0] != '\0')
		{
			stream << " (" << m_location.file << ':' << m_location.line;
			if (m_location.function != nullptr && m_location.function[0] != '\0')
			{
				stream << ", " << m_location.function;
			}
			stream << ')';
		}

		return stream.str();
	}
}
