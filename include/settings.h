/*
   W5100 TFTP Bootloader - Bootloader Ayarları
   Bu dosya bootloader için gerekli ayarları içerir
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

 // MCU frekansı (16 MHz)
//#define F_CPU 16000000UL

 // Bootloader boyutu (4096 byte = 4KB)
#define BOOTLOADER_SIZE 4096

 // Bootloader başlangıç adresi
#define BOOTLOADER_START (0x8000 - BOOTLOADER_SIZE)

 // Uygulama başlangıç adresi
#define APP_START 0x0000

 // Maksimum uygulama boyutu
#define MAX_APP_SIZE (BOOTLOADER_START - APP_START)

 // Bootloader versiyonu
#define BOOTLOADER_VERSION "1.0.0"

 // Bootloader tanımlayıcısı
#define BOOTLOADER_ID "W5100_TFTP"

 #endif /* _SETTINGS_H_ */