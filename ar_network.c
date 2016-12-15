#include "ar_network.h"

/** void connect_network()
 * Generate a delay network with uniform or preferential attachment
 * for a small world topology.
 */
void connect_network() {
    
    int i,j,k;
    
    // preferential attachment
    int s,t;
    int afferents_arr[NET_SIZE];
    double target_score_arr[NET_SIZE], max_score;
    int connections_bin;
    
    // init afferents_array and c_net
    for (i=0; i<NET_SIZE; i++) {
        afferents_arr[i] = 0;
		v_net[i].is_excit = randdouble(1.0) > INHIB_PERCENT ? TRUE : FALSE;
        for (j=0; j<NET_SIZE; j++) {
            c_net[i][j].is_set = FALSE;
        }
    }
    
    connections_bin = CONNECTION_PROB * NET_SIZE * NET_SIZE;
    printf("Connections to make: %i\n", connections_bin);
    
    if (CONNECT_SMALL_WORLD) {
        printf("Connecting small world...\n");
        for (i=0; i<connections_bin; i++) {
            
            // pick random source
            s = randint(NET_SIZE);
            t = randint(NET_SIZE);
            
            if (c_net[t][s].is_set) {
                i--;
                continue;
            }
            
            // use montecarlo to determine target cell selection probability
            // target with highest score on this round gets selected
            for (j=0; j<NET_SIZE; j++) {
                target_score_arr[j] = randdouble(CONNECTION_PROB * afferents_arr[j]/NET_SIZE + 1.0);//randdouble( CONNECTION_PROB + afferents_arr[j] * SMALL_WORLD_CONSTANT / (NET_SIZE - (CONNECTION_PROB * NET_SIZE)) );
                //else target_score_arr[j] = randdouble( CONNECTION_PROB );
            }
            
            // get max target_score_arr value
            max_score = 0.0;
            for (j=0; j<NET_SIZE; j++) {
                if (target_score_arr[j] > max_score) {
                    max_score = target_score_arr[j];
                    t = j;
                }
            }
            
            // set connection with probability
            if (t == s) { 
                c_net[t][s].is_set = FALSE; // prevent connection-to-self
                i--; // don't use up the connection from the bin
            } else if (c_net[t][s].is_set) {
                i--; // don't re-set and existing connection
            } else {
                c_net[t][s].is_set = TRUE;
                
                afferents_arr[t]++;
                //printf("Connections made: %i\n", i);
            }        
        }
    } else if (!OVERRIDE_TOPOLOGY) {
        printf("Connecting randomly with open boundaries...\n");
        int mx = SHEET_WIDTH/2;
        int my = SHEET_HEIGHT/2;
        
        float px, py, p_exc, p_inh;
        
        for (s=0; s<NET_SIZE; s++) {
            
            // determine if the connection should be set or not based on topological rules 
            // corner cells connect 4x less than center cells            
            // if source is away from center then reduce the probability of making a connection proportionally
            px = 1 - (abs(mx - get_node_x(s))*0.5 / mx);
            py = 1 - (abs(my - get_node_y(s))*0.5 / my);
            
            p_exc = px*py*CONNECT_EXCIT_PROB;
            p_inh = px*py*CONNECT_INHIB_PROB;
            
            //printf("neuron %i, excit prob %f, inhib prob %f\n", s, p_exc, p_inh);
            
            for (t=0; t<NET_SIZE; t++) {
                if (s != t) {
                    // if source node is excitatory
                    if (v_net[s].is_excit) {
                        if (randdouble(1.0) < p_exc) {
                            c_net[s][t].is_set = TRUE;
                            afferents_arr[t]++;
                        }
                        // if source is inhibitory
                    } else {
                        if (randdouble(1.0) < p_inh) {
                            c_net[s][t].is_set = TRUE;
                            afferents_arr[t]++;
                        }
                    }
                }
            }
        }
        
    } else {
        // connect with uniform random distribution
        printf("Connecting randomly...\n");
        for (t=0; t<NET_SIZE; t++) {
            for (s=0; s<NET_SIZE; s++) {
                if (randdouble( 1.0 ) <= CONNECTION_PROB && s != t) {
                    c_net[s][t].is_set = TRUE;
                    
                    afferents_arr[t]++;
                }
            }
        }
        
    }
    
    // Remove self connections:
    for (s=0; s<NET_SIZE; s++) {
        for (t=0; t<NET_SIZE; t++) {
            if (s==t) c_net[s][t].is_set = FALSE;
        }
    }
    
    // Prepare delay clusters if needed
    int delay_segments = DELAY_CLUSTERS*2 - 1;
    int delay_diff = MAX_DELAY-MIN_DELAY;
    int cluster_inc = delay_diff/delay_segments;
    int cluster_list[delay_segments];
    // Create list of [MinDel, MinDel+(1*inc),MinDel+(2*inc), ...] of number of connections for delays in the correcponding bounds.
    for (i=0; i<delay_segments; i++) {
        if (i%2 == 0) cluster_list[i] = cluster_inc*i+MIN_DELAY;
        else cluster_list[i] = 0;
    }
    
    // assign delays    
    int linear_delay;
    int delay_ok;
    int max_exc_delay = 0, max_inh_delay = 0;
    double delay;
    int decay_const;
    int del_missing = 0;
    for (i=0; i<NET_SIZE; i++) {
        
        for (j=0; j<NET_SIZE; j++) {
            
            if (OVERRIDE_TOPOLOGY) {
                
                if (DELAY_CLUSTERS>1) {
                    delay_ok = FALSE;
                    while (!delay_ok) {
                        linear_delay = randint(MAX_DELAY-MIN_DELAY)+MIN_DELAY;
                        for (k=0; k<delay_segments; k++) {
                            if (linear_delay >= cluster_list[k] && linear_delay<cluster_list[k]+cluster_inc) {
                                delay_ok = TRUE;
                            }
                        }
                    }
                    
                    //printf("linear_delay: %i\n", linear_delay);
                    
                } else {
                    linear_delay = randint(MAX_DELAY-MIN_DELAY)+MIN_DELAY;
                }
                

            } else {
                
                // compute topological delay: based on distance from 2D matrix size
                
                if (v_net[i].is_excit) decay_const = DELAY_EXCIT_DECAY_CONST;
                else decay_const = DELAY_INHIB_DECAY_CONST;
                
                delay = decay_const * 
                sqrt( pow( (i/SHEET_WIDTH)/(double)SHEET_WIDTH - (j/SHEET_WIDTH)/(double)SHEET_WIDTH, 2 ) + 
                     pow( (i%SHEET_HEIGHT)/(double)SHEET_HEIGHT - (j%SHEET_HEIGHT)/(double)SHEET_HEIGHT, 2 ) 
                     );
                
                linear_delay = (int) delay + MIN_DELAY;
            }            
            
            
            if (c_net[i][j].is_set) {
                if (linear_delay < MAX_DELAY) {
                    c_net[i][j].delay = linear_delay;
                } else { 
                    c_net[i][j].delay = 0; 
                    c_net[i][j].is_set = FALSE;
                    del_missing++;
                }
            }
            
            if (v_net[i].is_excit && linear_delay > max_exc_delay) max_exc_delay = linear_delay;
            if (!v_net[i].is_excit && linear_delay > max_inh_delay) max_inh_delay = linear_delay;
        }
    }
    
    //
    // analyze characteristics
    //
    
    // delay distribution bins
    int delay_bins[MAX_DELAY]; // delay_bins[0] is number of delays of length 0ts etc.
    int delay_node_bins[MAX_DELAY];
    
    // init delay bins
    for (i=0; i<MAX_DELAY; i++) {
        delay_bins[i] = 0;
    }
    
    // get network stats
    int total_connections = 0;
    for (t=0; t<NET_SIZE; t++) {
        
        // init delay_node_bins
        for (k=0; k<MAX_DELAY; k++) {
            delay_node_bins[k] = 0;
        }
        
        for (s=0; s<NET_SIZE; s++) {
            if (c_net[t][s].is_set) {
                total_connections++;
                
                // increase the number of delays found in that bin
                delay_bins[ c_net[t][s].delay ]++;
                delay_node_bins[ c_net[t][s].delay ]++;
            }
        }
        // store the node's incoming connections
        //save_nodeconnections(t);
        
        //save_delaynodedistrib(t, delay_node_bins);
        
        //printf("Afferents[%i]: %i\n", t, afferents_arr[t]);
        
    }
    
    double aff_mean;
    aff_mean = total_connections*1.0/NET_SIZE;
    
    // compute Delay SD
    int delay_list[total_connections];
    double delay_mean = 0.0;
    i=0;
    for (t=0; t<NET_SIZE; t++) {
        for (s=0; s<NET_SIZE; s++) {
            if (c_net[t][s].is_set) {
                delay_list[i] = c_net[t][s].delay;
                i++;
                delay_mean += c_net[t][s].delay;
            }
        }
    }    
    delay_mean /= total_connections*1.0;
    
    for (t=0; t<NET_SIZE; t++) {
        save_nodeconnections( t );
    }
    
    printf("Total afferents: %i\nMissing due to Delay Cutoff: %i\nMean afferents per node: %g, Connections SD: %g\nDelay mean: %g, Delay SD: %g\n", total_connections, del_missing, aff_mean, std_int(afferents_arr, NET_SIZE), delay_mean, std_int(delay_list, total_connections));
    printf("Default_Threshold %i, Max EXC delay in network: %i,  Max INH delay in network: %i\n", DEFAULT_THRESHOLD, max_exc_delay, max_inh_delay);
    
}

/** on_sheet_area(i, x, y, rad)
 * return TRUE if node i is within the area specified by
 * point (x,y) and radius rad on the sheet.
 * requires: get_node_x(i), get_node_y(i)
 */
int on_sheet_area(int i, int x, int y, int rad) {
    
    int j,k;    
    for (j=0; j<SHEET_WIDTH; j++) {
        for (k=0; k<SHEET_HEIGHT; k++) {
            if (get_node_x(i) >= x - rad 
                && get_node_x(i) <= x + rad 
                && get_node_y(i) >= y - rad
                && get_node_y(i) <= y + rad)
                return TRUE;
        }
    }
    
    return FALSE;
}

int get_node_x(int i) {
    return i%SHEET_WIDTH;
}

int get_node_y(int i) {
    return i/SHEET_WIDTH;
}