// eeprom.c
#include <avr/eeprom.h>
#include <string.h>
#include "eeprom.h"

static uint8_t is_empty(const uint8_t* addr, uint8_t len) {
    for(uint8_t i = 0; i < len; i++) {
        if(eeprom_read_byte(addr + i) != 0xFF) {
            return 0;
        }
    }
    return 1;
}

void eeprom_init(void) {
    // MAC
    if(is_empty((uint8_t*)EEPROM_MAC_ADDR, 6)) {
        uint8_t default_mac[] = DEFAULT_MAC;
        eeprom_write_block(default_mac, (void*)EEPROM_MAC_ADDR, 6);
    }

    // Device IP
    if(is_empty((uint8_t*)EEPROM_DEVICE_IP, 4)) {
        uint8_t default_device_ip[] = DEFAULT_DEVICE_IP;
        eeprom_write_block(default_device_ip, (void*)EEPROM_DEVICE_IP, 4);
    }

    // TFTP Server IP
    if(is_empty((uint8_t*)EEPROM_TFTP_SERVER_IP, 4)) {
        uint8_t default_tftp_ip[] = DEFAULT_TFTP_SERVER_IP;
        eeprom_write_block(default_tftp_ip, (void*)EEPROM_TFTP_SERVER_IP, 4);
    }

    // Gateway
    if(is_empty((uint8_t*)EEPROM_GATEWAY, 4)) {
        uint8_t default_gateway[] = DEFAULT_GATEWAY;
        eeprom_write_block(default_gateway, (void*)EEPROM_GATEWAY, 4);
    }

    // Subnet Mask
    if(is_empty((uint8_t*)EEPROM_SUBNET_MASK, 4)) {
        uint8_t default_mask[] = DEFAULT_SUBNET_MASK;
        eeprom_write_block(default_mask, (void*)EEPROM_SUBNET_MASK, 4);
    }

    // Device Name
    if(is_empty((uint8_t*)EEPROM_DEVICE_NAME, 12)) {
        eeprom_write_block(DEFAULT_DEVICE_NAME, (void*)EEPROM_DEVICE_NAME, 12);
    }

    // Version
    if(is_empty((uint8_t*)EEPROM_VERSION, 12)) {
        eeprom_write_block(DEFAULT_VERSION, (void*)EEPROM_VERSION, 12);
    }

    // Update Flag
    if(eeprom_read_byte((uint8_t*)EEPROM_UPDATE_FLAG) == 0xFF) {
        eeprom_write_byte((uint8_t*)EEPROM_UPDATE_FLAG, DEFAULT_UPDATE_FLAG);
    }
}

void eeprom_read_mac(uint8_t* mac) {
    eeprom_read_block(mac, (void*)EEPROM_MAC_ADDR, 6);
}

void eeprom_read_device_ip(uint8_t* ip) {
    eeprom_read_block(ip, (void*)EEPROM_DEVICE_IP, 4);
}

void eeprom_read_tftp_server_ip(uint8_t* ip) {
    eeprom_read_block(ip, (void*)EEPROM_TFTP_SERVER_IP, 4);
}

void eeprom_read_gateway(uint8_t* ip) {
    eeprom_read_block(ip, (void*)EEPROM_GATEWAY, 4);
}

void eeprom_read_subnet_mask(uint8_t* mask) {
    eeprom_read_block(mask, (void*)EEPROM_SUBNET_MASK, 4);
}

void eeprom_read_device_name(char* name) {
    eeprom_read_block(name, (void*)EEPROM_DEVICE_NAME, 12);
}

void eeprom_read_version(char* version) {
    eeprom_read_block(version, (void*)EEPROM_VERSION, 12);
}

uint8_t eeprom_read_update_flag(void) {
    return eeprom_read_byte((uint8_t*)EEPROM_UPDATE_FLAG);
}

void eeprom_write_update_flag(uint8_t flag) {
    eeprom_write_byte((uint8_t*)EEPROM_UPDATE_FLAG, flag);
}
