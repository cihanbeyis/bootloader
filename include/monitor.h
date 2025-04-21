// monitor.h
#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>

// Monitor modülünü başlatır
void monitor_init(void);

// EEPROM ve ağ bilgilerini ekrana yazar
void monitor_print_all(void);

// (Gerekirse ek görevler için)
void monitor_task(void);

#endif // MONITOR_H
