#ifndef AR_MAIN_H
#define AR_MAIN_H

#include "ar_simulate.h"

void run_single();
void run_dissipation_test();
void run_spike_comparison_test();
void run_delay_standard_deviation_test();
void run_input_file_test(char* argv[]);
void run_input_noise_test(char* argv[]);

#endif