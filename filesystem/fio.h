#pragma once

#include "stddef.h"

typedef struct CSC322FILE CSC322FILE;

/*
	fopen()
*/
CSC322FILE* CSC322_fopen(const char* filename, const char* mode);

/*
	fclose()
*/
int CSC322_fclose(CSC322FILE* stream);

/*
	fread()
*/
int CSC322_fread(void *buffer, size_t nBytes, CSC322FILE* stream);

/*
	fwrite()
*/
int CSC322_fwrite(void *buffer, size_t nBytes, CSC322FILE* stream);

/*
	fseek()
*/
int CSC322_fseek(CSC322FILE* stream, long offset, int origin);

/*
	ftell()
*/
int CSC322_ftell(CSC322FILE* stream);

/*
	remove()
*/
int CSC322_remove(const char* path);