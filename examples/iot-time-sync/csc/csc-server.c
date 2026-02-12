/**
 * \file
 *         A server for CSC experiments.
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <stdio.h>
#include <string.h> // for memcpy()
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "sys/rtimer.h"
#include "csc-data.h"

#define LOG_MODULE "CSC-Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define BEACON_INTERVAL (1 * CLOCK_SECOND) // 1 <= RTIMER_CLOCK_MAX/RTIMER_SECOND (=2 for TelosB)

PROCESS(csc_server_process, "A server for CSC experiments");
struct process *p2_ext_process = &csc_server_process; // for the P2 extension
AUTOSTART_PROCESSES(&csc_server_process);

PROCESS_THREAD(csc_server_process, ev, data)
{
  static struct etimer periodic_timer;
  static csc_data_t nn_data = {0, 0};

  PROCESS_BEGIN();

  // initialize NullNet
  nullnet_buf = (csc_data_t *)&nn_data;
  nullnet_len = sizeof(nn_data);
  
  etimer_set(&periodic_timer, BEACON_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    memcpy(nullnet_buf, &nn_data, sizeof(nn_data));
    nullnet_len = sizeof(nn_data);
#ifdef RTIMER_EXT
    nn_data.timestamp = RTIMER32_NOW();
#else
    nn_data.timestamp = RTIMER_NOW();
#endif
    NETSTACK_NETWORK.output(NULL);

#ifdef RTIMER_EXT
    LOG_INFO("Send a beacon with seq_num=%lu, timestamp=%lu\n",
      (unsigned long)nn_data.seq_num, (unsigned long)nn_data.timestamp);
#else
    LOG_INFO("Send a beacon with seq_num=%lu, timestamp=%u\n",
      (unsigned long)nn_data.seq_num, (unsigned)nn_data.timestamp);
#endif
    /* LOG_INFO_LLADDR(NULL); */
    /* LOG_INFO_("\n"); */

    nn_data.seq_num++;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}