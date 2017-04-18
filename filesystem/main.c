#include <stdio.h>
#include "driver.h"
//#include "filesystem.h"
#include "fio.h"

void runDriverTests();
void runFileSystemTests();

int main(int argc, char** argv) {
	/*printf("=== DRIVER TESTS ===\n");
	runDriverTests();*/
	printf("\n\n=== FILESYSTEM TESTS ===\n");
	runFileSystemTests();
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
	
	//EraseAllSectors();
	CSC322FILE* file = CSC322_fopen("hello.txt", "wb");
	int ret = CSC322_fwrite("hellohellohello", 15, file);
	if (ret < 0) {
		printf("Error writing file.");
	}
	ret = CSC322_fclose(file);
	if (ret < 0) {
		printf("Error closing file.");
	}
}
