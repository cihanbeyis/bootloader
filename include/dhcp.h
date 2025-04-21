// #ifndef DHCP_H
// #define DHCP_H

// #include <stdint.h>
// #include "timer.h" // timer_get fonksiyonu için header eklendi
// #include "w5100.h"

// // DHCP States
// #define DHCP_INIT          0
// #define DHCP_DISCOVERING   1
// #define DHCP_REQUESTING    2
// #define DHCP_LEASED        3
// #define DHCP_RENEWING      4
// #define DHCP_REBINDING     5
// #define DHCP_RELEASE_STATE 6
// #define DHCP_FAILED        7

// // DHCP Message Types
// #define DHCP_DISCOVER      1
// #define DHCP_OFFER         2
// #define DHCP_REQUEST       3
// #define DHCP_DECLINE       4
// #define DHCP_ACK          5
// #define DHCP_NAK          6
// #define DHCP_RELEASE_MSG   7

// // DHCP Options
// #define DHCP_OPT_SUBNET_MASK     1
// #define DHCP_OPT_ROUTER          3
// #define DHCP_OPT_DNS_SERVER      6
// #define DHCP_OPT_REQUESTED_IP    50  // REQUEST_IP -> REQUESTED_IP olarak düzeltildi
// #define DHCP_OPT_LEASE_TIME      51
// #define DHCP_OPT_MSG_TYPE        53
// #define DHCP_OPT_SERVER_ID       54
// #define DHCP_OPT_PARAM_REQUEST   55
// #define DHCP_OPT_END            255

// // Function prototypes
// void DHCP_init(uint8_t sock, uint8_t *buf);
// uint8_t DHCP_run(void);
// void DHCP_stop(void);
// uint8_t DHCP_checkLease(void);

// #endif
