#include "fio.h"
#include "filesystem.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"

#define MAX_FILE_SIZE 63*1024
#define ERR_FILESYSTEM -2
#define ERR_CLOSE -3
#define ERR_NULL -4
#define ERR_DELETE -5

struct CSC322FILE {
	fs_file* file;
	uint64_t fpos;
	bool dirty;
	const char* mode; // "rb", "wb", "w+b", "ab"
	uint64_t fsize;
	char buffer[MAX_FILE_SIZE];
};

CSC322FILE* CSC322_fopen(const char* filename, const char* mode)
{
	if (!fs_init()) {
		return NULL;
	}
	size_t str_len = strlen(filename);
	if (str_len > 64 || str_len == 0) {
		return NULL;
	}
	if (strcmp(mode, "rb") == 0) { 
		// "rb" attempt to open file for reading
		// if exists, fail if nonexist.
		fs_file* file = fs_open_file(filename);
		if (file == NULL) {
			return NULL;
		}
		CSC322FILE* open_file = calloc(1, sizeof(CSC322FILE)); // TODO: must delete when close file
		if (open_file == NULL) {
			return NULL;
		}
		open_file->file = file;
		open_file->mode = mode;
		open_file->fpos = 0;
		open_file->dirty = false;
		fs_load_file_data(open_file->file, open_file->buffer, MAX_FILE_SIZE); // TODO: prolly should check for error
		open_file->fsize = fs_file_size(open_file->file);
		return open_file;
	}
	else if (strcmp(mode, "wb") == 0) { 
		// "wb" open file for writing. 
		// If already exists destroy previous content.
		fs_file* file;
		file = fs_open_file(filename);
		if (file == NULL) {
			file = fs_create_file(filename);
			if (file == NULL) {
				// need to delete file created since it will linger in FS
				logger("FIO", "CSC322_fopen: failed to create file.");
			}
		}
		CSC322FILE* open_file = calloc(1, sizeof(CSC322FILE)); // TODO: must delete when close file
		if (open_file == NULL) {
			return NULL;
		}
		open_file->file = file;
		open_file->mode = mode;
		open_file->fpos = 0;
		open_file->fsize = 0;
		open_file->dirty = true;
		return open_file;
	}
	else if (strcmp(mode, "w+b") == 0) { 
		// "w+b" opens file for reading and writing.
		// If already exists destroy previous content.
		fs_file* file;
		file = fs_open_file(filename);
		if (file == NULL) {
			file = fs_create_file(filename);
			if (file == NULL) {
				logger("FIO", "CSC322_fopen: failed to create file.");
				return NULL;
			}
		}
		CSC322FILE* open_file = calloc(1, sizeof(CSC322FILE)); // TODO: must delete when close file
		if (open_file == NULL) {
			return NULL;
		}
		open_file->file = file;
		open_file->mode = mode;
		open_file->fpos = 0;
		open_file->fsize = 0;
		open_file->dirty = true;
		return open_file;
	}
	else if (strcmp(mode, "ab") == 0) { 
		// "ab" opens file for writing (appending).
		// Create if nonexist.
		fs_file* file;
		file = fs_open_file(filename);
		if (file == NULL) {
			file = fs_create_file(filename);
			if (file == NULL) {
				logger("FIO", "CSC322_fopen: failed to create file.");
			}
		}
		CSC322FILE* open_file = calloc(1, sizeof(CSC322FILE)); // TODO: must delete when close file
		if (open_file == NULL) {
			return NULL;
		}
		open_file->file = file;
		open_file->mode = mode;
		int64_t size = fs_file_size(open_file->file);
		open_file->fpos = size;
		open_file->fsize = size;
		open_file->dirty = true;
		fs_load_file_data(open_file->file, open_file->buffer, MAX_FILE_SIZE); // TODO: prolly should check for error
		return open_file;
	}
	else {
		// invalid mode
		return NULL;
	}
}

int CSC322_fclose(CSC322FILE* stream)
{
	if (stream == NULL) {
		return ERR_NULL;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	int err = 0;
	if (stream->dirty) {
		if (!fs_flush_to_disk(stream->file, stream->buffer, stream->fsize)) {
			err = ERR_CLOSE;
		}
	}
	free(stream); // Cleanup
	return err;
}

int CSC322_fread(void* buffer, size_t nBytes, CSC322FILE* stream)
{
	if (stream == NULL) {
		return ERR_NULL;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	if (nBytes == 0) {
		return 0;
	}
	if (nBytes < 0) {
		return -1;
	}
	// must be the correct mode to read
	if (strcmp(stream->mode, "ab") == 0) {
		return -1;
	}
	else if (strcmp(stream->mode, "wb") == 0) {
		return -1;
	}
	// check we dont go over our limit
	uint64_t futurePos = nBytes + stream->fpos;
	if (futurePos > stream->fsize) {
		// read only up to the file size
		futurePos = stream->fsize - 1;
	}
	// recalculate how much we will actually read
	uint64_t willRead = futurePos - stream->fpos;
	if (willRead == 0) {
		return EOF;
	}
	memcpy(buffer, stream->buffer, (size_t)willRead);
	stream->fpos = futurePos;
	return (int)willRead;
}

int CSC322_fwrite(void* buffer, size_t nBytes, CSC322FILE* stream)
{
	if (stream == NULL) {
		return ERR_NULL;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	if (nBytes == 0) {
		return 0;
	}
	if (nBytes < 0) {
		return -1;
	}
	if (strcmp(stream->mode, "rb") == 0) {
		return -1;
	}
	uint64_t roomLeft = MAX_FILE_SIZE - stream->fpos;
	uint64_t willWrite = nBytes;
	if (willWrite > roomLeft) {
		willWrite = roomLeft;
	}
	memcpy(stream->buffer + stream->fpos, buffer, (size_t)willWrite);
	stream->fpos = stream->fpos + willWrite;
	if (stream->fpos > stream->fsize) {
		stream->fsize = stream->fpos;
	}
	stream->dirty = true;
	return (int)willWrite;
}

int CSC322_fseek(CSC322FILE* stream, long offset, int origin)
{
	if (stream == NULL) {
		return ERR_NULL;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	if (strcmp(stream->mode, "ab") == 0) {
		return -1;
	}

	// = 0 == SEEK_SET
	if (origin == 0) {
		if (offset > MAX_FILE_SIZE || offset < 0) {
			return -1;
		}
		stream->fpos = offset - 1;
	}
	// = 1 == SEEK_CUR
	else if (origin == 1) {
		uint64_t temp = stream->fpos + offset;
		if (temp > MAX_FILE_SIZE || offset < 0) {
			return -1;
		}
		stream->fpos += offset - 1;
	}
	// = 2 == SEEK_END
	// http://en.cppreference.com/w/c/io/fseek
	// "Binary streams are not required to support SEEK_END, in particular if additional null bytes are output."
	else if (origin == 2) {
		return -1;
	}
	else {
		// error
		return -1;
	}
	// Check mode
	return 0;
}

int CSC322_ftell(CSC322FILE* stream)
{
	if (stream == NULL) {
		return ERR_NULL;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	return (int)stream->fpos;
}

int CSC322_remove(const char* filename)
{
	size_t str_len = strlen(filename);
	if (str_len > 64 || str_len == 0) {
		return -1;
	}
	if (!fs_init()) {
		return ERR_FILESYSTEM;
	}
	if (!fs_delete_file(filename)) {
		return ERR_DELETE;
	}
	return 0;
}
