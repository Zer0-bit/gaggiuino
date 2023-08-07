#include "stm32f4xx_hal.h"
#include "watch-upgrade.h"
#include "../log.h"

// Array that holds the start addresses of each flash sector
const std::array<uint32_t, FLASH_SECTOR_COUNT> flashSectorStartAddresses = {
  0x08000000,
  0x08004000,
  0x08008000,
  0x0800C000,
  0x08010000,
  0x08020000,
  0x08040000,
  0x08060000
};

void waitForFw() {
  while (!Serial); // Wait for serial port to connect
  // Wait for the start marker of the firmware binary
  uint32_t receivedMarker = 0;
  while (true) {
    if ((uint32_t)Serial.available() >= sizeof(receivedMarker)) {
      uint8_t* markerPtr = (uint8_t*)&receivedMarker;
      for (size_t i = 0; i < sizeof(receivedMarker); ++i) {
        markerPtr[i] = Serial.read();
      }
      if (receivedMarker == FW_UPGRADE_MARKER) {
        // Firmware binary available -> upgrade!
        updateFirmware();
      }
    }
  }
}

// Get the sector number based on the memory address
uint32_t getSectorNumber(uint32_t address) {
  for (uint32_t i = 0; i < FLASH_SECTOR_COUNT - 1; ++i) {
    if (address >= flashSectorStartAddresses[i] && address < flashSectorStartAddresses[i + 1]) {
      return i;
    }
  }
  return FLASH_SECTOR_COUNT - 1;
}

void updateFirmware() {
  Serial.println("Starting firmware update...");

  // Set the starting address for firmware flashing
  uint32_t address = BOOTLOADER_START_ADDRESS;

  /*
  According to the "Application Note: AN2606 - STM32™ microcontroller system memory boot mode"
  Most will have to kill loads of peripherals before writing the firmware to flash
  */

  // Erase the sectors where the firmware will be written
  eraseFlashSectors(address, Serial.available());

  // Write the firmware to the flash memory
  while (Serial.available()) {
    char buffer[256];
    size_t bytesRead = Serial.readBytes(buffer, sizeof(buffer));
    writeFlash(address, buffer, bytesRead);
    address += bytesRead;
  }

  // Post firmware write-to-flash reset
  NVIC_SystemReset();
}

// Erase the flash sectors
void eraseFlashSectors(uint32_t address, size_t size) {
  uint32_t sectorNumber = getSectorNumber(address);
  uint32_t lastSectorNumber = getSectorNumber(address + size - 1);

  HAL_FLASH_Unlock();

  for (uint32_t sector = sectorNumber; sector <= lastSectorNumber; ++sector) {
    FLASH_Erase_Sector(sector, VOLTAGE_RANGE_3);
  }

  HAL_FLASH_Lock();
}

// Write data to flash
void writeFlash(uint32_t address, const char* data, size_t size) {
  HAL_FLASH_Unlock();
  for (size_t i = 0; i < size; i += 4) {
    uint32_t wordData = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, wordData);
  }
  HAL_FLASH_Lock();
}
