/*
 * The FreeBSD variant of the Performance class.
 * This class does two jobs:
 * 1. Collect basic information like the number of CPUs.
 * 2. Collect system-wide performance information.
 */

/*
 * Copyright (c) 2017 Eric van Gyzen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// Turn on to display additional performance messages.
#define PERFORMANCE_DETAILS	0

#include <sys/types.h>
#include <sys/sysctl.h>

#include <assert.h>

#include <cstddef>
#include <cstdint>

#include "IOPerformance.h"

//
// Initializing system performance data.
//
Performance::Performance()
{
	// Obtaining the number of CPUs in the system and their speed.
	processor_count = Get_Processor_Count();
	timer_resolution = Get_Timer_Resolution();

	// Network performance statistics are disabled by default.  Assume this
	// unless later performance calls succeed.  Then, set the correct
	// number of interfaces.
	network_interfaces = 0;

	if (!timer_resolution || !processor_count) {
		cout << "*** Unable to initialize needed performance data.\n"
			"This error may indicate that you are trying to run\n"
			"on an unsupported processor or OS.\n"
			"See the Iometer User's Guide for\n"
			"information on supported platforms.\n";
		exit(1);
	}

	// Set the clock ticks per second
	// TODO: try to find a correct way to get this number
	clock_tick = sysconf(_SC_CLK_TCK);

	// Initialize all the arrays to 0.
	memset(raw_cpu_data, 0, sizeof(raw_cpu_data));
	memset(raw_ni_data, 0, sizeof(raw_ni_data));
	memset(raw_tcp_data, 0, sizeof(raw_tcp_data));
}

//
// Freeing memory used by class object.
//
Performance::~Performance()
{
}

int Performance::Get_Processor_Count()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

//
// Get the speed of the processors in Hz.
//
double Performance::Get_Timer_Resolution()
{
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_XSCALE) || defined(IOMTR_CPU_X86_64)
	std::uint64_t freq;
	std::size_t freq_size = sizeof(freq);
	int result;

	result = sysctlbyname("machdep.tsc_freq", &freq, &freq_size, NULL, 0);
	if (result != 0 || freq_size != sizeof(freq)) {
		freq = 0;
	}
	return ((double)freq);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot)
{
	// Get the performance data stored by the system.
#ifdef _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

	static long jiffies = 0;
	time_counter[snapshot] = jiffies++;	// TODO which timecounter?
	if (snapshot == LAST_SNAPSHOT) {
		// calculate time diff in clock ticks..
		timediff = (time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);
	}

	switch (perf_data_type) {
	case PERF_PROCESSOR:
		Get_CPU_Counters(snapshot);
		break;
	case PERF_NETWORK_TCP:
		Get_TCP_Counters(snapshot);
		break;
	case PERF_NETWORK_INTERFACE:
		Get_NI_Counters(snapshot);
		break;
	default:
		break;
	}
}

void Performance::Get_CPU_Counters(int snapshot)
{
	// TODO
	for (int i = 0; i < processor_count; i++) {
		raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] = 0;
		raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot] = 0;
		raw_cpu_data[i][CPU_TOTAL_UTILIZATION][snapshot] = 0;
		raw_cpu_data[0][CPU_IRQ][snapshot] = 0;
	}
}

void Performance::Get_TCP_Counters(int snapshot)
{
	// TODO
	raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
}

//
// Calculating CPU statistics based on snapshots of performance counters.
//
void Performance::Calculate_CPU_Stats(CPU_Results * cpu_results)
{
	// TODO
	// Loop though all CPUs and determine various utilization statistics.
	cpu_results->count = processor_count;
	for (int cpu = 0; cpu < processor_count; cpu++) {
		// Loop through the counters and calculate performance.
		for (int stat = 0; stat < CPU_RESULTS; stat++) {
			double result;

			if (stat == CPU_IRQ) {
				// we have to calculate Interrupts/sec.
				result = 0.0;
				    //((double)raw_cpu_data[cpu][stat][LAST_SNAPSHOT] -
				    //raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) * clock_tick / timediff;
				cpu_results->CPU_utilization[cpu][stat] = result;
			} else {
				// All other CPU statistics.
				result = 0.0;
				    //((double)raw_cpu_data[cpu][stat][LAST_SNAPSHOT] -
				    //raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / timediff;

				if (result < 0.0) {
					result = 0.0;
					//
					// CPU Utilization figures are outside valid range far too often.
					// Ok, not in every cycle but frequent still.
					// So, it is better to comment it out rather than have the message
					// pop up on the screen at regular intervals.
					//
					// cout << "***** Error : CPU utilization outside valid timerange 0% - 100% *****" << endl;
				}
				if (result > 1.0) {
					result = 1.0;
				}

				cpu_results->CPU_utilization[cpu][stat] = (result * 100);
			}

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "CPU " << cpu << " recorded stat " << stat << " = "
			    << cpu_results->CPU_utilization[cpu][stat] << endl;
#endif
		}
	}
}

//
// Calculate network performance statistics based on snapshots of performance counters.
//
void Performance::Calculate_TCP_Stats(Net_Results * net_results)
{
	// TODO
	// Loop through the counters and calculate performance.
	for (int stat = 0; stat < TCP_RESULTS; stat++) {
		// If we've never set the counter offsets, then we've never successfully retrieved
		// the performance data.  Set all of the values to 0.
		double result;

		//result = ((double)raw_tcp_data[stat][LAST_SNAPSHOT] - raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
		result = 0.0;
		result *= clock_tick;	// note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;

#if PERFORMANCE_DETAILS || _DETAILS
		cout << "TCP recorded stat " << stat << " = " << net_results->tcp_stats[stat] << endl;
#endif
	}
}

//
// Extract counters for network interface performance data.
//
void Performance::Get_NI_Counters(int snapshot)
{
	// TODO
	// In Linux, this modifies the network_interfaces member variable.
	for (int i = 0; i < MAX_NUM_INTERFACES; i++) {
		raw_ni_data[i][NI_IN_ERRORS][snapshot] = 0;
		raw_ni_data[i][NI_OUT_ERRORS][snapshot] = 0;
		raw_ni_data[i][NI_PACKETS][snapshot] = 0;
	}
}

//
// Calculate network performance statistics based on snapshots
// of performance counters.
//
void Performance::Calculate_NI_Stats(Net_Results * net_results)
{
	int net, stat;		// Loop control variables.

	// Loop through the counters and calculate performance.
	net_results->ni_count = network_interfaces;
	for (net = 0; net < network_interfaces; net++) {
		for (stat = 0; stat < NI_RESULTS; stat++) {
			// If we've never set the counter offsets, then we've never successfully retrieved
			// the performance data.  Set all of the values to 0.
			double result;

			//
			// Note:
			//              The array time_counter[] stores time in nanoseconds.
			// Earlier, we used to divide by the calculated value of timediff and then
			// multiply the result by clock_ticks per second to get the NI_data per
			// second which was theoretically correct (and mathematically same as what 
			// we are doing now) but reported wrong values while working with such 
			// large numbers.
			//
			//result = ((double) raw_ni_data[net][stat][LAST_SNAPSHOT] - 
			//      raw_ni_data[net][stat][FIRST_SNAPSHOT]) * 1000000000.0 / 
			//      ((double) time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);

			// DF: I think the time_counter do not use nanoseconds but clock_tick
			result = ((double)raw_ni_data[net][stat][LAST_SNAPSHOT] -
				  raw_ni_data[net][stat][FIRST_SNAPSHOT]) * clock_tick / timediff;

			net_results->ni_stats[net][stat] = result;

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "   Network interface " << net << " recorded stat " << stat << " = "
			    << net_results->ni_stats[net][stat] << endl;
#endif
		}
	}
}

// vim: noet sts=8 sw=8
