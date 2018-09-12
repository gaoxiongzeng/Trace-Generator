/*
    The code generates the output_flow_file based on flow_cdf_file
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cdf.h"

/* Basic flow settings */
int    host_num                  = 8; /* number of hosts */
int    flow_total_num            = 10000; /* total number of flows to generate */
int    flow_total_time           = 0; /* total time to generate requests (in seconds) */
int    load                      = 4000; /* average network load in Mbps per host */
int    incast                    = 0; /* all-to-one when set to 1 */
struct cdf_table *flow_size_dist = NULL; /* flow distribution table*/
char   flow_cdf_file[100]        = "cdf/dctcp.cdf"; /* flow size distribution file */
int    header_size               = 54;
int    max_ether_size            = 1500;

/* IP address configuration */
const char * const host_ip[] = {
    "192 168 111 1",
    "192 168 111 2",
    "192 168 111 3",
    "192 168 111 4",
    "192 168 111 5",
    "192 168 111 6",
    "192 168 111 7",
    "192 168 111 8"
};

/* Port usage (port id used) */
int host_port_offset[] = {
    20000,
    20000,
    20000,
    20000,
    20000,
    20000,
    20000,
    20000
};

/* Get the next available port id of the host */
static int get_host_next_port(int host) 
{
    host_port_offset[host]++;
    return (host_port_offset[host]);
}

/* Generate poission process arrival interval */
double poission_gen_interval(double avg_rate)
{
    if (avg_rate > 0)
        return -logf(1.0 - (double)(rand() % RAND_MAX) / RAND_MAX) / avg_rate;
    else
        return 0;
}

int main(void) 
{
    FILE   *output_flow_file = NULL;
    char   output_filename[100] = "trace_file/output.trace";
    int    flow_id = 0;
    int    flow_size = 0;
    double flow_start_time = 0.0; /* in second */
    int    max_payload_size = max_ether_size - header_size;
    double period_us;

    flow_size_dist = (struct cdf_table*)malloc(sizeof(struct cdf_table));
    init_cdf(flow_size_dist);
    load_cdf(flow_size_dist, flow_cdf_file);

    /* Average request arrival interval (in microsecond) */
    period_us = (avg_cdf(flow_size_dist)*8.0/max_payload_size*max_ether_size) / (host_num*load); 

    printf("host_num        %d \n", host_num);
    printf("flow_total_num  %d \n", flow_total_num);
    printf("flow_total_time %d \n", flow_total_time);
    printf("load            %d \n", load);
    printf("avg_flowsize    %f \n", avg_cdf(flow_size_dist));
    printf("period_us       %f \n", period_us);

    /* Convert flow_total_time to flow_total_num */
    if (flow_total_num == 0 && flow_total_time > 0)
        flow_total_num = flow_total_time * 1000000 / period_us;

    /* Set random seed */
    srand(754); 

    /* Generate traffic flows */
    for (flow_id=host_num; flow_id<flow_total_num; flow_id++) {
        int src_host = rand() % host_num;
        int dst_host = rand() % host_num;

        /* Skip if the src_host and dst_host are the same */
        while (src_host == dst_host)
            dst_host = rand() % host_num;

        /* Assign flow size and start time */
        flow_size = gen_random_cdf(flow_size_dist);
        flow_start_time = flow_start_time + poission_gen_interval(1.0 / period_us) / 1000000;

        /* Incast: only accept dst_host = 0 */
        if (incast && dst_host != 0) {
            flow_id--;
            continue;
        }

        /* Write to output file */
        output_flow_file = fopen(output_filename, "a");
        fprintf(output_flow_file, "%d %s %s %d %d %d %.9f\n",
            flow_id,
            host_ip[src_host],
            host_ip[dst_host],
            get_host_next_port(src_host),
            get_host_next_port(dst_host),
            flow_size,
            flow_start_time);
        fclose(output_flow_file);
    }

    return 0;
}