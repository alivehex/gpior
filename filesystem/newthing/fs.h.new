#ifndef _VFS_H_
#define _VFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mtd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLD_VFS_SUPPORT	0
#define SD_NAME			"mmc"
#define NAND0_NAME		"nand0"
#define NAND1_NAME		"nand1"
#define NAND2_NAME		"nand2"

#define FS_TYPE_DIR		0
#define FS_TYPE_FILE	1


#ifndef wint_t
typedef int wint_t;
#endif

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
	char name[255];	
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
extern int file_exists(const char *fn);
extern int rename ( const char * oldname, const char * newname );
extern int remove(const char *pathname);
extern int mkdir(const char *dirp);
extern int rmdir(const char *dirp);
extern DIR *opendir(const char *dirp);
extern struct dirent *readdir(DIR *dir);
extern int closedir(DIR *dir);


#if (OLD_VFS_SUPPORT)
typedef enum {
	FS_DTYPE_FILE,					/**< dirent type file */
	FS_DTYPE_DIR,					/**< dirent type dir */
} FS_DTYPE;

typedef struct {  
    unsigned short d_reclen;	
    char *d_name;				
	char d_type;
} FS_DIRENT;

typedef struct {
	DIR *std_dir;
	FS_DIRENT *fs_dirent;
} FS_DIR;


extern int umount(const char *path);
extern int vfopen(const char *path, const char *mode);
extern int vfread(int fh, void *buf, unsigned long len);
extern int vfwrite(int fh, const void *buf, unsigned long len);
extern int vfclose(int fh);
extern long vfseek(int fh, long offset, int dir);
extern long vftell(int fh);
extern void vfsync(int fh);
extern int vfgetc(int fh);
extern int vfgetwc(int f);
extern char *vfgetws(char *s, int n, int f);
extern FS_DIR *vopendir(const char *path);
extern FS_DIRENT *vreaddir(FS_DIR *dir);
extern int vclosedir(FS_DIR *dir);
extern int vmkdir(const char *path);
extern int vrmdir(const char *path);
extern int vrmfile(const char *path) ;
extern int vrename(const char *oldPath, const char *newPath);
extern int vfeof(int f);

#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef _VFS_H_ */
