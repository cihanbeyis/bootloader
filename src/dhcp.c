// #include "dhcp.h"
// #include "time.h"
// #include <string.h>
// #include <util/delay.h>

// #define DHCP_TIMEOUT      5000  // 5 saniye timeout
// #define DHCP_RETRY_COUNT  3     // Maksimum deneme sayısı

// static uint8_t dhcp_buffer[548];
// static uint8_t dhcp_state = DHCP_INIT;
// static uint8_t dhcp_sock;
// static uint32_t dhcp_lease_time = 0;
// static uint32_t dhcp_last_time = 0;
// static uint8_t dhcp_server_ip[4];
// static uint32_t xid;
// static uint32_t retry_count = 0;
// static uint32_t discover_start_time;

// void DHCP_init(uint8_t sock, uint8_t *buf) {
//     dhcp_sock = sock;
//     dhcp_state = DHCP_INIT;
//     xid = 0x12345678;  // Transaction ID
//     retry_count = 0;
// }

// static void send_discover(void) {
//     memset(dhcp_buffer, 0, 548);
    
//     // DHCP Message Structure
//     dhcp_buffer[0] = 0x01;  // Message type: Boot Request
//     dhcp_buffer[1] = 0x01;  // Hardware type: Ethernet
//     dhcp_buffer[2] = 0x06;  // Hardware address length
//     dhcp_buffer[3] = 0x00;  // Hops
    
//     // Transaction ID
//     dhcp_buffer[4] = (xid >> 24) & 0xFF;
//     dhcp_buffer[5] = (xid >> 16) & 0xFF;
//     dhcp_buffer[6] = (xid >> 8) & 0xFF;
//     dhcp_buffer[7] = xid & 0xFF;
    
//     // Client MAC address
//     memcpy(&dhcp_buffer[28], net_info.mac, 6);
    
//     // Magic Cookie
//     dhcp_buffer[236] = 99;
//     dhcp_buffer[237] = 130;
//     dhcp_buffer[238] = 83;
//     dhcp_buffer[239] = 99;
    
//     // DHCP Message Type = DISCOVER
//     dhcp_buffer[240] = DHCP_OPT_MSG_TYPE;
//     dhcp_buffer[241] = 1;
//     dhcp_buffer[242] = DHCP_DISCOVER;
    
//     // Parameter Request List
//     dhcp_buffer[243] = DHCP_OPT_PARAM_REQUEST;
//     dhcp_buffer[244] = 3;
//     dhcp_buffer[245] = DHCP_OPT_SUBNET_MASK;
//     dhcp_buffer[246] = DHCP_OPT_ROUTER;
//     dhcp_buffer[247] = DHCP_OPT_DNS_SERVER;
    
//     // End Option
//     dhcp_buffer[248] = DHCP_OPT_END;
    
//     // Send DISCOVER message
//     W5100_socket(dhcp_sock, Sn_MR_UDP, 68);
//     W5100_send(dhcp_sock, dhcp_buffer, 300);
// }

// static void send_request(void) {
//     memset(dhcp_buffer, 0, 548);
    
//     // DHCP Message Structure
//     dhcp_buffer[0] = 0x01;  // Message type: Boot Request
//     dhcp_buffer[1] = 0x01;  // Hardware type: Ethernet
//     dhcp_buffer[2] = 0x06;  // Hardware address length
//     dhcp_buffer[3] = 0x00;  // Hops
    
//     // Transaction ID
//     dhcp_buffer[4] = (xid >> 24) & 0xFF;
//     dhcp_buffer[5] = (xid >> 16) & 0xFF;
//     dhcp_buffer[6] = (xid >> 8) & 0xFF;
//     dhcp_buffer[7] = xid & 0xFF;
    
//     // Client MAC address
//     memcpy(&dhcp_buffer[28], net_info.mac, 6);
    
//     // Magic Cookie
//     dhcp_buffer[236] = 99;
//     dhcp_buffer[237] = 130;
//     dhcp_buffer[238] = 83;
//     dhcp_buffer[239] = 99;
    
//     // DHCP Message Type = REQUEST
//     dhcp_buffer[240] = DHCP_OPT_MSG_TYPE;
//     dhcp_buffer[241] = 1;
//     dhcp_buffer[242] = DHCP_REQUEST;
    
//     // Requested IP Address
//     dhcp_buffer[243] = DHCP_OPT_REQUESTED_IP;
//     dhcp_buffer[244] = 4;
//     memcpy(&dhcp_buffer[245], net_info.ip, 4);
    
//     // DHCP Server Identifier
//     dhcp_buffer[249] = DHCP_OPT_SERVER_ID;
//     dhcp_buffer[250] = 4;
//     memcpy(&dhcp_buffer[251], dhcp_server_ip, 4);
    
//     // Parameter Request List
//     dhcp_buffer[255] = DHCP_OPT_PARAM_REQUEST;
//     dhcp_buffer[256] = 3;
//     dhcp_buffer[257] = DHCP_OPT_SUBNET_MASK;
//     dhcp_buffer[258] = DHCP_OPT_ROUTER;
//     dhcp_buffer[259] = DHCP_OPT_DNS_SERVER;
    
//     // End Option
//     dhcp_buffer[260] = DHCP_OPT_END;
    
//     // Send REQUEST message
//     W5100_send(dhcp_sock, dhcp_buffer, 300);
// }

// static uint8_t process_offer(void) {
//     uint16_t len = W5100_getRXReceivedSize(dhcp_sock);
//     if(len > 0) {
//         W5100_recv(dhcp_sock, dhcp_buffer, len);
        
//         // Verify transaction ID
//         uint32_t recv_xid = ((uint32_t)dhcp_buffer[4] << 24) | 
//                            ((uint32_t)dhcp_buffer[5] << 16) |
//                            ((uint32_t)dhcp_buffer[6] << 8) |
//                            (uint32_t)dhcp_buffer[7];
        
//         if(recv_xid == xid) {
//             // Store offered IP
//             memcpy(net_info.ip, &dhcp_buffer[16], 4);
            
//             // Find DHCP message type and server ID
//             uint16_t opts_ptr = 240;
//             uint8_t is_offer = 0;
//             uint8_t has_server_id = 0;
            
//             while(opts_ptr < len) {
//                 if(dhcp_buffer[opts_ptr] == DHCP_OPT_MSG_TYPE && 
//                    dhcp_buffer[opts_ptr+2] == DHCP_OFFER) {
//                     is_offer = 1;
//                 }
//                 else if(dhcp_buffer[opts_ptr] == DHCP_OPT_SERVER_ID) {
//                     memcpy(dhcp_server_ip, &dhcp_buffer[opts_ptr+2], 4);
//                     has_server_id = 1;
//                 }
                
//                 if(is_offer && has_server_id) return 1;
                
//                 if(dhcp_buffer[opts_ptr] == DHCP_OPT_END) break;
//                 opts_ptr += dhcp_buffer[opts_ptr+1] + 2;
//             }
//         }
//     }
//     return 0;
// }

// static uint8_t process_ack(void) {
//     uint16_t len = W5100_getRXReceivedSize(dhcp_sock);
//     if(len > 0) {
//         W5100_recv(dhcp_sock, dhcp_buffer, len);
        
//         // Verify transaction ID
//         uint32_t recv_xid = ((uint32_t)dhcp_buffer[4] << 24) | 
//                            ((uint32_t)dhcp_buffer[5] << 16) |
//                            ((uint32_t)dhcp_buffer[6] << 8) |
//                            (uint32_t)dhcp_buffer[7];
        
//         if(recv_xid == xid) {
//             uint16_t opts_ptr = 240;
//             while(opts_ptr < len) {
//                 if(dhcp_buffer[opts_ptr] == DHCP_OPT_MSG_TYPE && 
//                    dhcp_buffer[opts_ptr+2] == DHCP_ACK) {
                    
//                     // IP adresi ve lease time'ı kaydet
//                     memcpy(net_info.ip, &dhcp_buffer[16], 4);
                    
//                     // Lease time'ı ara ve kaydet
//                     opts_ptr = 240;
//                     while(opts_ptr < len) {
//                         if(dhcp_buffer[opts_ptr] == DHCP_OPT_LEASE_TIME) {
//                             dhcp_lease_time = ((uint32_t)dhcp_buffer[opts_ptr+2] << 24) |
//                                             ((uint32_t)dhcp_buffer[opts_ptr+3] << 16) |
//                                             ((uint32_t)dhcp_buffer[opts_ptr+4] << 8) |
//                                             (uint32_t)dhcp_buffer[opts_ptr+5];
//                             return 1;
//                         }
//                         if(dhcp_buffer[opts_ptr] == DHCP_OPT_END) break;
//                         opts_ptr += dhcp_buffer[opts_ptr+1] + 2;
//                     }
//                 }
//                 if(dhcp_buffer[opts_ptr] == DHCP_OPT_END) break;
//                 opts_ptr += dhcp_buffer[opts_ptr+1] + 2;
//             }
//         }
//     }
//     return 0;
// }

// uint8_t DHCP_run(void) {
//     switch(dhcp_state) {
//         case DHCP_INIT:
//             send_discover();
//             discover_start_time = timer_get();
//             dhcp_state = DHCP_DISCOVERING;
//             break;
            
//         case DHCP_DISCOVERING:
//             if(process_offer()) {
//                 send_request();
//                 discover_start_time = timer_get();
//                 dhcp_state = DHCP_REQUESTING;
//             }
//             else if((timer_get() - discover_start_time) > DHCP_TIMEOUT) {
//                 if(++retry_count >= DHCP_RETRY_COUNT) {
//                     dhcp_state = DHCP_FAILED;
//                     retry_count = 0;
//                 }
//                 else {
//                     dhcp_state = DHCP_INIT;
//                 }
//             }
//             break;
            
//         case DHCP_REQUESTING:
//             if(process_ack()) {
//                 dhcp_state = DHCP_LEASED;
//                 dhcp_last_time = timer_get();
//                 retry_count = 0;
//             }
//             else if((timer_get() - discover_start_time) > DHCP_TIMEOUT) {
//                 if(++retry_count >= DHCP_RETRY_COUNT) {
//                     dhcp_state = DHCP_FAILED;
//                     retry_count = 0;
//                 }
//                 else {
//                     dhcp_state = DHCP_INIT;
//                 }
//             }
//             break;
            
//         case DHCP_LEASED:
//             if(DHCP_checkLease()) {
//                 dhcp_state = DHCP_RENEWING;
//                 send_request();
//                 discover_start_time = timer_get();
//             }
//             break;
            
//         case DHCP_RENEWING:
//             if(process_ack()) {
//                 dhcp_state = DHCP_LEASED;
//                 dhcp_last_time = timer_get();
//             }
//             else if((timer_get() - discover_start_time) > DHCP_TIMEOUT) {
//                 dhcp_state = DHCP_REBINDING;
//                 send_discover();
//                 discover_start_time = timer_get();
//             }
//             break;
            
//         case DHCP_REBINDING:
//             if(process_offer()) {
//                 send_request();
//                 discover_start_time = timer_get();
//                 dhcp_state = DHCP_REQUESTING;
//             }
//             else if((timer_get() - discover_start_time) > DHCP_TIMEOUT) {
//                 dhcp_state = DHCP_INIT;
//             }
//             break;
            
//         case DHCP_FAILED:
//             // DHCP başarısız oldu, kullanıcı müdahalesi gerekebilir
//             break;
//     }
    
//     return dhcp_state;
// }

// void DHCP_stop(void) {
//     W5100_close(dhcp_sock);
//     dhcp_state = DHCP_INIT;
// }

// uint8_t DHCP_checkLease(void) {
//     uint32_t current_time = timer_get();
//     if((current_time - dhcp_last_time) > dhcp_lease_time) {
//         return 1;
//     }
//     return 0;
// }
