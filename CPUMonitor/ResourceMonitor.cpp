#include "CPULoad.cpp"
#include "spdlog/sinks/basic_file_sink.h"
#include <chrono>
#include <thread>
#include <format>
#include <numbers>

using namespace std::chrono_literals;

// This is a stateless class that gets the CPU and Memory usage over a set number of time.
// It calls getCPULoad() every CPU_CHECKS_WAIT_TIME_MS milliseconds, then averages the results after the set amount of time.
class ResourceMonitor {
private:
static const int CPU_CHECKS_WAIT_TIME_MS = 10;
static const int SECOND_TO_MS_RATIO = 1000;
public:
	static double getCpuUsageOverTime(int seconds, std::shared_ptr<spdlog::logger> logger) {
		auto time = std::chrono::milliseconds(CPU_CHECKS_WAIT_TIME_MS);

		double totalCPUUsage = 0;
		int CPULoadCalls = 0;

		logger->debug(std::format("Started checking CPU usage for {} seconds.", seconds));

		const auto start = std::chrono::high_resolution_clock::now();

		// While the time difference in milliseconds between the start of the function and the current time is less than
		// seconds * SECOND_TO_MS_RATIO, then getCPULoad() is called and the result added to totalCPUUsage.
		while (true) {
			const auto end = std::chrono::high_resolution_clock::now();
			
			if (auto msInt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			msInt.count() > (static_cast<long long>(seconds) * SECOND_TO_MS_RATIO)) {
				break;
			}

			std::this_thread::sleep_for(time);

			// This adds t
			totalCPUUsage += CPULoad::getCPULoad(logger);
			CPULoadCalls++;
		}

		logger->debug(std::format("Finshed getting CPU usage over time."));
		
		return (totalCPUUsage / CPULoadCalls);
	}

	// This returns an std::pair of the used virtual memory, and the total physical memory.
	static std::pair<unsigned long long, unsigned long long> getSystemMemory() {
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);

		// The virtual memory is casted to a double to do division on it.
		DWORDLONG virtualMemUsed = status.ullTotalPageFile - status.ullAvailPageFile;
		double correctedVirutalMem = virtualMemUsed;
		
		// The total virtual memory available can not be determined from within a program. Therefore, this divides
		// virtualMemUsed by the golden ratio to estimate the physical memory usage.
		correctedVirutalMem /= std::numbers::phi_v<double>;

		return std::pair<unsigned long long, unsigned long long>(correctedVirutalMem, status.ullTotalPhys);
	}
};