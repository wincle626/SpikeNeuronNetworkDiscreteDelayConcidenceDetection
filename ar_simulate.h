#ifndef AR_SIMULATE_H
#define AR_SIMULATE_H

#include "ar_network.h"

void init_nodes();
void reset_axons();
void get_input(int time_step);
void set_activation (int input[NET_SIZE]);
int run_simulation();

#endif