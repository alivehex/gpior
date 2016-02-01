#ifndef _VFS_H_
#define _VFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mtd.h"

#define FS_TYPE_DIR		0
#define FS_TYPE_FILE	1

struct __FILE {
	void *handle;
	enum fs_t fstype;
	int isdir;
	long fsize;
};

struct dirent {
	int ftype;
	long fsize;
	char name[128];
	char full_path[256];
};

typedef struct {
	void *handle;
	struct dirent *fnode;
	enum fs_t fstype;
	char name[128];	
} DIR;

extern int format(const char *devname);
extern void stdio_init(void);
extern int mount(const char *devname, const char *path, int readonly);
extern int unmount(const char *mount_point);
extern FILE *fopen(const char *fname, const char *mode);
extern int fclose(FILE *fh);
extern size_t fread( void * ptr, size_t size, size_t count, FILE * stream);
extern size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream);
extern int fseek(FILE * stream, long int offset, int origin);
extern long int ftell(FILE * stream);
extern int fflush(FILE * stream);
extern int feof(FILE * stream);


#endif /* ifndef _VFS_H_ */
