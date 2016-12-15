#ifndef AR_IO_H
#define AR_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define FALSE 0
#define TRUE 1

/** TIME RESOLUTION **/
#define TIME_RESOLUTION 1 // resolution of time step (10th of ms)

/** NETWORK PARAMS **/
#define NET_SIZE        20//100//25
#define SHEET_WIDTH     5
#define SHEET_HEIGHT    5

/** CONNECTION PARAMS **/
#define INHIB_PERCENT 0.0					// percentage of inhibitory nodes
#define CONNECTION_PROB 1.0//0.025//0.25//1.0	// probability of connecting to other node
#define CONNECT_EXCIT_PROB 0.6//0.05		// probability that an excitatory connection is made
#define CONNECT_INHIB_PROB 0.6//0.125		// probability that an inhibitory connection is made

#define OVERRIDE_TOPOLOGY TRUE          // If true, then randomly assign delays using RAND_DELAY_RANGE
#define DELAY_EXCIT_DECAY_CONST 10
#define DELAY_INHIB_DECAY_CONST DELAY_EXCIT_DECAY_CONST//(3*DELAY_EXCIT_DECAY_CONST);
#define MIN_DELAY (TIME_RESOLUTION*20)
#define MAX_DELAY (TIME_RESOLUTION*40)
#define DELAY_CLUSTERS 1                // Number of delay subsets (1 is uniform distrib): Delay distrib from MinDel to MaxDel is divided by 2*DELAY_CLUSTERS-1 and
                                        // Every second delay set is used on an equal share of connections

#define CONNECT_SMALL_WORLD FALSE    // connect network using preferential attachement
#define SMALL_WORLD_CONSTANT 0.02   // a smaller constant means smaller std

/** NODE PARAMS **/
#define DEFAULT_THRESHOLD 2000 // 6 // Nutzel et al. 94: I/T = Log(NET_SIZE), so T = I/Log(NET_SIZE)
#define REFRACTORY_PERIOD (TIME_RESOLUTION*10) // 30 // the time step before a node's activity can change again (10 = 1ms at 0.1ms resolution)
#define SPIKE 1000                             // the node spike value
#define RESET 0                             // the node reset value
#define LEAK_PERIOD (TIME_RESOLUTION*1)     // the number of time steps before sub-threshold is lost

/** GLOBAL INHIBITION **/
#define GLOBAL_INHIBITION   0.0//1.3   //DOUBLE The higher the greater the inhibition: usually between 1.0 to 3.0

/** INPUT OPTIONS **/
#define INPUT_SYNCHRONOUS   TRUE
#define INPUT_ASYNC			FALSE
#define INPUT_MAZE			FALSE
#define INPUT_SPIKE_NUM     1080//280
#define INIT_SPIKE_PERCENT  0.25 //0.3    // Used Only in Random Init Mode - percentage of spiking nodes at initial step
#define ASYNC_INPUT_PERIOD  200
#define ASYNC_INPUT_STEPS   1       // Apply input on regular intervals of period REFRACTORY_PERIOD 
                                    // of amount INPUT_SPIKE_NUM/ASYNC_INPUT_STEPS : this keeps the amount of async input
                                    // proportional to the synchronous case
#define INPUT_FILE_PATTERNS 38760 //177100//2300
#define NOISE_SPIKEREMOVAL	FALSE	// Remove spikes from 'noisy_cells' amount of neurones
#define NOISE_SPIKEADDITION TRUE	// Force spiking of cells at random time steps

/** SIMULATION PARAMS **/
#define UPDATE_STEPS 600
#define SAVE_SPIKETIMES TRUE
#define SAVE_SPIKECOUNTS TRUE
/** TEST TYPE **/
#define RUN_DISSIPATION_TEST FALSE
#define RUN_SPIKE_COMPARISON_TEST FALSE
#define RUN_DSD_TEST FALSE          // Delay SD mean spike test: mean number of spikes over N runs for a given DSD
#define DSD_SAMPLES_NUMBER 30       // Number of simulations to run to collect statistics


typedef struct{
    int threshold;
    int activation;
    int refractory_time;
    int leak_status;
    int spike_train[MAX_DELAY];
    int is_excit;
    int dpush;
}node;

typedef struct{
    int is_set;
    int delay;
}edge;

node v_net[NET_SIZE];
edge c_net[NET_SIZE][NET_SIZE];

// input_patterns[ Choose(N,K) ] [ NET_SIZE ]
int input_patterns[INPUT_FILE_PATTERNS][NET_SIZE];
int input_patterns_counter;

int input_pattern[NET_SIZE];
int individual_cell_spikecount[NET_SIZE];

FILE *spiketimes_file;
FILE *spikecount_file;
FILE *nodeconnections_file;
FILE *delaydistribution_file;
FILE *delaynodedistrib_file;

double dsd_meanspikes;
int dsd_spikecount_list[DSD_SAMPLES_NUMBER];
int pattern_id;
int pattern_id2;
int noisy_cells;

int min(int a, int b);
int randint(int range);
double randdouble(double range);
int print_activations();
void save_spiketime(int node_id, int time_step);
void save_spikecounts(int spike_count, int time_step);
void save_nodeconnections(int node_id);
void save_delaydistribution(int delay_bins[MAX_DELAY]);
void save_delaynodedistrib(int node_id, int delay_bins[MAX_DELAY]);
double std_int(int *value_list, int length);
int load_network_file(char *file_name);
void load_input_patterns(char *input_file_name);

#endif