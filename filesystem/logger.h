#pragma once

#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define LOG
//#define LOG_STDOUT

void logger(const char* tag, const char* msg);