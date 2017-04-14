#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

/*
Error Codes
*/
#define ERR_BAD_ADDRESS -1
#define ERR_DRIVER_FILE -2
#define ERR_BAD_SECTOR -3
#define ERR_ERASE -4
#define ERR_READ -5
#define ERR_WRITE -6

/* 
EraseAllSectors will clear all the memory in the underlying 
driver file. Will return 1 on success and a negative number 
on an error. The negative return value can identify which
error occured. 
*/
int32_t EraseAllSectors();

/*
EraseSector will clear all the memory in the specified sector
of the underlying driver file. Will return 1 on success and 
a negative number on an error. The negative return value can
identify which error occured.
*/
int32_t EraseSector(uint32_t n);


/*
ReadWord will attempt to read an uint16 (2bytes) from 
the address given. Will return an error code with 0
or greater indicating success and a negative value indicating 
an error occurred. 
*/
int32_t ReadWord(uint32_t address);

/*
WriteWord will attemp to write an uint16 to 
the address given. Will return an error code with 1
inidcating success and a negative value inidicating
an error occurred. 
*/
int32_t WriteWord(uint32_t address, uint16_t value);

#endif // !DRIVER_H

