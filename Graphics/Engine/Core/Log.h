#pragma once

#include "Engine/Core/Error.h"
#include "Engine/Core/Result.h"

#include <filesystem>
#include <string>

namespace Engine
{
	enum class LogLevel
	{
		Trace,
		Info,
		Warning,
		Error,
		Fatal
	};

	struct LogConfig final
	{
		std::filesystem::path filePath;
		LogLevel minimumLevel = LogLevel::Info;
		bool writeToStandardError = true;
	};

	class Log final
	{
	public:
		static Result<void> Initialize(const LogConfig& config);
		static void Shutdown() noexcept;

		static void Write(
			LogLevel level,
			const std::string& category,
			const std::string& message) noexcept;
		static void Write(LogLevel level, const Error& error) noexcept;
	};
}
