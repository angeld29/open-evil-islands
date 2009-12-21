#include <stdio.h>

#include "ceio.h"

static int close_file(void* client_data)
{
	return fclose((FILE*)client_data);
}

static size_t read_file(void* data, size_t size, size_t n, void* client_data)
{
	return fread(data, size, n, (FILE*)client_data);
}

static size_t write_file(const void* data, size_t size,
							size_t n, void* client_data)
{
	return fwrite(data, size, n, (FILE*)client_data);
}

static int seek_file(long int offset, int whence, void* client_data)
{
	return fseek((FILE*)client_data, offset, whence);
}

static long int tell_file(void* client_data)
{
	return ftell((FILE*)client_data);
}

const io_callbacks IO_CALLBACKS_FILE = {
	close_file, read_file, write_file, seek_file, tell_file
};
