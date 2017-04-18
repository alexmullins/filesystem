#pragma warning(disable:4996) // Used to turn off warnings about fopen

#include "driver.h"
#include "logger.h"
#include <stdio.h>
#include <io.h>
#include <stdint.h>

int32_t _erase_all_sectors();
int32_t _erase_sector(uint32_t sector);
int32_t _sector_to_begin_addr(uint32_t sector);
int32_t _sector_to_end_addr(uint32_t sector);
int32_t _read_word(uint32_t address);
int32_t _write_word(uint32_t address, uint16_t value, uint16_t old);
int32_t _ensure_driver_file_open();
int32_t _driver_file_exists(char* name);
int32_t _create_driver_file(char* name);

static FILE* driverFile = NULL;

int32_t EraseAllSectors() 
{
	if (_ensure_driver_file_open() == -1) {
		return ERR_DRIVER_FILE;
	}
	if (_erase_all_sectors() < 0) {
		return ERR_ERASE;
	}
	return 1;
}

int32_t _erase_all_sectors() 
{
	for (int i = 0; i < SECTOR_NUM; ++i) {
		if (_erase_sector(i) < 0) {
			return -1;
		}
	}
	return 1;
}

int32_t EraseSector(uint32_t sector)
{
	if (_ensure_driver_file_open() == -1) {
		return ERR_DRIVER_FILE;
	}
	if (sector > SECTOR_NUM - 1) {
		return ERR_BAD_SECTOR;
	}
	if (_erase_sector(sector) < 0) {
		return ERR_ERASE;
	}
	return 1;
}

int32_t _erase_sector(uint32_t sector) 
{
	int32_t begin = _sector_to_begin_addr(sector);
	int32_t end = _sector_to_end_addr(sector);
	int32_t i = begin;
	
	if (fseek(driverFile, begin, SEEK_SET) != 0) {
		return -1;
	}

	int ret;
	while (i < end + 1) {
		ret = fputc(0xff, driverFile);
		if (ret == EOF) {
			return -1;
		}
		++i;
	}
	return 1;
}

int32_t _sector_to_begin_addr(uint32_t sector)
{
	return sector * SECTOR_SIZE;
}

int32_t _sector_to_end_addr(uint32_t sector)
{
	return (((sector + 1) * SECTOR_SIZE) - 1);
}

int32_t ReadWord(uint32_t address)
{
	if (_ensure_driver_file_open() == -1) {
		return ERR_DRIVER_FILE;
	}
	if (address % 2 != 0) {
		return ERR_BAD_ADDRESS;
	}
	if (address >= DRIVER_FILE_SIZE) {
		return ERR_BAD_ADDRESS;
	}

	int val = _read_word(address);
	return val;
}

int32_t _read_word(uint32_t address) 
{
	if (fseek(driverFile, address, SEEK_SET) != 0) {
		logger("DRIVER", "Problem seeking in driver file trying to read word.");
		return INT32_MIN;
	}
	uint16_t val = 0;
	int ret = fread(&val, sizeof(val), 1, driverFile);
	if (ret < 1) {
		logger("DRIVER", "Problem reading uint16_t from driver file.");
		return INT32_MIN;
	}
	return val;
}

int32_t WriteWord(uint32_t address, uint16_t value)
{
	if (_ensure_driver_file_open() == -1) {
		return ERR_DRIVER_FILE;
	}
	if (address % 2 != 0) {
		return ERR_BAD_ADDRESS;
	}
	if (address >= DRIVER_FILE_SIZE) {
		return ERR_BAD_ADDRESS;
	}

	int32_t old = _read_word(address);
	if (old == INT32_MIN) {
		return ERR_READ;
	}
	if (_write_word(address, value, old) < 0) {
		return ERR_WRITE;
	}
	return 1;
}

int32_t _write_word(uint32_t address, uint16_t value, uint16_t old) 
{
	if (fseek(driverFile, address, SEEK_SET) != 0) {
		logger("DRIVER", "Problem seeking in driver file trying to write word.");
		return -1;
	}
	uint16_t newVal = old & value;
	if (newVal != value) {
		char msg[80];
		sprintf_s(msg, 80, "_write_word: newVal(%d) != value(%d) \n", newVal, value);
		logger("DRIVER", msg);
	}
	int ret = fwrite(&newVal, sizeof(newVal), 1, driverFile);
	if (ret < 1) {
		logger("DRIVER", "Problem writing uint16_t in driver file.");
		return -1;
	}
	return 1;
}

int32_t _ensure_driver_file_open() 
{
	if (driverFile) {
		// driverFile open
		return 1;
	}
	else {
		// driverFile isn't open
		if (_driver_file_exists("driver_file.dat") > 0) {
			// driverFile exists and is appropriate length
			FILE* fp = fopen("driver_file.dat", "r+b");
			if (fp == NULL) {
				logger("DRIVER", "Could not open driver file1.\n");
				return -1;
			}
			driverFile = fp;
			return 1;
		}
		else {
			// driverFile doesn't exist.
			logger("DRIVER", "Driver file doesn't exist.\n");
			return _create_driver_file("driver_file.dat");
		}
	}
}

int32_t _driver_file_exists(char* name)
{
	if (_access(name, 0) != -1) {
		// file exists, now open and check size.
		FILE* fp = fopen(name, "r+b");
		if (fp == NULL) {
			logger("DRIVER", "Could not open driver file2.\n");
			return -1;
		}
		if (fseek(fp, DRIVER_FILE_SIZE-1, SEEK_SET) != 0) {
			fclose(fp);
			return -1;
		}
		long pos = ftell(fp);
		if (pos != DRIVER_FILE_SIZE-1) {
			fclose(fp);
			return -1;
		}
		fclose(fp);
		return 1;
	}
	else {
		return -1;
	}
}

int32_t _create_driver_file(char* name) 
{
	FILE* fp = fopen(name, "w+b");
	if (fp == NULL) {
		logger("DRIVER", "Could not open driver file3.\n");
		return -1;
	}
	driverFile = fp;

	if (_erase_all_sectors() < 0) {
		return -1;
	}
	return 1;
}