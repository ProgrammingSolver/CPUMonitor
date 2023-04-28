#include <Windows.h>
#include <utility>
#include <stdexcept>
#include "spdlog/sinks/basic_file_sink.h"

// This is a stateless class that determines the CPU load at any given moment. This should be called repeatedly over time,
// then averaged, in order to get a accurate result.
class CPULoad {
private:
	static const int DECIMAL_TO_NUMBER_RATIO = 100;

public:
	// This calculates the CPU Load at any given moment, given idleTicks, and totalTicks.
	static double calculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks) {
		static unsigned long long previousTotalTicks = 0;
		static unsigned long long previousIdleTicks = 0;

		unsigned long long totalTicksSinceLastTime = totalTicks - previousTotalTicks;
		unsigned long long idleTicksSinceLastTime = idleTicks - previousIdleTicks;

		// There is precision loss here by casting a unsigned long long to a double, however calculating the
		// CPU usage over time is inherently inprecise, so that loses far more precision that is lost here.
		double ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((double)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

		previousTotalTicks = totalTicks;
		previousIdleTicks = idleTicks;
		return ret;
	}

	// This converts the ft, a given FILETIME, from 16 to 64 bits, before giving it to CalculateCPULoad().
	static unsigned long long fileTimeToInt64(const FILETIME& ft) { 
		return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
	}

	// idleTime is the number of CPU ticks spent idling at any given moment, and kernel and user time both represent ticks
	// where the CPU is performing activity, hoever kernel and user time happen seperately, so they are added together.
	static double getCPULoad(std::shared_ptr<spdlog::logger> logger) {
		// idleTime, kernelTime, and userTime are impossible to declare inside of GetSystemTimes(), so they are declared here.
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;
	
		// This gets a reference to idleTime, kernelTime, and userTime from the OS, then converts them from 16 bits to
		// 64 bits, and gives them as parameters to CaclulateCpuLoad().
		if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
			return (calculateCPULoad(fileTimeToInt64(idleTime),
			fileTimeToInt64(kernelTime) + fileTimeToInt64(userTime)) * DECIMAL_TO_NUMBER_RATIO);
		}

		logger->error("OS failed to return system times.");
		throw std::domain_error("OS failed to return system times.");
	}
};