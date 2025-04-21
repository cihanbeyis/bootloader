// eeprom.h
#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

// EEPROM Memory Map
#define EEPROM_MAC_ADDR         0   // 6 bytes
#define EEPROM_DEVICE_IP        6   // 4 bytes
#define EEPROM_TFTP_SERVER_IP   10  // 4 bytes
#define EEPROM_GATEWAY          14  // 4 bytes
#define EEPROM_SUBNET_MASK      18  // 4 bytes
#define EEPROM_DEVICE_NAME      22  // 12 bytes
#define EEPROM_VERSION          34  // 12 bytes
#define EEPROM_UPDATE_FLAG      127  // 1 byte

// Default values
#define DEFAULT_MAC             {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01}
#define DEFAULT_DEVICE_IP       {192, 168, 1, 207}
#define DEFAULT_TFTP_SERVER_IP  {192, 168, 1, 208}
#define DEFAULT_GATEWAY         {192, 168, 1, 1}
#define DEFAULT_SUBNET_MASK     {255, 255, 255, 0}
#define DEFAULT_DEVICE_NAME     "NONAME"
#define DEFAULT_VERSION         "0.0.0"
#define DEFAULT_UPDATE_FLAG     0xAA

void eeprom_init(void);
void eeprom_read_mac(uint8_t* mac);
void eeprom_read_device_ip(uint8_t* ip);
void eeprom_read_tftp_server_ip(uint8_t* ip);
void eeprom_read_gateway(uint8_t* ip);
void eeprom_read_subnet_mask(uint8_t* mask);
void eeprom_read_device_name(char* name);
void eeprom_read_version(char* version);
uint8_t eeprom_read_update_flag(void);
void eeprom_write_update_flag(uint8_t flag);

#endif
