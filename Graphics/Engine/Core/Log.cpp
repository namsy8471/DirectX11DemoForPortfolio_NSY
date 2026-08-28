#include "Engine/Core/Log.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <system_error>
#include <thread>

namespace
{
	struct LogState final
	{
		std::mutex mutex;
		std::ofstream file;
		Engine::LogLevel minimumLevel = Engine::LogLevel::Info;
		bool writeToStandardError = true;
	};

	LogState& State() noexcept
	{
		static LogState state;
		return state;
	}

	const char* LevelName(Engine::LogLevel level) noexcept
	{
		switch (level)
		{
		case Engine::LogLevel::Trace:
			return "TRACE";
		case Engine::LogLevel::Info:
			return "INFO";
		case Engine::LogLevel::Warning:
			return "WARN";
		case Engine::LogLevel::Error:
			return "ERROR";
		case Engine::LogLevel::Fatal:
			return "FATAL";
		default:
			return "UNKNOWN";
		}
	}

	std::string FormatRecord(
		Engine::LogLevel level,
		const std::string& category,
		const std::string& message)
	{
		const auto now = std::chrono::system_clock::now();
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()).count();

		std::ostringstream stream;
		stream << milliseconds
			<< " [" << LevelName(level) << ']'
			<< " [thread=" << std::this_thread::get_id() << ']'
			<< " [" << category << "] "
			<< message;
		return stream.str();
	}
}

namespace Engine
{
	Result<void> Log::Initialize(const LogConfig& config)
	{
		LogState& state = State();
		std::lock_guard<std::mutex> lock(state.mutex);

		state.file.close();
		state.file.clear();
		state.minimumLevel = config.minimumLevel;
		state.writeToStandardError = config.writeToStandardError;

		if (config.filePath.empty())
		{
			return Result<void>::Success();
		}

		std::error_code filesystemError;
		const std::filesystem::path parent = config.filePath.parent_path();
		if (!parent.empty())
		{
			std::filesystem::create_directories(parent, filesystemError);
			if (filesystemError)
			{
				return Result<void>::Failure(ENGINE_ERROR(
					ErrorCode::IoFailure,
					"Log",
					"Could not create the log directory: " + filesystemError.message()));
			}
		}

		state.file.open(config.filePath, std::ios::out | std::ios::app);
		if (!state.file.is_open())
		{
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::IoFailure,
				"Log",
				"Could not open the log file: " + config.filePath.string()));
		}

		return Result<void>::Success();
	}

	void Log::Shutdown() noexcept
	{
		LogState& state = State();
		std::lock_guard<std::mutex> lock(state.mutex);
		if (state.file.is_open())
		{
			state.file.flush();
			state.file.close();
		}
	}

	void Log::Write(
		LogLevel level,
		const std::string& category,
		const std::string& message) noexcept
	{
		try
		{
			LogState& state = State();
			std::lock_guard<std::mutex> lock(state.mutex);
			if (static_cast<int>(level) < static_cast<int>(state.minimumLevel))
			{
				return;
			}

			const std::string record = FormatRecord(level, category, message);
			if (state.writeToStandardError)
			{
				std::clog << record << '\n';
			}
			if (state.file.is_open())
			{
				state.file << record << '\n';
				state.file.flush();
			}
		}
		catch (...)
		{
			// Logging must never destabilize the runtime it is observing.
		}
	}

	void Log::Write(LogLevel level, const Error& error) noexcept
	{
		Write(level, error.Subsystem(), error.ToString());
	}
}
