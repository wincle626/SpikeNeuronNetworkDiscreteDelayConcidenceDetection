#include "ar_main.h"


void run_single() {

	if (SAVE_SPIKETIMES)  spiketimes_file = fopen("out_spiketimes.txt", "w");
	if (SAVE_SPIKECOUNTS) spikecount_file = fopen("out_spikecounts.txt", "w");
	
	pattern_id = 4; //4: sequential synch input
	connect_network();
	init_nodes();
	reset_axons();
	run_simulation(0);
	
	if (SAVE_SPIKETIMES) fclose(spiketimes_file);
	if (SAVE_SPIKECOUNTS) fclose(spikecount_file);
}

void run_dissipation_test() {
	
	int i;
	
	double avg_dissip_time = 0.0;
	int dissip_time;
	for (i=0; i<30; i++) {
		if (SAVE_SPIKETIMES)  spiketimes_file = fopen("out_spiketimes.txt", "w");
		init_nodes();
		reset_axons();
		connect_network();
		dissip_time = run_simulation(i) + 1;
		printf("Dissipation time for run %i = %i\n", i, dissip_time);
		avg_dissip_time += dissip_time;
		if (SAVE_SPIKETIMES)  fclose(spiketimes_file);
	}
	avg_dissip_time /= 30.0;
	
	double avg_spikecount;
	for (i=0; i<DSD_SAMPLES_NUMBER; i++) {
		avg_spikecount += dsd_spikecount_list[i];
	}
	avg_spikecount /= DSD_SAMPLES_NUMBER;
	
	printf("\n\nDISIPATION TEST RESULTS\n");
	printf("Average dissipation time: %g of %i\n", avg_dissip_time, UPDATE_STEPS);
	printf("Average spike number: %g\n", avg_spikecount);
	
}

void run_spike_comparison_test() {
	int i;
	// compare between two different inputs
	if (SAVE_SPIKETIMES)  spiketimes_file = fopen("out_spiketimes.txt", "w");
	if (SAVE_SPIKECOUNTS) spikecount_file = fopen("out_spikecounts.txt", "w");
	
	int input_c[NET_SIZE];
	for (i=0; i<NET_SIZE; i++) {
		// input an c pattern in a 5x5 grid
		//if (i==6 || i==7 || i==8 || i==11 || i==16 || i==17 || i==18) {
		//if (i==10 || i==11 || i==12 || i==13 || i==14 || i==15) {
		if (i<6) {
			input_c[i] = 1;
		} else {
			input_c[i] = 0;
		}
	}
	
	int input_t[NET_SIZE];
	for (i=0; i<NET_SIZE; i++) {
		// input an o pattern in a 5x5 grid
		//if (i==6 || i==7 || i==8 || i==12 || i==17 || i==22) {
		//if (i==10 || i==11 || i==12 || i==13 || i==14 || i==16) {
		if (i < 6) { //12 && i >= 6) {
			input_t[i] = 1;
		} else {
			input_t[i] = 0;
		}
	}
	
	// use same network for both tests
	init_nodes();
	connect_network();
	
	//test 1
	//set_activation(input_c);
	run_simulation(0);
	if (SAVE_SPIKETIMES) fclose(spiketimes_file);
	
	init_nodes();
	reset_axons();
	
	// test 2
	//set_activation(input_t);        
	if (SAVE_SPIKETIMES) spiketimes_file = fopen("out_spiketimes2.txt",  "w");
	run_simulation(0);
	
	if (SAVE_SPIKETIMES) fclose(spiketimes_file);
	if (SAVE_SPIKECOUNTS) fclose(spikecount_file);
}	

void run_delay_standard_deviation_test() {
	int i;
	// Delay Standard Deviation and Spiketimes files generation for averaging
	char spiketimes_filename_str[35];
	char spikecounts_filename_str[35];
	dsd_meanspikes = 0.0;
	for (i=0; i<DSD_SAMPLES_NUMBER; i++) {
		sprintf(spiketimes_filename_str, "%s%d%s%d%s", "drange", MAX_DELAY-MIN_DELAY,"_out_spiketimes", i, ".txt");
		if (SAVE_SPIKETIMES) spiketimes_file = fopen(spiketimes_filename_str, "w");            
		sprintf(spikecounts_filename_str, "%s%d%s%d%s", "drange", MAX_DELAY-MIN_DELAY,"_out_spikecounts", i, ".txt");
		if (SAVE_SPIKECOUNTS) spikecount_file = fopen(spikecounts_filename_str, "w");
		
		init_nodes();
		reset_axons();
		connect_network();
		run_simulation(i);
		if (SAVE_SPIKETIMES) fclose(spiketimes_file);
		if (SAVE_SPIKECOUNTS) fclose(spikecount_file);
	}
	dsd_meanspikes /= (1.0*DSD_SAMPLES_NUMBER);
	
	printf("Mean spike count over %i runs is %g, SD: %g\n", DSD_SAMPLES_NUMBER, dsd_meanspikes, std_int(dsd_spikecount_list, DSD_SAMPLES_NUMBER) );
	
	printf("[");
    for (i=0; i<DSD_SAMPLES_NUMBER; i++){
        printf("%i, ", dsd_spikecount_list[i]);
    }
    printf("]\n");
	
}


void run_input_file_test(char *argv[]) {
	int i, j;
	
	if (argv[1] == NULL) {
		printf("Generating Network...\n");
		connect_network();
		return;
	}
	
	pattern_id  = 0;
	pattern_id2 = 0;
	
	// init input_patterns
	for (i=0; i<INPUT_FILE_PATTERNS; i++) {
		for (j=0; j<NET_SIZE; j++) {
			input_patterns[i][j] = 0;
		}
	}
	// loading the input file
	load_input_patterns("binary_input_list_25-6.txt");
	
	for (i=0; i<INPUT_FILE_PATTERNS; i++) {
	//for (i=0; i<1; i++) {  
		input_patterns_counter = i;
		
		init_nodes();
		reset_axons();
		
		if (argv[1] != NULL) {            
			printf("\nUsing Existing Network...\n");
			if (load_network_file(argv[1])) printf("Loaded Network File Successfully!\n");
		} else {
			printf("Generating Network...\n");
			connect_network();
		}
		
		if (argv[1]!= NULL && argv[2] != NULL) {
			pattern_id = atoi(argv[2]); // The type of input pattern to present: see get_input()
			if (argv[3] != NULL) pattern_id2 = atoi(argv[3]);// The second input type.
			
			char spiketimes_filename_str[100];
			sprintf(spiketimes_filename_str, "%s%d%s", "files_spiketimes/out_spiketimes_", i, ".txt");
			spiketimes_file = fopen(spiketimes_filename_str, "w");
		} else {
			spiketimes_file = fopen("out_spiketimes.txt", "w");
		}
		if (SAVE_SPIKECOUNTS) spikecount_file = fopen("out_spikecounts.txt", "w");
		
		printf("\nRunning...\n");
		
		run_simulation(0);
		
		if (SAVE_SPIKETIMES) fclose(spiketimes_file);
		if (SAVE_SPIKECOUNTS) fclose(spikecount_file);
		
	}
	
}




void run_input_noise_test(char *argv[]) {
	int i, j;
	
	pattern_id = 4; // fixed synchronous input
	noisy_cells = 0;

	if (argv[1] == NULL) {
		printf("Generating Network...\n");
		connect_network();
		return;
	}
	
	// generate n sequences of noisy responses: one for each network
	int n;
	for (n=1; n<31; n++) {
	
	noisy_cells = 0;
	i = 0;
	//for (i=0; i<NET_SIZE; i++) {  // Do one test after removing up to NET_SIZE neurones
	while (i<NET_SIZE) {
		
		init_nodes();
		reset_axons();
		
		if (argv[1] != NULL) {            
			printf("\nUsing Existing Network...\n");
			//if (load_network_file(argv[1])) printf("Loaded Network File Successfully!\n");
			char net_filename_str[100];
			sprintf(net_filename_str, "%s%d%s", "out_nodeconnections_", n, ".txt");
			if (load_network_file(net_filename_str)) printf("Loaded Network File Successfully!\n");
		}
		
		if (argv[1]!= NULL && argv[2] != NULL) {
			pattern_id = atoi(argv[2]); // The type of input pattern to present: see get_input()
			
			char spiketimes_filename_str[100];
			sprintf(spiketimes_filename_str, "%s%d%s%d%s", "seq", n, "/out_spiketimes_", i, ".txt");
			
			spiketimes_file = fopen(spiketimes_filename_str, "w");
		} else {
			spiketimes_file = fopen("out_spiketimes.txt", "w");
		}
		if (SAVE_SPIKECOUNTS) spikecount_file = fopen("out_spikecounts.txt", "w");
		
		printf("\nRunning...\n");
		
		run_simulation(0);
		
		if (SAVE_SPIKETIMES) fclose(spiketimes_file);
		if (SAVE_SPIKECOUNTS) fclose(spikecount_file);
	
		i += 10;
		noisy_cells += 10;
	}
	
	} // end generate n sequences
}


int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    srand48(time(NULL));
    
    nodeconnections_file   = fopen("out_nodeconnections.txt", "w");
	// delaydistribution_file = fopen("out_delaydistribution.txt", "w");
	// delaynodedistrib_file  = fopen("out_delaynodedistrib.txt", "w");
    
	// run_dissipation_test();
		
	// run_spike_comparison_test();
		
	// run_delay_standard_deviation_test();
	
	// run_input_file_test(argv);
    
	// run_input_noise_test(argv);
	
	run_single();
	
	
    fclose(nodeconnections_file);
	// fclose(delaydistribution_file);
	// fclose(delaynodedistrib_file);    

	return 0;
}