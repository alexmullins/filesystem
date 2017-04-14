#pragma warning(disable:4996) // Used to turn off warnings about fopen and ctime

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include "logger.h"

#ifdef LOG

#define LOG_SIZE 100
static FILE* logFile = NULL;

bool log_init() {
	if (logFile != NULL) {
		return true;
	}
	time_t now;
	time(&now);
	logFile = fopen("log.txt", "w");
	if (logFile == NULL) {
		return false;
	}
	return true;
}

void logger(const char* tag, const char* msg) {
	log_init();
	time_t t;
	time(&t);

#ifdef LOG_STDOUT
	printf("%s [%s]: %s\n", ctime(&t), tag, msg);
#else
	fprintf(logFile, "%s [%s]: %s\n", ctime(&t), tag, msg);
#endif // LOG_STDOUT

	return;
}

#else 

void logger(const char* tag, const char* msg) {
	return;
}

#endif // LOG