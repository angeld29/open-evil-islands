#include <stdio.h>

#include "ceio.h"

static int file_close(void* client_data)
{
	return fclose((FILE*)client_data);
}

static size_t file_read(void* data, size_t size, size_t n, void* client_data)
{
	return fread(data, size, n, (FILE*)client_data);
}

static size_t file_write(const void* data, size_t size,
							size_t n, void* client_data)
{
	return fwrite(data, size, n, (FILE*)client_data);
}

static int file_seek(long int offset, int whence, void* client_data)
{
	return fseek((FILE*)client_data, offset, whence);
}

static long int file_tell(void* client_data)
{
	return ftell((FILE*)client_data);
}

const ceio_callbacks CEIO_CALLBACKS_FILE = {
	file_close, file_read, file_write, file_seek, file_tell
};
