/*
 * proto_winmx.c
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

void ndpi_search_winmx(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow) {
  
	struct ndpi_packet_struct *packet = &flow->packet;
	
	if (flow->l4.tcp.winmx_stage == 0) {
		if (packet->payload_packet_len == 1 || (packet->payload_packet_len > 1 && packet->payload[0] == 0x31)) {
			return;
		}
		
		/* did not see this pattern in any trace that we have */
		if (((packet->payload_packet_len) == 4)
			&& (memcmp(packet->payload, "SEND", 4) == 0)) {

			NDPI_LOG(0MX, ndpi_struct, NDPI_LOG_DEBUG, "maybe WinMX Send\n");
			flow->l4.tcp.winmx_stage = 1;
			return;
		}

		if (((packet->payload_packet_len) == 3)
			&& (memcmp(packet->payload, "GET", 3) == 0)) {
			NDPI_LOG(0MX, ndpi_struct, NDPI_LOG_DEBUG, "found winmx by GET\n");
			flow->ndpi_result_app = NDPI_RESULT_APP_WINMX;
			flow->ndpi_excluded_app[NDPI_RESULT_APP_WINMX] = 1;
			return;
		}

		if (packet->payload_packet_len == 149 && packet->payload[0] == '8') {
			NDPI_LOG(0MX, ndpi_struct, NDPI_LOG_DEBUG, "maybe WinMX\n");
			if (get_u_int32_t(packet->payload, 17) == 0
				&& get_u_int32_t(packet->payload, 21) == 0
				&& get_u_int32_t(packet->payload, 25) == 0
				&& get_u_int16_t(packet->payload, 39) == 0 && get_u_int16_t(packet->payload, 135) == htons(0x7edf)
				&& get_u_int16_t(packet->payload, 147) == htons(0xf792)) {

				NDPI_LOG(0MX, ndpi_struct, NDPI_LOG_DEBUG, "found winmx by pattern in first packet\n");
				flow->ndpi_result_app = NDPI_RESULT_APP_WINMX;
				flow->ndpi_excluded_app[NDPI_RESULT_APP_WINMX] = 1;
				return;
			}
		}
		/* did not see this pattern in any trace that we have */
	} else if (flow->l4.tcp.winmx_stage == 1) {
	  
		if (packet->payload_packet_len > 10 && packet->payload_packet_len < 1000) {
		  
			u_int16_t left = packet->payload_packet_len - 1;
			
			while (left > 0) {
				if (packet->payload[left] == ' ') {
					NDPI_LOG(0MX, ndpi_struct, NDPI_LOG_DEBUG, "found winmx in second packet\n");
					flow->ndpi_result_app = NDPI_RESULT_APP_WINMX;
					flow->ndpi_excluded_app[NDPI_RESULT_APP_WINMX] = 1;
					return;
				} else if (packet->payload[left] < '0' || packet->payload[left] > '9') {
					break;
				}
				
				left--;
			}
		}
	}

	flow->ndpi_excluded_app[NDPI_RESULT_APP_WINMX] = 1;
}

void ndpi_register_proto_winmx (struct ndpi_detection_module_struct *ndpi_mod) {

  int tcp_ports[5] = {0, 0, 0, 0, 0};
  int udp_ports[5] = {0, 0, 0, 0, 0};

  ndpi_initialize_scanner_app (ndpi_mod, NDPI_RESULT_APP_WINMX, "WinMX", NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION, tcp_ports, udp_ports, ndpi_search_winmx);
}