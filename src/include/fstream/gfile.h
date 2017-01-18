#ifndef GFILE_H
#define GFILE_H

/*#include "c.h"*/
#include <sys/types.h>
#ifndef WIN32
#include <bzlib.h>
#include <zlib.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif
#ifdef WIN32
#ifndef _WIN64
typedef long ssize_t;
#else
typedef _int64 ssize_t;
#endif
#endif

#ifdef WIN32
typedef BOOL bool_t;
#else
typedef char bool_t;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#endif

struct gpfxdist_t;

typedef enum Compression_type
{
	NO_COMPRESSION = 0,
	GZ_COMPRESSION,
	BZ_COMPRESSION
} compression_type;

typedef enum FileAccessType
{
	Invalid = 0,
	CFileObj,
	PostgresFileObj
} FileAccessType;

typedef enum GFileErrorCode
{
	GF_NoError = 0,
	GF_AccessError = -1,
	GF_BZDecompressInitError = -2,
	GF_BZDecompressError = -3,
	GF_InflateInit2Error = -4,
	GF_InflateError = -5,
	GF_DeflateInit2Error = -6,
	GF_DeflateError = -7,
	GF_OutOfMemory = -8
} GFileErrorCode;

typedef struct FileAccessInterface
{
	FileAccessType ftype;
	ssize_t (*read_file)(struct FileAccessInterface *fileaccess, void *ptr, size_t size);
	ssize_t (*write_file)(struct FileAccessInterface *fileaccess, void *ptr, size_t size);
	void (*close_file)(struct FileAccessInterface *fileaccess);
} FileAccessInterface;

typedef struct gfile_t gfile_t;
typedef void*(*GFileAlloca)(size_t size);
typedef void(*GFileFree)(void*);

/*
 * MPP-13817 (support opening files without O_SYNC)
 */
int gfile_open_flags(int writing, int usesync);
#define GFILE_OPEN_FOR_READ  	    0
#define GFILE_OPEN_FOR_WRITE_NOSYNC 1
#define GFILE_OPEN_FOR_WRITE_SYNC   2

gfile_t* gfile_create(compression_type compression, bool_t is_write, GFileAlloca gfile_alloca, GFileFree gfile_free);
void gfile_destroy(gfile_t* fd);
int gfile_open(gfile_t* fd, const char* fpath, int flags, int* response_code, const char** response_string, struct gpfxdist_t* transform);
void gfile_close(gfile_t*fd);
off_t gfile_get_compressed_size(gfile_t*fd);
off_t gfile_get_compressed_position(gfile_t*fd);
bool_t gfile_is_win_pipe(gfile_t* fd);
ssize_t gfile_read(gfile_t* fd, void* ptr, size_t len); /* gfile_read reads as much as it can--short read indicates error. */
ssize_t gfile_write(gfile_t* fd, void* ptr, size_t len);
void gfile_printf_then_putc_newline(const char*format,...) __attribute__ ((__format__ (__printf__, 1, 0)));
int gz_file_open(gfile_t *fd, FileAccessInterface* file_access);
GFileErrorCode gfile_error_code(gfile_t *fd);
void gfile_reset_error(gfile_t *fd);
bool_t gfile_has_error(gfile_t *fd);
char* gfile_error_detail(gfile_t *fd);
#endif
