#include "ar_simulate.h"


/** Initialize the network's status
 */
void init_nodes() {
    
    int i, j, in_num;
    
    for (i=0; i<NET_SIZE; i++) {
        
        v_net[i].threshold = DEFAULT_THRESHOLD;
        
        v_net[i].refractory_time = 0;
        
        v_net[i].activation = RESET;
        
        v_net[i].leak_status = 0;
        
        v_net[i].is_excit = randdouble(1.0) > INHIB_PERCENT ? TRUE : FALSE;
        
        v_net[i].dpush = MAX_DELAY-1;
                
    }
}

/** get_input(int time_step)
 * Adds input activity to the v_net neurones.
 */

void get_input(int time_step) {
    
    int i;
	
    if (INPUT_ASYNC && time_step%ASYNC_INPUT_PERIOD == 0 && time_step < ASYNC_INPUT_STEPS*ASYNC_INPUT_PERIOD) {
        
        // assign activations
        for (i=0; i<NET_SIZE; i++) {
            if (input_pattern[i] == SPIKE) {
                v_net[i].activation  = DEFAULT_THRESHOLD;
                v_net[i].leak_status = LEAK_PERIOD;
            }
        }
	}

	int size_counter = 6;//240;//24;//6;
	int rand_cell = 0;

    if (INPUT_SYNCHRONOUS) {
        
        
        for(i=0; i<NET_SIZE; i++) {
            
			
			// INPUT_PATTERNS FROM FILE
			if (time_step == 0 && input_patterns[input_patterns_counter][i] == 1 && pattern_id == 1) {
                v_net[i].activation  = DEFAULT_THRESHOLD;
                v_net[i].leak_status = LEAK_PERIOD;				
			}
			
            // PERIODIC INPUT PULSES
            if (time_step%(200*TIME_RESOLUTION) == 0 && i >= 0 && i <3 && pattern_id == 2) {
                v_net[i].activation = DEFAULT_THRESHOLD;
                v_net[i].leak_status = LEAK_PERIOD;
			}
			
			// Random synchronous input
            if (time_step == 450 && size_counter > 0 && pattern_id == 3){
                rand_cell = randint(NET_SIZE);
                v_net[rand_cell].activation = v_net[rand_cell].threshold;
                v_net[rand_cell].leak_status = LEAK_PERIOD;
                size_counter--;
            }
			
			// Fixed synchronous
			if (time_step == 0 && pattern_id == 4){
				// stim cells 0 to size_counter-1
				if (i < size_counter) {
					v_net[i].activation = v_net[i].threshold;
					v_net[i].leak_status = LEAK_PERIOD;					
				}
			}
		}
	}
        
	if (INPUT_MAZE) {
		
		for (i=0; i< NET_SIZE; i++) {
			            
            // CONTEXT STIM A
            if (time_step == 450 && i >= 0 && i < 6 && pattern_id == 2) {
                v_net[i].activation = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }            
            
            // CONTEXT STIM B
            if (time_step == 450 && i >= 6 && i < 12 && pattern_id == 3) {
                v_net[i].activation = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }
            
            // CUE STIM X
            if (time_step == 800 && i >= 12 && i < 18 && pattern_id2 == 4){
                v_net[i].activation = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;            
            }
            
            // CUE STIM Y
            if (time_step == 800 && i >= 18 && i < 24 && pattern_id2 == 5){
                v_net[i].activation = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;            
            } 
            
            
            // CONTEXT STIM C
            if (time_step == 450 && (i==0 || i ==1 || i==6 || i==7 || i==12 || i==13) && pattern_id == 6){
                v_net[i].activation  = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }

            // CONTEXT STIM D
            if (time_step == 450 && (i==8 || i ==9 || i==14 || i==15 || i==20 || i==21) && pattern_id == 7){
                v_net[i].activation  = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }
            
            // CUE STIM V
            if (time_step == 800 && (i==16 || i ==17 || i==22 || i==23 || i==2 || i==3) && pattern_id2 == 8){
                v_net[i].activation  = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }

            // CUE STIM W
            if (time_step == 800 && (i==4 || i ==5 || i==10 || i==11 || i==18 || i==19) && pattern_id2 == 9){
                v_net[i].activation  = v_net[i].threshold;
                v_net[i].leak_status = LEAK_PERIOD;
            }
            
        }
    }
    
}

/** set_activation
 * Set the network activation based on an input matrix of activation coefficients.
 */
void set_activation (int input[NET_SIZE]) {
    
    int i;
    for (i=0; i<NET_SIZE; i++) {
        v_net[i].activation = DEFAULT_THRESHOLD * input[i];
        if (input[i] > 0) v_net[i].leak_status = LEAK_PERIOD;
    }
}

/** void reset_axons()
 * reset all connection axons to the inactive state.
 */
void reset_axons() {
    int s,k;
    for (s=0; s<NET_SIZE; s++) {
        for (k=0; k<MAX_DELAY; k++) {
            v_net[s].spike_train[k] = RESET;
        }
    }
}

/** run_simulation
 * Perform the simulation for UPDATE_STEPS number of steps.
 */
int run_simulation(int iteration) {
            
    // simulation inits
    int step_spike_count = 0;
    int i,s,t,l,a,b;
    int last_spike_step = 0; // keep track of last spike to detect dissipation point.
    int total_spikecount = 0;
    
    //init individual_cell_spikecount
    for (i=0; i<NET_SIZE; i++) individual_cell_spikecount[i] = 0;
    
    for (i=0; i<UPDATE_STEPS; i++) {
        
        
        // add any external input
        get_input(i);
        
        // update spike trains and reset source nodes in first cycle
        for (s=0; s<NET_SIZE; s++) {
                        
            // if source node spikes then :
            if (v_net[s].activation >= v_net[s].threshold) {
                
                // add spike to edge's spike_train
                v_net[s].spike_train[v_net[s].dpush] = SPIKE;
                v_net[s].dpush--;
                if (v_net[s].dpush < 0) v_net[s].dpush = MAX_DELAY-1;
            
                // reset source node
                v_net[s].activation = RESET;
                // set refractory period
                v_net[s].refractory_time = REFRACTORY_PERIOD;
                
                step_spike_count++; 
                if (SAVE_SPIKETIMES) save_spiketime(s, i);
                individual_cell_spikecount[s]++;
                
            } else  {
                //v_net[s].spike_train[0] = RESET;

                v_net[s].spike_train[v_net[s].dpush] = RESET;
                v_net[s].dpush--;
                if (v_net[s].dpush < 0) v_net[s].dpush = MAX_DELAY-1;
            }
            
            // FOR PURE COINCIDENCE DETECTION (no sub-threshold integration)
            //v_net[s].activation = RESET;
            
            // FOR LEAKY INTEGRATION (some sub-threshold integration)
            if (v_net[s].leak_status == 0) {
                v_net[s].activation = RESET;
            }
            // Decrement cell's leak_status
            if (v_net[s].leak_status > 0)  {
                //printf("step: %i, leak neurone %i, status %i\n",i, s, v_net[s].leak_status);
                v_net[s].leak_status--;
            }
            
            // Decrement cell's supra-theshold if any
            if (v_net[s].threshold > DEFAULT_THRESHOLD) {
                v_net[s].threshold = max(DEFAULT_THRESHOLD, v_net[s].threshold-10);
            }
            
        }
        
        // update the target node in second cycle
        
        // Global inhibition: Compute the exponential time difference since last spike
        double glob_inhib;
        if (GLOBAL_INHIBITION == 0.0) {
            glob_inhib = 0.0;
        } else {
            glob_inhib = exp(-(i-last_spike_step)*1.0/GLOBAL_INHIBITION);
        }
        //printf("time %i     last %i     global_inhib %g\n", i, last_spike_step, glob_inhib);
        
        int pre_activation, sum_activation;
        int dpull;
        for (t=0; t<NET_SIZE; t++) {
            
            if (v_net[t].refractory_time > 0) { // check if refractory period is over
                
                v_net[t].refractory_time--;
                
            } else { // refractory period is over
                
                pre_activation = v_net[t].activation;
                sum_activation = 0;
                
                for (s=0; s<NET_SIZE; s++) {
                    
                    if (c_net[s][t].is_set) {
                        
                        // dpull: the circular index to pull voltage from this axon according to delay
                        dpull = (v_net[s].dpush+c_net[s][t].delay)%MAX_DELAY;
                        //if (v_net[s].is_excit && glob_inhib < drand48()){
                        if (v_net[s].is_excit){
                            sum_activation += v_net[s].spike_train[dpull];
                        } else { 
                            sum_activation -= v_net[s].spike_train[dpull];
                        }
                    }
                }
                
                // Global Inhibition
                //sum_activation -= (int) round(SPIKE*glob_inhib); // USE GLOB_INHIB 2.7 for TIME_RESOLUTION 10
                
                //v_net[t].threshold = (int) DEFAULT_THRESHOLD + round(SPIKE*glob_inhib);
                //printf("threshold[%i] %i\n", t, v_net[t].threshold);
                
                // Use increasing Threshold method : use global inhib 0.7 approx
                //if (sum_activation >= v_net[t].threshold) v_net[t].threshold += (int) ((sum_activation-v_net[t].threshold)*glob_inhib);
                
                v_net[t].activation = sum_activation+pre_activation;
                                
                // set leak_status if there is at least one new incoming spike 
                if(sum_activation > 0) { 
                    //v_net[t].leak_status = LEAK_PERIOD;
                    v_net[t].leak_status = min((int) ((sum_activation*1.0/v_net[t].threshold) * LEAK_PERIOD), LEAK_PERIOD);
                    //printf("| %i leak_status %i frac %i\n", t, v_net[t].leak_status, (int) ((sum_activation*1.0/v_net[t].threshold) * LEAK_PERIOD));
                }
				
            }
			
			
			// Remove Spikes for NOISE set
			if (NOISE_SPIKEREMOVAL && t < noisy_cells && i >= 800){ // Prolonged (cont) removal
			//if (NOISE_SPIKEREMOVAL && t < noisy_cells && i >= 800 && i < 811){ // Temporary (burst) removal
				v_net[t].activation  = RESET;
				v_net[t].leak_status = 0;
			}
			
			// Add Spikes for NOISE
			if (NOISE_SPIKEADDITION && t < noisy_cells && i == 800) { // Temporary misfiring
			//if (NOISE_SPIKEADDITION && t < noisy_cells && i >= 800) { // Prolonged misfiring
				//if (randint(10) == 0) { //Prolonged misfiring
					v_net[t].activation	 = v_net[t].threshold;
					v_net[t].leak_status = LEAK_PERIOD;
				//}
			}
			
        }
        
        // Save last_spike_step for global inhibition
        if (step_spike_count > 0) {
            last_spike_step = i;
        }        
        
        // compute average threshold over eash step
        /*
        if (i%20 == 0) {
            int thresh_avg = 0;
            for (s=0; s<NET_SIZE; s++) {
                thresh_avg += v_net[s].threshold;
            }
            printf("step: %i avg thresh %g  glb_inh %g\n", i, thresh_avg*1.0/NET_SIZE, glob_inhib);
        }
        */
        
        /**** DATA RECORDING ****/
        
        // save spike counts
        if (SAVE_SPIKECOUNTS) save_spikecounts(step_spike_count, i);
        
        dsd_meanspikes += step_spike_count;
        total_spikecount += step_spike_count;
        step_spike_count = 0;
    }
    printf("Total spikes: %i\n", total_spikecount);
    dsd_spikecount_list[iteration] = total_spikecount;    
    
    printf("Individual cell spikecount: ");
    for (i=0; i<NET_SIZE; i++) {
        printf("%i, ", individual_cell_spikecount[i]);
    }
    printf("\n");
    
    return last_spike_step;
    
}