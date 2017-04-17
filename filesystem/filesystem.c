#include "filesystem.h"
#include "driver.h"
#include "logger.h"
#include "sglib.h"
#include <stddef.h>
#include <malloc.h>
#include <string.h>

static bool failed = false;
static uint32_t nextEntryLocation = 0;
static uint32_t nextObjectID = 0;
static fs_file* FILE_SYSTEM_LIST;

#define TYPE_HEADER 0
#define TYPE_DATA_CHUNK 1

#define STATUS_USED 1
#define STATUS_INVALID 2

#define LOG_ENTRY_COMMON_SIZE 10 // status + object_id + chunk_id + type + payload_size
#define LOG_ENTRY_PAYLOAD_SIZE LOG_ENTRY_SIZE - LOG_ENTRY_COMMON_SIZE

typedef struct log_entry {
	uint16_t log_id;
	uint16_t status;		// on disk
	uint16_t object_id;		// on disk
	uint16_t chunk_id;		// on disk
	uint16_t type;			// on disk
	uint16_t payload_size;	// on disk
	struct log_entry* next;
	struct log_entry* prev;
} log_entry;

struct fs_file {
	const char* file_name;
	uint64_t data_size;
	struct log_entry* header;
	struct log_entry* data_chunks;
	struct fs_file* next;
	struct fs_file* prev;
};

log_entry* log_deserialize_entry(int index);
bool log_entry_is_used(log_entry* entry);
bool log_entry_is_valid( log_entry* entry);
bool log_entry_is_header_type(log_entry* entry);
bool log_entry_is_data_chunk_type(log_entry* entry);
void fs_stitch_together(log_entry* header_list, log_entry* data_chunk_list);
void fs_read_filenames_from_disk();
bool log_invalidate_entry(int index);

bool fs_init()
{
	if (failed) {
		return false;
	}
	struct log_entry* entry = NULL;
	struct log_entry* header_list = NULL;
	struct log_entry* data_chunk_list = NULL;

	uint32_t highObjID = 0;

	for (int i = 0; i < LOG_TOTAL_ENTRIES; i++) {
		entry = log_deserialize_entry(i);
		if (entry == NULL) {
			logger("FILESYSTEM", "Could not deserialize entry");
			failed = true;
			return false;
		}
		if (!log_entry_is_used(entry)) {
			nextEntryLocation = i;
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
			logger("FILESYSTEM", "Found unknown log entry type");
			free(entry);
			continue;
		}
		if (entry->object_id > highObjID) {
			highObjID = entry->object_id;
		}
	}
	highObjID++;
	nextObjectID = highObjID;
	int hlen, dlen = 0;
	SGLIB_DL_LIST_LEN(struct log_entry, header_list, prev, next, hlen);
	SGLIB_DL_LIST_LEN(struct log_entry, data_chunk_list, prev, next, dlen);
	if (dlen == 0) {
		logger("FILESYSTEM", "Found no data chunk entries.");
	}
	if (hlen == 0) {
		logger("FILESYSTEM", "Found no header entries.");
	}
	
	fs_stitch_together(header_list, data_chunk_list); // should clean up leftover entries in lists.
	// fs_read_filenames_from_disk();

	return true;
}

fs_file* fs_open_file(const char* filename)
{
	bool found = false;
	fs_file* file = NULL;
	fs_file* theFile = NULL;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct fs_file, FILE_SYSTEM_LIST, file, prev, next, {
		if (!found) {
			if (strcmp(filename, file->file_name) == 0) {
				theFile = file;
				found = true;
			}
		}
	});
	if (theFile == NULL) {
		logger("FILESYSTEM", "Could not find file with matching filename to open.");
		return NULL;
	}
	return theFile;
}

fs_file * fs_create_file(const char * filename)
{
	fs_file* file = calloc(1, sizeof(fs_file));
	file->file_name = filename;
	SGLIB_DL_LIST_ADD(struct fs_file, FILE_SYSTEM_LIST, file, prev, next);
	return file;
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
	int num_words = LOG_ENTRY_COMMON_SIZE / 2;
	uint16_t buffer[LOG_ENTRY_COMMON_SIZE / 2];

	uint32_t address = index * LOG_ENTRY_SIZE;

	for (int i = 0; i < num_words; i++) {
		int32_t word = ReadWord(address);
		if (word < 0) {
			logger("FILESYSTEM", "Read word error when deserializing");
			return NULL;
		}
		buffer[i] = word;
		address += 2;
	}

	log_entry* entry = calloc(1, sizeof(log_entry));
	if (entry == NULL) {
		logger("FILESYSTEM", "Calloc return NULL pointer.");
		return NULL;
	}
	// status + object_id + chunk_id + type + payload_size
	entry->log_id = index;
	entry->status = buffer[0];
	entry->object_id = buffer[1];
	entry->chunk_id = buffer[2];
	entry->type = buffer[3];
	entry->payload_size = buffer[4];

	return entry;
}

bool log_entry_is_used(log_entry * entry)
{
	return ((entry->status & STATUS_USED) == 0);
}

bool log_entry_is_valid(log_entry * entry)
{
	return ((entry->status & STATUS_INVALID) > 0);
}

bool log_entry_is_header_type(log_entry * entry)
{
	return (entry->type == TYPE_HEADER);
}

bool log_entry_is_data_chunk_type(log_entry * entry)
{
	return (entry->type == TYPE_DATA_CHUNK);
}

void fs_stitch_together(log_entry * header_list, log_entry * data_chunk_list)
{
	log_entry* header;
	log_entry* data;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct log_entry, header_list, header, prev, next, {
		SGLIB_DL_LIST_DELETE(struct log_entry, header_list, header, prev, next);
		int obj_id = header->object_id;
		fs_file* file = calloc(1, sizeof(fs_file));
		if (file == NULL) {
			logger("FILESYSTEM", "Calloc return NULL fs_file while stitching.");
			return;
		}
		file->header = header;
		SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct log_entry, data_chunk_list, data, prev, next, {
			if (data->object_id == obj_id) {
				SGLIB_DL_LIST_DELETE(struct log_entry, data_chunk_list, data, prev, next);
				SGLIB_DL_LIST_ADD(struct log_entry, file->data_chunks, data, prev, next);
			}
		});
		// TODO: sort the file->data_chunks list
		SGLIB_DL_LIST_ADD(struct fs_file, FILE_SYSTEM_LIST, file, prev, next);
	});
	// TODO: Clean up garbage in data_chunk_list
	return;
}

void fs_read_filenames_from_disk() {
	return;
}

bool log_invalidate_entry(int index) {
	return false;
}