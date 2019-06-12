#pragma once
#include "types/string.hpp"

namespace otc {

	enum class LogLevel {
		DEBUG,
		OPTIMIZE,
		WARN,
		ERROR,
		FATAL
	};

	struct LogInterface {

		LogInterface() = default;
		virtual ~LogInterface() = default;

		LogInterface(const LogInterface &) = delete;
		LogInterface(LogInterface &&) = delete;
		LogInterface &operator=(const LogInterface &) = delete;
		LogInterface &operator=(LogInterface &&) = delete;
		
		virtual void printString(const LogLevel level, const String16 &str) const = 0;

		template<const LogLevel level, typename ...args>
		inline void print(const args &...arg) const {
			String16 value = String16::concat(arg...);
			printString(level, value);
		}

	};

	struct ConsoleLogInterface : public LogInterface {
		void printString(const LogLevel, const String16 &) const final override {}
	};

	struct FileLogInterface : public LogInterface {

		FileLogInterface();
		~FileLogInterface();

		void printString(const LogLevel level, const String16 &str) const final override;
	};

}