#include "stdio.h"
#include "driver.h"

int main(int argc, char** argv) {
	printf("Hello world\n");

	// Test Read and Write
	printf("Reading word\n");
	int32_t ret = ReadWord(0x00000000);
	if (ret < 0) {
		printf("Couldn't ReadWord, %d\n", ret);
		return 1;
	}
	printf("Val: %d\n", (uint16_t)ret);

	printf("Writing a word\n");
	ret = WriteWord(0, 30);
	if (ret < 0) {
		printf("Couldn't WriteWord: %d\n", ret);
		return 1;
	}

	printf("Reading word\n");
	ret = ReadWord(0);
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

	printf("Reading word\n");
	ret = ReadWord(0);
	if (ret < 0) {
		printf("Couldn't ReadWord, %d\n", ret);
		return 1;
	}
	printf("Read value: %d\n", ret);

	return 0;
}