#pragma once
#include "loginterface.hpp"

namespace otc {

	class Log {

	public:

		template<LogLevel level, typename ...args>
		inline static void print(const args &...arg) {
			if(log)
				log->print<level>(arg...);
		}

		template<typename ...args>
		inline static void printDebug(const args &...arg) {
			print<LogLevel::DEBUG>(arg...);
		}

		template<typename ...args>
		inline static void printOptimize(const args &...arg) {
			print<LogLevel::OPTIMIZE>(arg...);
		}

		template<typename ...args>
		inline static void printWarn(const args &...arg) {
			print<LogLevel::WARN>(arg...);
		}

		template<typename ...args>
		inline static void printError(const args &...arg) {
			print<LogLevel::ERROR>(arg...);
		}

		template<typename ...args>
		inline static void printFatal(const args &...arg) {
			print<LogLevel::FATAL>(arg...);
		}

		template<typename T>
		inline static void setInterface() {
			static_assert(std::is_base_of<LogInterface, T>::value, "Cannot set interface, it requires a LogInterface class");
			destroyPointer(log);
			log = new T();
		}

	private:

		static LogInterface *log;

	};

}