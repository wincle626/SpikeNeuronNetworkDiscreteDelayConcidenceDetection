#include "ar_io.h"

int min(int a, int b) {
    return a > b ? b : a;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int randint(int range) {
    return range * (rand() / ((double)RAND_MAX + 1));
    //return RandomInt(0, range);
}

double randdouble(double range) {
    return range * drand48();
    //return RandomDouble(0.0, range);
}

int print_activations() {
    
    int i,j;
    double avg = 0.0;
    printf("\nActivation of Nodes:\n");
    for (i=0; i<SHEET_HEIGHT; i++) {
        for (j=0; j<SHEET_WIDTH; j++) {
            printf("%i ", v_net[i*SHEET_WIDTH+j].activation);
            avg+=v_net[i*SHEET_WIDTH+j].activation;
        }
        printf("\n");
    }
    printf("\n");
    avg /= NET_SIZE;
    printf("Average network activation %f\n", avg);
    
    return TRUE;
}

void save_spiketime(int node_id, int time_step) {
    
    char node_id_str[10];
    char time_step_str[10];
    
    sprintf(node_id_str, "%d", node_id);
    //sprintf(time_step_str, "%f", time_step*1.0/10000);
    sprintf(time_step_str, "%i", time_step);
    
    if (spiketimes_file!=NULL) {
        fputs ( node_id_str , spiketimes_file );
        fputs ( " " , spiketimes_file );
        fputs ( time_step_str , spiketimes_file );
        fputs ( "\n" , spiketimes_file );
    }    
}

void save_spikecounts(int spike_count, int time_step) {
    
    char spikecount_str[10];
    char time_step_str[10];
    
    sprintf(spikecount_str, "%d", spike_count);
    sprintf(time_step_str, "%d", time_step);
    
    if (spikecount_file!=NULL) {
        fputs ( time_step_str , spikecount_file );
        fputs ( "," , spikecount_file );
        fputs ( spikecount_str , spikecount_file );
        fputs ( "\n" , spikecount_file );
    }    
    
}

void save_nodeconnections(int node_id) {
    
    int i,c;
    char connection_str[20];
    for (i=0; i<NET_SIZE; i++) {
        // c_net[source][target]
        if (c_net[i][node_id].is_set && nodeconnections_file != NULL) {
            if (v_net[i].is_excit) c = 1; else c = 0;
            sprintf(connection_str, "%d%s%d%s%d%s%d%s", node_id," ", i, " ", c ," ",c_net[i][node_id].delay, "\n");
            fputs ( connection_str, nodeconnections_file);
        }
    }
}

void save_delaydistribution(int delay_bins[MAX_DELAY]) {
    
    int i;
    char delay_str[20];
    for (i=0; i<MAX_DELAY; i++) {
        sprintf(delay_str, "%d%s%d%s", i," ", delay_bins[i], "\n");
        fputs ( delay_str, delaydistribution_file);
    }
}

void save_delaynodedistrib(int node_id, int delay_bins[MAX_DELAY]) {
    
    int i;
    char delay_str[20];
    
    sprintf(delay_str, "%s%d%s", " Node-", node_id, "\n");
    fputs ( delay_str, delaynodedistrib_file);
    
    for (i=0; i<MAX_DELAY; i++) {
        if (delay_bins[i] > 0 ) {
            sprintf(delay_str, "%d%s%d%s", i, " ", delay_bins[i], "\n");
            fputs ( delay_str, delaynodedistrib_file);
        }
    }
}

/** Compute the standard deviation of a list of doubles.
 */
double std_int(int *value_list, int length) {
    
    int i;
    double mean = 0.0;
    double sumdiffsq = 0.0;
    
    for (i=0; i<length; i++) {
        mean += value_list[i]*1.0;
    }
    mean /= length;
    
    
    for (i=0; i<length; i++) {
        sumdiffsq += pow( value_list[i] - mean, 2);
    }
    
    return sqrt(sumdiffsq / length);
    
}

int load_network_file(char *file_name) {
    
    FILE *fp;
    
    fp = fopen(file_name, "r");
    
    if (fp == NULL) {
        printf("Can't open network file...\n");
        return FALSE;
    }
    
    printf("Loading network file...\n");
    
    
    // RESET NETWORK
    int i,j;
    for (i=0; i<NET_SIZE; i++){
        for (j=0; j<NET_SIZE; j++) {
            c_net[i][j].is_set = FALSE;
            c_net[i][j].delay  = MAX_DELAY;
        }
    }
    
    int net_vals[4];
    int n;
    char line[100];
    char field[10];
    
    while (fgets(line, 100, fp) != NULL) {
        //printf("line: %s", line);
        const char *c = line;
        i = 0;
        while ( sscanf(c, "%9[^ ]%n", field, &n) == 1 ) {
            net_vals[i] = atoi(field);
            c+=n;
            if (*c != ' ') break;
            ++c; // skip delimiter
            i++;
        }
        
        c_net[net_vals[1]][net_vals[0]].is_set = TRUE;
        c_net[net_vals[1]][net_vals[0]].delay  = net_vals[3];
        v_net[net_vals[1]].is_excit            = net_vals[2];
        
        //printf("net vals %i %i %i %i %g\n", net_vals[0], net_vals[1],net_vals[2],net_vals[3], weight);
    }
    fclose(fp);
    
    // PRINT NETWORK STATS
    int total_connections   = 0;
    float total_weight      = 0.0;
    int total_delay         = 0;
    int total_excitatory    = 0;
    int max_delay = 0;
    int min_delay = MAX_DELAY;
    
    for (i=0; i<NET_SIZE; i++){
        for (j=0; j<NET_SIZE; j++) {
            if (c_net[i][j].is_set) {
                total_connections++;
                total_delay += c_net[i][j].delay;
                
                if (c_net[i][j].delay>max_delay) max_delay = c_net[i][j].delay;
                if (c_net[i][j].delay<min_delay) min_delay = c_net[i][j].delay;
            }
        }
        
        if (v_net[i].is_excit) total_excitatory++;
    }    
    
    printf("Total connections: %i\n", total_connections);
    printf("Mean afferents per node: %g, Delay mean: %g \n", total_connections*1.0/NET_SIZE, total_delay*1.0/total_connections);
    printf("Default_Threshold %g, Max delay: %i ,  Min delay in network: %i \n", DEFAULT_THRESHOLD*1.0, max_delay, min_delay);
    
    return TRUE;
}


void load_input_patterns(char *input_file_name) {
    
    FILE *fp;
    fp = fopen(input_file_name, "r");
    
    if (fp == NULL) {
        printf("Can't open input file...\n");
		exit(0);
    }
	
    printf("Loading input file...\n");
	
    int in_vals[NET_SIZE];
    int i,n;
    char line[100];
    char field[10];
    int p = 0;
    while (fgets(line, 100, fp) != NULL) {
        //printf("line: %s", line);
        const char *c = line;
        i = 0;
        while ( sscanf(c, "%9[^ ]%n", field, &n) == 1 ) {
            input_patterns[p][i] = atoi(field);
            c+=n;
            if (*c != ' ') break;
            ++c; // skip delimiter
            i++;
        }
		p++;
    }
    fclose(fp);
}