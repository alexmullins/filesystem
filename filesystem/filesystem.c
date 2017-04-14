#include "filesystem.h"
#include "driver.h"
#include "logger.h"
#include "sglib.h"
#include <stddef.h>
#include <malloc.h>

static bool failed = false;
static fs_file* FILE_SYSTEM_LIST;

#define TYPE_HEADER 0
#define TYPE_DATA_CHUNK 1

typedef struct log_entry {
	uint16_t log_id;
	uint16_t status;
	uint16_t object_id;
	uint16_t chunk_id;
	uint16_t type;
	struct log_entry* next;
	struct log_entry* prev;
} log_entry;

struct fs_file {
	const char* file_name;
	uint64_t data_size;
	struct log_entry header;
	int num_data_chunks;
	log_entry* data_chunk_list;
};

log_entry* log_deserialize_entry(int index);
bool log_entry_is_used(log_entry* entry);
bool log_entry_is_valid( log_entry* entry);
bool log_entry_is_header_type(log_entry* entry);
bool log_entry_is_data_chunk_type(log_entry* entry);
bool fs_stitch_together(log_entry* header_list, log_entry* data_chunk_list);

bool fs_init()
{
	if (failed) {
		return false;
	}
	// Loop until we find first unused entry
	// For each entry:
	//		1. Create a log_entry struct from the data read
	//		2. Add to either temp header_list or temp data_chunk_list
	// For each entry in the header_list:
	//		1. Find corresponding data chunks in the data_chunk_list
	//		2. Create fs_file object from the header and data_chunks
	struct log_entry* entry = NULL;
	struct log_entry* header_list = NULL;
	struct log_entry* data_chunk_list = NULL;

	for (int i = 0; i < LOG_TOTAL_ENTRIES; i++) {
		entry = log_deserialize_entry(i);
		if (entry == NULL) {
			logger("FILESYSTEM", "Could not deserialize entry");
			failed = true;
			return false;
		}
		if (!log_entry_is_used(entry)) {
			free(entry);
			break;
		}
		if (!log_entry_is_valid(entry)) {
			free(entry);
			continue;
		}
		if (log_entry_is_header_type(entry)) {
			SGLIB_DL_LIST_ADD(struct log_entry, header_list, entry, prev, next);
		}
		else if (log_entry_is_data_chunk_type(entry)) {
			SGLIB_DL_LIST_ADD(struct log_entry, data_chunk_list, entry, prev, next);
		}
		else {
			logger("FILESYSTEM", "Found unknownn log entry type");
			free(entry);
			continue;
		}
	}

	int hlen, dlen = 0;
	SGLIB_DL_LIST_LEN(struct log_entry, header_list, prev, next, hlen);
	SGLIB_DL_LIST_LEN(struct log_entry, data_chunk_list, prev, next, dlen);
	if (hlen == 0) {
		logger("FILESYSTEM", "Found no header entries.");
	}
	if (dlen == 0) {
		logger("FILESYSTEM", "Found no data chunk entries.");
	}

	if (!fs_stitch_together(header_list, data_chunk_list)) {
		logger("FILESYSTEM", "Couldn't stitch together log entries.");
	}
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

log_entry * log_deserialize_entry(int index)
{
	return NULL;
}

bool log_entry_is_used(log_entry * entry)
{
	return false;
}

bool log_entry_is_valid(log_entry * entry)
{
	return false;
}

bool log_entry_is_header_type(log_entry * entry)
{
	return false;
}

bool log_entry_is_data_chunk_type(log_entry * entry)
{
	return false;
}

bool fs_stitch_together(log_entry * header_list, log_entry * data_chunk_list)
{
	return false;
}
