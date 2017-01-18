/* compress_zlib.c */
#include "postgres.h"
#include "fstream/gfile.h"
#include "storage/bfz.h"
#include <zlib.h>

/* Use the one defined in fileam.c */
extern void* gfile_malloc(size_t size);
extern void gfile_free(void*a);

struct bfz_zlib_freeable_stuff
{
	struct bfz_freeable_stuff super;

	/* handle for the compressed file */
	gfile_t *gfile;
};

typedef struct FileObject {
	FileAccessInterface file_access;
	File file;
} FileObject;

static ssize_t
read_file(FileAccessInterface *file_access, void *ptr, size_t size)
{
	Assert(file_access);
	Assert(PostgresFileObject == file_access->ftype);
	FileObject* fobj = (FileObject*)file_access;
	return FileRead(fobj->file, ptr, size);
}

static ssize_t
write_file(FileAccessInterface *file_access, void *ptr, size_t size)
{
	Assert(file_access);
	Assert(PostgresFileObject == file_access->ftype);
	FileObject* fobj = (FileObject*)file_access;
	return FileWrite(fobj->file, ptr, size);
}

static void
close_file(FileAccessInterface *file_access)
{
	/* don't call the `FileClose`. BFZ owns File and it knows when to close it.*/
	return;
}

static FileAccessInterface*
gfile_create_fileobj_access(File file, GFileAlloca gfile_alloca)
{
	FileObject *fobj = gfile_alloca(sizeof(FileObject));
	fobj->file_access.ftype = PostgresFileObj;
	fobj->file_access.read_file = read_file;
	fobj->file_access.write_file = write_file;
	fobj->file_access.close_file = close_file;
	fobj->file = file;
	return &fobj->file_access;
}


/* This file implements bfz compression algorithm "zlib". */

/*
 * bfz_zlib_close_ex
 *	Close buffers etc. Does not close the underlying file!
 */
static void
bfz_zlib_close_ex(bfz_t *thiz)
{
	struct bfz_zlib_freeable_stuff *fs = (void *) thiz->freeable_stuff;

	if (NULL != fs)
	{
		Assert(NULL != fs->gfile);

		gfile_close(fs->gfile);
		gfile_destroy(fs->gfile);
		fs->gfile = NULL;

		pfree(fs);
		thiz->freeable_stuff = NULL;
	}
}

/*
 * bfz_zlib_write_ex
 *	 Write data to an opened compressed file.
 *	 An exception is thrown if the data cannot be written for any reason.
 */
static void
bfz_zlib_write_ex(bfz_t *thiz, const char *buffer, int size)
{
	struct bfz_zlib_freeable_stuff *fs = (void *) thiz->freeable_stuff;

	ssize_t written = gfile_write(fs->gfile, (void *)buffer, size);
	if (written < 0)
	{
		ereport(ERROR,
				(errcode(ERRCODE_IO_ERROR),
				errmsg("could not write to temporary file: %m")));
	}
}

/*
 * bfz_zlib_read_ex
 *	Read data from an already opened compressed file.
 *
 *	The buffer pointer must be valid and have at least size bytes.
 *	An exception is thrown if the data cannot be read for any reason.
 *
 * The buffer is filled completely.
 */
static int
bfz_zlib_read_ex(bfz_t *thiz, char *buffer, int size)
{
	struct bfz_zlib_freeable_stuff *fs = (void *) thiz->freeable_stuff;

	ssize_t read = gfile_read(fs->gfile, buffer, size);
	if (read < 0)
	{
		ereport(ERROR,
				(errcode(ERRCODE_IO_ERROR),
				errmsg("could not read from temporary file: %m")));
	}

	return read;
}

/*
 * bfz_zlib_init
 *	Initialize the zlib subsystem for a file.
 *
 *	The underlying file descriptor fd should already be opened
 *	and valid. Memory is allocated in the current memory context.
 */
void
bfz_zlib_init(bfz_t * thiz)
{
	Assert(TopMemoryContext == CurrentMemoryContext);
	struct bfz_zlib_freeable_stuff *fs = palloc(sizeof *fs);
	bool is_write = thiz->mode == BFZ_MODE_APPEND;
	fs->gfile = gfile_create(GZ_COMPRESSION, is_write, gfile_malloc, gfile_free);
	FileAccessInterface* file_access = gfile_create_fileobj_access(thiz->file, gfile_malloc);
	int res = gz_file_open(fs->gfile, file_access);

	if (res == 1)
	{
		ereport(ERROR,
				(errcode(ERRCODE_IO_ERROR),
				errmsg("gz_file_open failed: %m")));
	}

	thiz->freeable_stuff = &fs->super;
	fs->super.read_ex = bfz_zlib_read_ex;
	fs->super.write_ex = bfz_zlib_write_ex;
	fs->super.close_ex = bfz_zlib_close_ex;
}
