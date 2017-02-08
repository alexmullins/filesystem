#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#ifndef _DRIVER_H
#define _DRIVER_H


extern FILE* driverFile;

/* 
	EraseAllSectors will clear all the memory in the underlying 
	driver file. Will return 1 on success and a negative number 
	on an error. The negative return value can idetify which
	error occured. 
*/
int EraseAllSectors();

/*
	EraseSector will clear all the memory in the underlying
	driver file. Will return 1 on success and a negative number
	on an error. The negative return value can idetify which
	error occured.
*/
int EraseSector(int n);


/*
	ReadWord will attempt to read an uint16 (2bytes) from 
	the address given. Will return an error code with 0
	or greater indicating success and a negative value indicating 
	an error occurred. 
*/
int ReadWord(uint32_t address);

/*
	WriteWord will attemp to write an uint16 to 
	the address given. Will return an error code with 1
	inidcating success and a negative value inidicating
	an error occurred. 
*/
int WriteWord(uint32_t address, uint16_t value);

#endif // !_DRIVER_H

