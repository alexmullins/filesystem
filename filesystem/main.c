#include "stdio.h"
#include "driver.h"

int main(int argc, char** argv) {
	printf("Hello world\n");

	// Test Read and Write
	printf("Reading word\n");
	int32_t ret = ReadWord(1024);
	if (ret < 0) {
		printf("Couldn't ReadWord, %d\n", ret);
		return 1;
	}
	printf("Val: %d\n", (uint16_t)ret);

	printf("Writing a word\n");
	ret = WriteWord(1024, 30);
	if (ret < 0) {
		printf("Couldn't WriteWord: %d\n", ret);
		return 1;
	}

	printf("Reading word\n");
	ret = ReadWord(1024);
	if (ret < 0) {
		printf("Couldn't ReadWord back out, %d\n", ret);
		return 1;
	}
	printf("ReadWord: %d\n", ret);
	
	// Test EraseAll
	printf("Erasing all sectors\n");
	ret = EraseAllSectors();
	if (ret < 0) {
		printf("Couldn't EraseAllSectors, %d\n", ret);
		return 1;
	}

	// Test for address errors.
	printf("Try to read word at odd boundary\n");
	ret = ReadWord(1);
	if (ret != ERR_BAD_ADDRESS) {
		printf("Expected to get error\n");
		return 1;
	}
	printf("Got expected error\n");
	
	printf("Try to read word outside of boundary\n");
	ret = ReadWord(UINT32_MAX);
	if (ret != ERR_BAD_ADDRESS) {
		printf("Expected to get error\n");
		return 1;
	}
	printf("Got expected error\n");

	return 0;
}