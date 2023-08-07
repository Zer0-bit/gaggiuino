#ifndef WATCH_UPGRADE_H
#define WATCH_UPGRADE_H

#include <Arduino.h>

// Replace FLASH_SECTOR_COUNT and SECTOR_SIZE with actual values for the STM32F411CEU6 mcu
#define FW_UPGRADE_MARKER 0x53544D20 // HEX for STM32
#define FLASH_SECTOR_SIZE 0x8
#define FLASH_SECTOR_COUNT 8
#define SECTOR_SIZE 16384

void waitForFw(void);
void updateFirmware(void);
void eraseFlashSectors(uint32_t address, size_t size);
void writeFlash(uint32_t address, const char* data, size_t size);

#endif
