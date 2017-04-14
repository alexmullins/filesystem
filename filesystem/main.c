#include <stdio.h>
#include "driver.h"
#include "filesystem.h"

void runDriverTests();
void runFileSystemTests();

int main(int argc, char** argv) {
	printf("=== DRIVER TESTS ===\n");
	runDriverTests();
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
	printf("Num of log entries: %d\n", LOG_TOTAL_ENTRIES);
}
