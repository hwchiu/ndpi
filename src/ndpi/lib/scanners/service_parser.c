/*
 * service_parser.c
 *
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

#include "ndpi_utils.h"
#include "ahocorasick.h"

static void ndpi_match_service(struct ndpi_detection_module_struct *ndpi_struct, ndpi_automa *automa, struct ndpi_flow_struct *flow,
						char *string_to_match, u_int string_to_match_len) {
  
  int matching_protocol_id = NDPI_RESULT_SERVICE_STILL_UNKNOWN;
  struct ndpi_packet_struct *packet = &flow->packet;
  AC_TEXT_t ac_input_text;

  if ((automa->ac_automa == NULL) || (string_to_match_len == 0)) {
    return;
  }

  if (!automa->ac_automa_finalized) {
    ac_automata_finalize((AC_AUTOMATA_t*)automa->ac_automa);
    automa->ac_automa_finalized = 1;
  }

  ac_input_text.astring = string_to_match, ac_input_text.length = string_to_match_len;
  ac_automata_search (((AC_AUTOMATA_t*)automa->ac_automa), &ac_input_text, (void*)&matching_protocol_id);
  ac_automata_reset(((AC_AUTOMATA_t*)automa->ac_automa));

  if (matching_protocol_id != NDPI_RESULT_SERVICE_STILL_UNKNOWN) {
    flow->ndpi_result_service = matching_protocol_id;
  }
}

void ndpi_search_service_by_ip(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow, u_int32_t saddr, u_int32_t daddr) { /* host endianess */
  
    /*
      Apple (FaceTime, iMessage,...)
      17.0.0.0/8
    */
    if(((saddr & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)
       || ((daddr & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_APPLE;
      return;
    }

    /* 
       Skype
       157.56.0.0/14, 157.60.0.0/16, 157.54.0.0/15
    */
    if(
       (((saddr & 0xFF3F0000 /* 255.63.0.0 */) == 0x9D380000 /* 157.56.0.0/ */) || ((daddr & 0xFF3F0000 /* 255.63.0.0 */) == 0x9D380000))
       || (((saddr & 0xFFFF0000 /* 255.255.0.0 */) == 0x9D3C0000 /* 157.60.0.0/ */) || ((daddr & 0xFFFF0000 /* 255.255.0.0 */) == 0x9D3D0000))
       || (((saddr & 0xFF7F0000 /* 255.255.0.0 */) == 0x9D360000 /* 157.54.0.0/ */) || ((daddr & 0xFF7F0000 /* 255.127.0.0 */) == 0x9D360000))
       || (((saddr & 0xFFFE0000 /* 255.254.0.0 */) == 0x9D360000 /* 157.54.0.0/ */) || ((daddr & 0xFFFE0000 /* 255.254.0.0 */) == 0x9D360000))
       ) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_SKYPE;
      return;
    }
  
    /*
      Google
      173.194.0.0/16
    */
    if(((saddr & 0xFFFF0000 /* 255.255.0.0 */) == 0xADC20000  /* 173.194.0.0 */)
       || ((daddr & 0xFFFF0000 /* 255.255.0.0 */) ==0xADC20000 /* 173.194.0.0 */)) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_GOOGLE;
      return;
    }
    
    /* 
       Twitter Inc.
    */
    
    // 192.133.76.0/22
    /* 192.133.76.0 - 192.133.79.255 */
    if(((saddr >= 3229961216) && (saddr <= 3229962239))
       || ((daddr >= 3229961216) && (daddr <= 3229962239))
       || ((saddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0x5C854C00/* 92.133.76.0 */)
       || ((daddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0x5C854C00/* 92.133.76.0 */)
       ) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_TWITTER;
      return;
    }
    // 199.16.156.0/22
    /* 199.16.156.0 - 199.16.159.255 */
    if(((saddr >= 3339754496) && (saddr <= 3339755519))
       || ((daddr >= 3339754496) && (daddr <= 3339755519))
       || ((saddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0xC7109C00/* 199.16.156.0 */)
       || ((daddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0xC7109C00/* 199.16.156.0 */)
       ) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_TWITTER;
      return;
    }

     // 199.59.148.0/22
    /* 199.59.148.0 - 199.59.151.255 */
    if(((saddr >= 3342570496) && (saddr <= 3342571519))
       || ((daddr >= 3342570496) && (daddr <= 3342571519))
       || ((saddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0xC73B9400/* 199.59.148.0 */)
       || ((daddr & 0xFFFFFC00 /* 255.255.252.0  */) == 0xC73B9400/* 199.59.148.0 */)
       ) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_TWITTER;
      return;
    }

     // 199.96.56.0 /21
    /* 199.96.56.0 - 199.96.63.255 */
    if(((saddr >= 3344971776) && (saddr <= 3344973823))
       || ((daddr >= 3344971776) && (daddr <= 3344973823))
       || ((saddr & 0xFFFFF800 /* 255.255.248.0  */) == 0xC7603800/* 199.96.56.0 */)
       || ((daddr & 0xFFFFF800 /* 255.255.248.0  */) == 0xC7603800/* 199.96.56.0 */)
       ) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_TWITTER;
      return;
    }
    
    /* 
       CIDR:           69.53.224.0/19
       OriginAS:       AS2906
       NetName:        NETFLIX-INC
    */
    if(((saddr & 0xFFFFE000 /* 255.255.224.0 */) == 0x4535E000 /* 69.53.224.0 */)
       || ((daddr & 0xFFFFE000 /* 255.255.224.0 */) == 0x4535E000 /* 69.53.224.0 */)) {
      flow->ndpi_result_service = NDPI_RESULT_SERVICE_NETFLIX;
      return;
    }    
}

void ndpi_search_service(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = &flow->packet;
  
  if (flow->ndpi_result_service != NDPI_RESULT_SERVICE_STILL_UNKNOWN) {
    return;
  }
  
  if (((char *)flow->host_server_name) != NULL && strlen((const char*)flow->host_server_name) != 0) {
    ndpi_match_service(ndpi_struct, &ndpi_struct->service_automa, flow, (char *)flow->host_server_name, strlen((const char*)flow->host_server_name));
  }
  
  if (flow->ndpi_result_service != NDPI_RESULT_SERVICE_STILL_UNKNOWN) {
    return;
  }
  
  if (flow->certificate != NULL && strlen(flow->certificate) != 0) {
    ndpi_match_service(ndpi_struct, &ndpi_struct->service_automa, flow, flow->certificate, strlen(flow->certificate));
  }
  
  if (flow->ndpi_result_service != NDPI_RESULT_SERVICE_STILL_UNKNOWN) {
    return;
  }  
  
  if (packet->iph /* IPv4 Only: we need to support packet->iphv6 at some point. */) {
    ndpi_search_service_by_ip(ndpi_struct, flow, ntohl(packet->iph->saddr), ntohl(packet->iph->daddr));
  }
	
  /* Break after 10 packets. */
  if ((flow->ndpi_result_content == NDPI_RESULT_CONTENT_STILL_UNKNOWN) && (flow->packet_counter > 20)) {
    NDPI_LOG(0, ndpi_struct, NDPI_LOG_DEBUG, "Could not find any HTTP content.\n");
    flow->ndpi_result_content = NDPI_RESULT_CONTENT_UNKNOWN;
    return;
  }
}

static int ac_match_service_handler(AC_MATCH_t *m, void *param) {
  
  int *matching_protocol_id = (int*)param;

  /* Stopping to the first match. We might consider searching
   * for the more specific match, paying more cpu cycles. */
  *matching_protocol_id = m->patterns[0].rep.number;

  return 1; /* 0 to continue searching, !0 to stop */
}

static int service_to_automa(struct ndpi_detection_module_struct *ndpi_struct, ndpi_automa *automa, char *value, int protocol_id) {
  AC_PATTERN_t ac_pattern;
  ac_pattern.astring = value;
  ac_pattern.rep.number = protocol_id;
  ac_pattern.length = strlen(ac_pattern.astring);
  ac_automata_add(((AC_AUTOMATA_t*)automa->ac_automa), &ac_pattern);
}

void ndpi_register_service_parser (struct ndpi_detection_module_struct *ndpi_mod) {
  
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_FACEBOOK, "Facebook", ndpi_search_service);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_TWITTER, "Twitter", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_YOUTUBE, "YouTube", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_GOOGLE, "Google", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_NETFLIX, "Netflix", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_LASTFM, "LastFM", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_GROOVESHARK, "Grooveshark", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_APPLE, "Apple", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_WHATSAPP, "WhatsApp", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_APPLE_ICLOUD, "Apple_iCloud", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_APPLE_ITUNES, "Apple_iTunes", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_TUENTI, "Tuenti", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_WIKIPEDIA, "Wikipedia", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_MSN, "MSN", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_AMAZON, "Amazon", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_EBAY, "eBay", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_CNN, "CNN", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_DROPBOX, "Dropbox", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_SKYPE, "Skype", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_VIBER, "Viber", NULL);
  ndpi_initialize_scanner_service (ndpi_mod, NDPI_RESULT_SERVICE_YAHOO, "Yahoo", NULL);
  
  ndpi_mod->service_automa.ac_automa = ac_automata_init(ac_match_service_handler);
  
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "amazon.com", NDPI_RESULT_SERVICE_AMAZON);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "amazonaws.com", NDPI_RESULT_SERVICE_AMAZON);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "amazon-adsystem.com", NDPI_RESULT_SERVICE_AMAZON);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".apple.com", NDPI_RESULT_SERVICE_APPLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".mzstatic.com", NDPI_RESULT_SERVICE_APPLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".icloud.com", NDPI_RESULT_SERVICE_APPLE_ICLOUD);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "itunes.apple.com", NDPI_RESULT_SERVICE_APPLE_ITUNES);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".cnn.c", NDPI_RESULT_SERVICE_CNN);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".cnn.net", NDPI_RESULT_SERVICE_CNN);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".dropbox.com", NDPI_RESULT_SERVICE_DROPBOX);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".ebay.com", NDPI_RESULT_SERVICE_EBAY);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".ebaystatic.com", NDPI_RESULT_SERVICE_EBAY);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".ebaydesc.com", NDPI_RESULT_SERVICE_EBAY);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".ebayrtm.com", NDPI_RESULT_SERVICE_EBAY);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".facebook.com", NDPI_RESULT_SERVICE_FACEBOOK);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".fbcdn.net", NDPI_RESULT_SERVICE_FACEBOOK);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "fbcdn-", NDPI_RESULT_SERVICE_FACEBOOK);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".gstatic.com", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".googlesyndication.com", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".googletagservices.com", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".2mdn.net", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".doubleclick.net", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "googleads.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "google-analytics.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "googleusercontent.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "googleadservices.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "googleapis.com", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".google.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".gmail.", NDPI_RESULT_SERVICE_GOOGLE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".grooveshark.com", NDPI_RESULT_SERVICE_GROOVESHARK);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".last.fm", NDPI_RESULT_SERVICE_LASTFM);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "msn.com", NDPI_RESULT_SERVICE_MSN);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".netflix.com", NDPI_RESULT_SERVICE_NETFLIX);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".skype.com", NDPI_RESULT_SERVICE_SKYPE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".skypeassets.com", NDPI_RESULT_SERVICE_SKYPE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".tuenti.com", NDPI_RESULT_SERVICE_TUENTI);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".twttr.com", NDPI_RESULT_SERVICE_TWITTER);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "twitter.", NDPI_RESULT_SERVICE_TWITTER);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "twimg.com", NDPI_RESULT_SERVICE_TWITTER);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".viber.com", NDPI_RESULT_SERVICE_VIBER);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "wikipedia.", NDPI_RESULT_SERVICE_WIKIPEDIA);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "wikimedia.", NDPI_RESULT_SERVICE_WIKIPEDIA);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "mediawiki.", NDPI_RESULT_SERVICE_WIKIPEDIA);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "wikimediafoundation.", NDPI_RESULT_SERVICE_WIKIPEDIA);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".whatsapp.net", NDPI_RESULT_SERVICE_WHATSAPP);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".yahoo.", NDPI_RESULT_SERVICE_YAHOO);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "yimg.com", NDPI_RESULT_SERVICE_YAHOO);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "yahooapis.", NDPI_RESULT_SERVICE_YAHOO);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "youtube.", NDPI_RESULT_SERVICE_YOUTUBE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".googlevideo.com", NDPI_RESULT_SERVICE_YOUTUBE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, ".ytimg.com", NDPI_RESULT_SERVICE_YOUTUBE);
  service_to_automa(ndpi_mod, &ndpi_mod->service_automa, "youtube-nocookie.", NDPI_RESULT_SERVICE_YOUTUBE);
}

void ndpi_unregister_service_parser (struct ndpi_detection_module_struct *ndpi_mod) {
  
  if(ndpi_mod->service_automa.ac_automa != NULL) {
      ac_automata_release((AC_AUTOMATA_t*)ndpi_mod->service_automa.ac_automa);
  }
}
