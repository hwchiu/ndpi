/*
 * proto_tftp.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
 * Copyright (C) 2014 Tomasz Bujlow <tomasz@skatnet.dk>
 *
 * This file is part of nDPIng, an open source deep packet inspection
 * library based on nDPI, OpenDPI, and PACE technology by ipoque GmbH
 *
 * nDPIng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPIng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPIng.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "ndpi_api.h"

void ndpi_search_tftp(struct ndpi_detection_module_struct
						*ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;
	NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "search TFTP.\n");

	if (packet->payload_packet_len > 3 && flow->l4.udp.tftp_stage == 0
		&& ntohl(get_u_int32_t(packet->payload, 0)) == 0x00030001) {
		NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "maybe tftp. need next packet.\n");
		flow->l4.udp.tftp_stage = 1;
		return;
	}
	
	if (packet->payload_packet_len > 3 && (flow->l4.udp.tftp_stage == 1)
		&& ntohl(get_u_int32_t(packet->payload, 0)) == 0x00040001) {

		NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "found tftp.\n");
		flow->ndpi_result_app = NDPI_RESULT_APP_TFTP;
		flow->ndpi_excluded_app[NDPI_RESULT_APP_TFTP] = 1;
		return;
	}
	
	if (packet->payload_packet_len > 1
		&& ((packet->payload[0] == 0 && packet->payload[packet->payload_packet_len - 1] == 0)
			|| (packet->payload_packet_len == 4 && ntohl(get_u_int32_t(packet->payload, 0)) == 0x00040000))) {
		NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "skip initial packet.\n");
		return;
	}

	NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "exclude TFTP.\n");
	flow->ndpi_excluded_app[NDPI_RESULT_APP_TFTP] = 1;
}

void ndpi_register_proto_tftp (struct ndpi_detection_module_struct *ndpi_mod) {

  int tcp_ports[5] = {0, 0, 0, 0, 0};
  int udp_ports[5] = {0, 0, 0, 0, 0};

  ndpi_initialize_scanner_app (ndpi_mod, NDPI_RESULT_APP_TFTP, "TFTP", NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD, tcp_ports, udp_ports, ndpi_search_tftp);
}