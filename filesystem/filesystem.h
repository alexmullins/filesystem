#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct fs_file fs_file;

/*
	Initializes the underlying 'filesystem'
	for use by the File IO layer.
*/
bool fs_init();

/*
	Return the fs_file object named
	filename. Returns NULL if no fs_file
	object has that name. 
*/
fs_file* fs_open_file(const char* filename);

/*
	Creates a fs_file with the filename
	set to filename. Returns the fs_file object.
*/
fs_file* fs_create_file(const char* filename);

/*
	Attemps to load the buffer with the 
	contents of the fs_file data entries.
*/
bool fs_load_file_data(fs_file* file, char* buffer, int bufferSize);

/*
	Returns the total size of the fs_file objects 
	data entry payloads.
*/
int fs_file_size(fs_file* file);

/*
	Flushes content to disk.
*/
bool fs_flush_to_disk(fs_file* file, char* buffer, uint64_t size);

/*
	Deletes the fs_file object from the 
	filesystem with the given filename.
*/
bool fs_delete_file(const char* filename);