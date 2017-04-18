#include "filesystem.h"
#include "driver.h"
#include "logger.h"
#include "sglib.h"
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

static bool failed = false;
static bool success = false;
static uint32_t nextEntryLocation = 0;
static uint32_t nextObjectID = 0;
static fs_file* FILE_SYSTEM_LIST;

#define TYPE_HEADER 0
#define TYPE_DATA_CHUNK 1

#define STATUS_USED 1
#define STATUS_INVALID 2

#define LOG_ENTRY_COMMON_SIZE 10 // status + object_id + chunk_id + type + payload_size
#define LOG_ENTRY_MAX_PAYLOAD_SIZE LOG_ENTRY_SIZE - LOG_ENTRY_COMMON_SIZE

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
uint8_t* log_load_payload(log_entry* entry);
void fs_sort_files_by_name();
void fs_sort_files_data_chunks();
uint16_t log_get_next_obj_id();
uint16_t log_get_next_log_id();
bool log_serialize_entry(log_entry* entry, uint8_t* payload);
void log_entry_set_used(log_entry* entry);
void log_entry_set_valid(log_entry* entry);

bool fs_init()
{
	if (failed) {
		return false;
	}
	if (success) {
		return true;
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
	success = true;
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
		return true;
	}
	
	fs_stitch_together(header_list, data_chunk_list); // should clean up leftover entries in lists.
	fs_read_filenames_from_disk();
	fs_sort_files_by_name();
	fs_sort_files_data_chunks();

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

// assuming no fs_file with filename exists.
fs_file * fs_create_file(const char * filename)
{
	fs_file* file = calloc(1, sizeof(fs_file));
	file->file_name = filename;
	SGLIB_DL_LIST_ADD(struct fs_file, FILE_SYSTEM_LIST, file, prev, next);
	return file;
}

bool fs_load_file_data(fs_file* file, char* buffer, int bufferSize)
{
	// Foreach data chunk in fs_file
	// Read payload and fill in buffer
	log_entry* entry = NULL;
	int read = 0;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct log_entry, file->data_chunks, entry, prev, next, {
		uint8_t* payload = log_load_payload(entry);
		read += entry->payload_size;
		if (read < bufferSize) {
			memcpy(buffer, payload, entry->payload_size);
			buffer += entry->payload_size;
		}
		free(payload);
	});
	return false;
}

int fs_file_size(fs_file* file)
{
	return (int)file->data_size;
}

bool fs_flush_to_disk(fs_file* file, char* buffer, uint64_t size)
{	
	int max_chunk_id = 0;
	int num_of_data_chunks = 0;
	if (file->data_chunks != NULL) {
		// Invalidate all previous data chunks.
		log_entry* entry = NULL;
		SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct log_entry, file->data_chunks, entry, prev, next, {
			log_invalidate_entry(entry->log_id);
			if (entry->chunk_id > max_chunk_id) {
				max_chunk_id = entry->chunk_id;
			}
			num_of_data_chunks++;
		});
	}
	// Check if we need to write a new header
	if (file->header == NULL) {
		log_entry* header = calloc(1, sizeof(log_entry));
		header->object_id = log_get_next_obj_id();
		header->chunk_id = 0;
		header->type = TYPE_HEADER;
		header->payload_size = strlen(file->file_name) + 1;
		file->header = header;
		log_serialize_entry(header, (uint8_t*)file->file_name); // will set the new log_id for us
	}
	// Calculate the num of needed data chunks to hold size of data.
	int num_of_data_chunks_needed = size / LOG_ENTRY_MAX_PAYLOAD_SIZE;
	if (size % LOG_ENTRY_MAX_PAYLOAD_SIZE != 0) {
		num_of_data_chunks_needed++;
	}
	max_chunk_id++;
	int diff = num_of_data_chunks_needed - num_of_data_chunks;
	if (diff > 0) {
		// need more chunks
		for (int i = 0; i < diff; i++) {
			log_entry* new_entry = calloc(1, sizeof(log_entry));
			new_entry->object_id = file->header->object_id;
			new_entry->chunk_id = max_chunk_id;
			max_chunk_id++;
			new_entry->type = TYPE_DATA_CHUNK;
			log_entry* last_entry = NULL;
			SGLIB_DL_LIST_GET_LAST(struct log_entry, file->data_chunks, prev, next, last_entry);
			if (last_entry == NULL) {
				SGLIB_DL_LIST_ADD(struct log_entry, file->data_chunks, new_entry, prev, next);
			}
			else {
				SGLIB_DL_LIST_ADD_AFTER(struct log_entry, last_entry, new_entry, prev, next);
			}
		}
	}
	else if (diff < 0) {
		// remove chunks
		diff *= -1;
		for (int i = 0; i < diff; i++) {
			log_entry* last_entry = NULL;
			SGLIB_DL_LIST_GET_LAST(struct log_entry, file->data_chunks, prev, next, last_entry);
			if (last_entry == NULL) {
				last_entry = file->data_chunks;
			}
			SGLIB_DL_LIST_DELETE(struct log_entry, file->data_chunks, last_entry, prev, next);
			free(last_entry);
		}
	}

	char* current = buffer;
	int left = size;
	log_entry* entry = NULL;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct log_entry, file->data_chunks, entry, prev, next, {
		if (left > 0) {
			if (left >= LOG_ENTRY_MAX_PAYLOAD_SIZE) {
				entry->payload_size = LOG_ENTRY_MAX_PAYLOAD_SIZE;
				left -= LOG_ENTRY_MAX_PAYLOAD_SIZE;
			}
			else {
				entry->payload_size = left;
				left -= left;
			}
			log_serialize_entry(entry, current);
			current = current + entry->payload_size;
		}
	});
	
	return true;
}

uint16_t log_get_next_obj_id() {
	uint16_t temp = nextObjectID;
	nextObjectID++;
	return temp;
}

uint16_t log_get_next_log_id() {
	uint16_t temp = nextEntryLocation;
	nextEntryLocation++;
	return temp;
}

bool log_serialize_entry(log_entry* entry, uint8_t* payload) {
	entry->log_id = log_get_next_log_id();
	entry->status = 0xFFFF;
	log_entry_set_used(entry);
	log_entry_set_valid(entry);
	uint32_t address = entry->log_id * LOG_ENTRY_SIZE;
	int payload_size = entry->payload_size;
	if (payload_size % 2 != 0) {
		payload_size++;
	}
	int total_size = LOG_ENTRY_COMMON_SIZE + payload_size;
	uint16_t* buffer = calloc(1, total_size);
	// status + object_id + chunk_id + type + payload_size
	buffer[0] = entry->status;
	buffer[1] = entry->object_id;
	buffer[2] = entry->chunk_id;
	buffer[3] = entry->type;
	buffer[4] = entry->payload_size;

	buffer += LOG_ENTRY_COMMON_SIZE;

	memcpy(buffer, payload, payload_size);

	int num_words = total_size / 2;

	for (int i = 0; i < num_words; i++) {
		WriteWord(address, buffer[i]);
		address += 2;
	}
	free(buffer);
	return true;
}

// TODO
void log_entry_set_used(log_entry * entry)
{
}

// TODO
void log_entry_set_valid(log_entry * entry)
{
}

// TODO
bool fs_delete_file(const char * filename)
{
	return false;
}

log_entry* log_deserialize_entry(int index)
{
	int num_words = LOG_ENTRY_COMMON_SIZE / 2;
	uint16_t buffer[LOG_ENTRY_COMMON_SIZE / 2];

	uint32_t address = index * LOG_ENTRY_SIZE;

	for (int i = 0; i < num_words; i++) {
		int32_t word = ReadWord(address);
		if (word == INT32_MIN) {
			logger("FILESYSTEM", "Read word error when deserializing");
			return NULL;
		}
		buffer[i] = (uint16_t)word;
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
	// Loop over all fs_file in FILE_SYSTEM_LIST
	// Grabbing the header entry and loading the payload for it
	// store in fs_file.file_name
	fs_file* file = NULL;
	log_entry* header = NULL;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct fs_file, FILE_SYSTEM_LIST, file, prev, next, {
		header = file->header;
		uint8_t* name = log_load_payload(header);
		if (name == NULL) {
			logger("FILESYSTEM", "Load payload returned NULL.");
			return;
		}
		file->file_name = (const char*)name; // must free when deleting file from FILE_SYSTEM_LIST
	});
}


uint8_t* log_load_payload(log_entry* entry) {
	uint16_t payload_size = entry->payload_size;
	uint16_t payload_size_fixed = payload_size;
	uint8_t* buffer = calloc(payload_size, sizeof(uint8_t));
	if (payload_size % 2 != 0) {
		payload_size_fixed++;
	}
	if (payload_size_fixed >= LOG_ENTRY_MAX_PAYLOAD_SIZE) {
		payload_size_fixed = LOG_ENTRY_MAX_PAYLOAD_SIZE;
	}
	uint16_t num_words = payload_size_fixed / 2;
	uint32_t address = (entry->log_id * LOG_ENTRY_SIZE) + LOG_ENTRY_COMMON_SIZE;
	for (int i = 0; i < payload_size;) {
		uint32_t word = ReadWord(address);
		if (word == INT32_MIN) {
			logger("FILESYSTEM", "ReadWord returned INT32_MIN while reading payload.");
			return NULL;
		}
		uint8_t* p = NULL;
		p = (uint8_t*)word;
		buffer[i] = p[0];
		i++;
		if (i == payload_size) { // break early if we are reading odd amount of data
			break;
		}
		buffer[i] = p[1];
		i++;
		address += 2;
	}
	return buffer;
}


#define FS_FILE_NAME_COMPARE(f1, f2) (strcmp(f1->file_name, f2->file_name))

void fs_sort_files_by_name()
{
	SGLIB_DL_LIST_SORT(struct fs_file, FILE_SYSTEM_LIST, FS_FILE_NAME_COMPARE, prev, next);
}

#define FS_DATA_CHUNK_COMPARE(e1, e2) (e1->chunk_id - e2->chunk_id)

void fs_sort_files_data_chunks()
{
	fs_file* file = NULL;
	SGLIB_DL_LIST_MAP_ON_ELEMENTS(struct fs_file, FILE_SYSTEM_LIST, file, prev, next, {
		SGLIB_DL_LIST_SORT(struct log_entry, file->data_chunks, FS_DATA_CHUNK_COMPARE, prev, next);
	});
}

// TODO
bool log_invalidate_entry(int index) {
	return false;
}

void log_first_entry() {
	log_entry* entry = log_deserialize_entry(0);
	printf("Status: %d\n", entry->status);
	printf("Object_ID: %d\n", entry->object_id);
	printf("Chunk_ID: %d\n", entry->chunk_id);
	printf("Type: %d\n", entry->type);
	printf("Payload Size: %d\n", entry->payload_size);
	printf("IsUsed: %d\n", log_entry_is_used(entry));
	printf("IsValid: %d\n", log_entry_is_valid(entry));
	printf("IsHeader: %d\n", log_entry_is_header_type(entry));
	printf("IsData: %d\n", log_entry_is_data_chunk_type(entry));
	fs_init();
}