#include "filesystem.h"
#include "stddef.h"

#define MAX_FILES 256

static fs_file* FILE_SYSTEM[MAX_FILES];

typedef enum entry_type { HEADER, DATA_CHUNK } entry_type;

struct log_entry {
	uint64_t log_id;
	uint64_t object_id;
	uint64_t chunk_id;
	entry_type type;
};

struct fs_file {
	const char* file_name;
	uint64_t data_size;
	struct log_entry header;
	int num_data_chunks;
	struct log_entry* data_chunk_array;
};

bool fs_init()
{
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

