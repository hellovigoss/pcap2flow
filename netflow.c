

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "time.h"
#include "netflow.h"

static unsigned int total_flows;

void nf_init_peer(nf_peer_t *nf_peer, struct in_addr *peer_ip, unsigned short peer_port) {
  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket < 0) {
    fprintf(stderr, "Failed to request socket\n");
    exit(-1);
  }
  nf_peer->socket = s;
  nf_peer->sockaddr.sin_family = AF_INET;
  nf_peer->sockaddr.sin_port = htons(peer_port);
  memcpy(&nf_peer->sockaddr.sin_addr, peer_ip, sizeof(struct in_addr));
  /*if (bind(nf_peer->socket, (struct sockaddr *)&nf_peer->sockaddr, sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Failed to bind socket to port: %s\n", strerror(errno));
    exit(-1);
  }*/
  total_flows = 0;
}

void nf_export(nf_peer_t *nf_peer, nf_v5_packet_t *packet, unsigned int num_records) {
  unsigned char *p = (char *)packet;
  int i;

  packet->header.version = htons(5);
  packet->header.count = htons(num_records);
  packet->header.sys_uptime = htonl(time_sysuptime());
  packet->header.ts_sec = htonl(time_epoch_sec());
  packet->header.ts_msec = htonl(time_epoch_msec());
  packet->header.sequence = htonl(total_flows);
  packet->header.engine_type = 0;
  packet->header.engine_id = 0;
  packet->header.sampling_interval = 0;

  int error = sendto( nf_peer->socket, packet, 
                      (sizeof(nf_v5_header_t) + sizeof(nf_v5_record_t) * num_records), 
                      0, 
                      (struct sockaddr *)&nf_peer->sockaddr, sizeof(struct sockaddr_in)
                    );
        
  if (error < 0) {
    printf("Error exporting %d records to fd %d: %s\n", num_records, nf_peer->socket, strerror(errno));
  }
  total_flows += num_records;
}
