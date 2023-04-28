#include <iostream>
#include <string>
#include <memory>
#include <format>
#include <utility>
#include <stdexcept>
#include "spdlog/sinks/basic_file_sink.h"
#include "ResourceMonitor.cpp"

// This gets the CPU and total memory usage of the OS.
void getResourceUsage(int seconds, std::shared_ptr<spdlog::logger> logger) {
	const double BILLION = 1000000000.0;

	try {
		// This gets the cpu and memory usage.
		double CPUUsage = ResourceMonitor::getCpuUsageOverTime(seconds, logger);
		auto memUsage = ResourceMonitor::getSystemMemory();
		
		// This turns the memory usage into doubles.
		double decimalUsedMem = std::get<0>(memUsage);
		double decimalTotalMem = std::get<1>(memUsage);
		
		// This divides the memory by a billion to make it easier to represent as a decimal.
		decimalUsedMem /= BILLION;
		decimalTotalMem /= BILLION;

		// This prints out the memory as a float with 2 decimal points of precision.
		std::printf("CPU Usage: %.2f%%\tMemory Usage: %.2f GB / %.2f GB\n", CPUUsage, decimalUsedMem, decimalTotalMem);
		logger->debug(std::format("CPU usage averaged {} over {} seconds", CPUUsage, seconds));
	}

	// If an error occurs with CPULoad, then std::domain_error e is thrown.
	// The error is logged, and a user friendly error is displayed.
	catch (std::domain_error& e) {
		logger->error(std::format("std::domain_error failed while retrieving resource load: {}", e.what()));
		std::cout << "Failed to get CPU or Memory from OS.";
	}
}

// This gets the CPU usage and memory usage of the computer.
// It checks the average of the CPU usage over SECONDS seconds in order to get an accurate estimate.
int main() {
	const int SECONDS = 3;
	std::cout << "Resource Monitor started.\n";
	
	try {
		// This initalizes the logger.
		auto logger = spdlog::basic_logger_mt("ResourceMonitorLog", "log.txt");
		logger->set_level(spdlog::level::debug);

		logger->info("Program Started.");
		logger->debug(std::format("CPU Check Time: {}", SECONDS));
		
		// This gets the resource usage indefinitely.
		while (true) {
			getResourceUsage(SECONDS, logger);
		}
	}

	catch (const spdlog::spdlog_ex& e) {
		std::printf("Log init failed: %s\n", e.what());
	}
}