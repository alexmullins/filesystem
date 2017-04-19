#include <stdio.h>
#include "driver.h"
//#include "filesystem.h"
#include "fio.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void runDriverTests();
void runFileSystemTests();

int main(int argc, char** argv) {
	/*printf("=== DRIVER TESTS ===\n");
	runDriverTests();*/
	printf("\n\n=== FILESYSTEM TESTS ===\n");
	runFileSystemTests();
	getchar();
	return 0;
}

void runDriverTests() {
	printf("Hello world\n");

	// Test Read and Write
	printf("Reading word\n");
	int32_t ret = ReadWord(1024);
	if (ret < 0) {
		printf("Couldn't ReadWord, %d\n", ret);
		return;
	}
	printf("Val: %d\n", (uint16_t)ret);

	printf("Writing a word\n");
	ret = WriteWord(1024, 0);
	if (ret < 0) {
		printf("Couldn't WriteWord: %d\n", ret);
		return;
	}

	printf("Reading word\n");
	ret = ReadWord(1024);
	if (ret < 0) {
		printf("Couldn't ReadWord back out, %d\n", ret);
		return;
	}
	printf("ReadWord: %d\n", ret);

	// Overwrite
	printf("Writing a word\n");
	ret = WriteWord(1024, 30);
	if (ret < 0) {
		printf("Couldn't WriteWord: %d\n", ret);
		return;
	}

	printf("Reading word\n");
	ret = ReadWord(1024);
	if (ret < 0) {
		printf("Couldn't ReadWord back out, %d\n", ret);
		return;
	}
	printf("ReadWord: %d\n", ret);

	// Test EraseAll
	printf("Erasing all sectors\n");
	ret = EraseAllSectors();
	if (ret < 0) {
		printf("Couldn't EraseAllSectors, %d\n", ret);
		return;
	}

	// Test for address errors.
	printf("Try to read word at odd boundary\n");
	ret = ReadWord(1);
	if (ret != ERR_BAD_ADDRESS) {
		printf("Expected to get error\n");
		return;
	}
	printf("Got expected error\n");

	printf("Try to read word outside of boundary\n");
	ret = ReadWord(UINT32_MAX);
	if (ret != ERR_BAD_ADDRESS) {
		printf("Expected to get error\n");
		return;
	}
	printf("Got expected error\n");

	return;
}

void fill_up_invalid_entries() {
	EraseAllSectors();
	printf("test: fill_up_invalid_entries()\n");
	while (true) {
		CSC322FILE* file = CSC322_fopen("hello.txt", "wb");
		int ret = CSC322_fwrite("hellohellohello", 15, file);
		if (ret < 0) {
			printf("Error writing file.\n");
			break;
		}
		ret = CSC322_fclose(file);
		if (ret < 0) {
			printf("Error closing file.\n");
			break;
		}
	}
}

void write_and_read_same() {
	EraseAllSectors();
	printf("test: write_and_read_same()\n");
	printf("writing file...\n");
	CSC322FILE* file = CSC322_fopen("hello.txt", "wb");
	int ret = CSC322_fwrite("hellohellohello", 15, file);
	if (ret < 0) {
		printf("Error writing file.\n");
		return;
	}
	ret = CSC322_fclose(file);
	if (ret < 0) {
		printf("Error closing file.\n");
		return;
	}
	printf("reading file..\n");
	file = CSC322_fopen("hello.txt", "rb");
	char buf[15];
	ret = CSC322_fread(buf, 15, file);
	if (ret < 0) {
		printf("Error writing file.\n");
		return;
	}
	printf("ret: %d\n", ret);
	if (memcmp("hellohellohello", buf, 15) != 0) {
		printf("Error dst != src.\n");
	}
	else {
		printf("OK write-read works.\n");
	}
	ret = CSC322_fclose(file);
	if (ret < 0) {
		printf("Error closing file.\n");
	}
	return;
}

void write_and_read_large_file() {
	EraseAllSectors();
	printf("test: write_and_read_large_file()\n");
	printf("writing file...\n");

#define SIZE 1*1024

	uint8_t buffer[SIZE] = { 0 };

	for (int i = 0; i < SIZE; i++) {
		buffer[i] = (int16_t)rand() % 100;
	}

	CSC322FILE* file = CSC322_fopen("hello.txt", "wb");
	int ret = CSC322_fwrite(buffer, SIZE, file);
	if (ret < 0) {
		printf("Error writing file.\n");
		return;
	}
	ret = CSC322_fclose(file);
	if (ret < 0) {
		printf("Error closing file.\n");
		return;
	}
	printf("reading file..\n");
	uint8_t buffer2[SIZE] = { 0 };
	file = CSC322_fopen("hello.txt", "rb");
	ret = CSC322_fread(buffer2, SIZE, file);
	if (ret < 0) {
		printf("Error writing file.\n");
		return;
	}
	printf("ret: %d\n", ret);
	if (memcmp(buffer, buffer2, SIZE) != 0) {
		printf("Error dst != src.\n");
	}
	else {
		printf("OK write-read works.\n");
	}
	ret = CSC322_fclose(file);
	if (ret < 0) {
		printf("Error closing file.\n");
	}
	return;
}

void runFileSystemTests()
{
	/*EraseAllSectors();
	printf("Num of log entries: %d\n", LOG_TOTAL_ENTRIES);
	log_first_entry();
	uint8_t buffer[2];
	buffer[0] = 3;
	buffer[1] = 0;
	uint16_t word = *(uint16_t*)&buffer;
	printf("WORD: %d\n", word);
	printf("5/2 = %d\n", 5 / 2);*/

	// fill up a file
	
	EraseAllSectors();
	//fill_up_invalid_entries();
	write_and_read_large_file();
}


