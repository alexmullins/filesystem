#include "filesystem.h"
#include "driver.h"
#include <stddef.h>

static fs_file* FILE_SYSTEM_LIST;

#define TYPE_HEADER 0
#define TYPE_DATA_CHUNK 1

struct log_entry {
	uint16_t log_id;
	uint16_t status;
	uint16_t object_id;
	uint16_t chunk_id;
	uint16_t type;
	struct log_entry* next;
	struct log_entry* prev;
};

struct fs_file {
	const char* file_name;
	uint64_t data_size;
	struct log_entry header;
	int num_data_chunks;
	struct log_entry* data_chunk_list;
};

bool fs_init()
{
	// Loop until we find first unused entry
	// For each entry:
	//		1. Create a log_entry struct from the data read
	//		2. Add to either temp header_list or temp data_chunk_list
	// For each entry in the header_list:
	//		1. Find corresponding data chunks in the data_chunk_list
	//		2. Create fs_file object from the header and data_chunks


	return true;
}

fs_file* fs_open_file(const char* filename)
{
	return NULL;
}

fs_file * fs_create_file(const char * filename)
{
	return NULL;
}

bool fs_load_file_data(fs_file* file, char* buffer, int bufferSize)
{
	return false;
}

int fs_file_size(fs_file* file)
{
	return (int)file->data_size;
}

bool fs_flush_to_disk(fs_file* file, char* buffer, uint64_t size)
{
	return false;
}

bool fs_delete_file(const char * filename)
{
	return false;
}

