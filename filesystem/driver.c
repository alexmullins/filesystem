#include "driver.h"
#include "stdio.h"
#include "io.h"

#define KB 1024
#define NUM_SECTORS 20
#define DRIVER_FILE_SIZE 64 * KB * NUM_SECTORS

static FILE* driverFile = NULL;

int EraseAllSectors() 
{
	if (ensureDriverFileOpen() == -1) {
		return NO_DRIVER_FILE;
	}
}

int EraseSector(uint32_t sector) 
{
	if (ensureDriverFileOpen() == -1) {
		return NO_DRIVER_FILE;
	}
}

int ReadWord(uint32_t address)
{
	if (ensureDriverFileOpen() == -1) {
		return NO_DRIVER_FILE;
	}
}

int WriteWord(uint32_t address, uint16_t value)
{
	if (ensureDriverFileOpen() == -1) {
		return NO_DRIVER_FILE;
	}
}

int ensureDriverFileOpen() {
	if (driverFile) {
		// driverFile open
		return 1;
	}
	else {
		// driverFile isn't open
		if (driverFileExists("driver_file.dat")) {
			// driverFile exists and is appropriate length
			FILE* fp = fopen("driver_file.dat", "rwb");
			if (!fp) {
				return -1;
			}
			driverFile = fp;
			return 1;
		}
		else {
			// driverFile doesn't exist.
			return createDriverFile("driver_file.dat");
		}
	}
}

int driverFileExists(char* name)
{
	if (_access(name, 0) != -1) {
		// file exists, now open and check size.
		FILE* fp = fopen(name, "rwb");
		if (!fp) {
			return -1;
		}
		if (fseek(fp, 0L, SEEK_END) < 0) {
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

int createDriverFile(char* name) {
	FILE* fp = fopen(name, "rwb");
	if (!fp) {
		return -1;
	}
	if (fseek(fp, DRIVER_FILE_SIZE - 1, SEEK_SET) < 0) {
		fclose(fp);
		return -1;
	}
	if (fputc(255, fp) == EOF) {
		fclose(fp);
		return -1;
	}
	if (fseek(fp, 0L, SEEK_SET) < 0) {
		fclose(fp);
		return -1;
	}
	driverFile = fp;
	return 1;
}