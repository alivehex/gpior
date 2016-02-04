#include "fs.h"
#include "mtd.h"

#include <string.h>

#if (MTD_FATFS)
#include "ff.h"
#include "ffconf.h"
#include "sdspi.h"
#endif

#if (MTD_YAFFS2)
#include "yaffsfs.h"
#include "yaffs_list.h"
#endif

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "VFS"
// #include "trace.h"


// #include "hex_uart.h"
// #define STDOUT_UART_BAUDRATE	921600
// #define STDOUT_UART_CHANNEL		2
// #define STDOUT_UART_LBUFLEN		2048

// #define STDOUT_UART_INIT()	UART_open(STDOUT_UART_CHANNEL, \
// 	STDOUT_UART_BAUDRATE, STDOUT_UART_LBUFLEN)
	
	
FILE __stdin  = { NULL, FS_TYPE_STDIN,  0, 0 };
FILE __stdout = { NULL, FS_TYPE_STDOUT, 0, 0 };
FILE __stderr = { NULL, FS_TYPE_STDERR, 0, 0 };

#if (MTD_FATFS)
extern int ff_save_mtddev(int index, mtd_dev_t *dev);
static int fatfs_dev_index = 0;
#endif

#if (MTD_YAFFS2)
extern int yaffs_format(struct yaffs_dev *dev);
#endif

/*
** @ Get ROOT path in an full file path
**/
static int get_file_path(const char *fpath, char *rpath, int len) {
	int i;
	
	for (i = 0; i < len; i ++) {
		if (fpath[i] == '/') {
			if (i == 0) {
				rpath[i] = fpath[i];
				continue;
			} 
			else {
				rpath[i] = 0;
				return i;
			}			
		}
		else {
			rpath[i] = fpath[i];
		}
	}
	rpath[i] = 0;
	return i;
}

/*
** @ convert STDFLAG to FFLAG
** @ return 1 if seek needed, else 0
**/
static int stdflg_to_ffflg(const char *mode, unsigned int *ff_flag) {
#if (MTD_FATFS)	
	unsigned int seek_end = 0;
	
	*ff_flag = 0;
	while ( *mode != 0 ) {
		switch(*mode ++) {
		case 'r':
			*ff_flag |= (FA_READ | FA_OPEN_EXISTING);
		break;
		case 'w':
			*ff_flag |= FA_WRITE;
			*ff_flag &= ~(FA_OPEN_ALWAYS);
			*ff_flag |= FA_CREATE_ALWAYS;
		break;
		case 'a':
			*ff_flag |= FA_WRITE;
			if ((*ff_flag & FA_CREATE_ALWAYS) == 0) {
				*ff_flag |= FA_OPEN_ALWAYS;
			}
			seek_end = 1;
		break;
		case '+':
			*ff_flag |= (FA_READ | FA_WRITE);
		break;
		}
	}	
	return seek_end;
#else
	return 0;
#endif
}

/*
** @ convert STDFLAG to YAFFSFLAG 
**/
#if (MTD_YAFFS2)
static int stdflg_to_yfsflg(const char *mode, unsigned int *yoflag, unsigned int *ymode) {

	*yoflag = 0;
	*ymode = 0;
	
	while ( *mode != 0 ) {
		switch(*mode ++) {
		case 'r':
			/* this will clear the O_WRONLY and O_RDWR */
			*yoflag &= ~0xF;
			*ymode |= (S_IREAD); // | S_IWRITE);
		break;
		case 'w':
			*yoflag |= (O_WRONLY | O_TRUNC | O_CREAT);
			*ymode |= (S_IREAD | S_IWRITE);
		break;
		case 'a':
			*yoflag |= (O_WRONLY | O_APPEND);
			*ymode |= (S_IREAD | S_IWRITE);
		break;
		case '+':
			*yoflag |= O_RDWR;
			*yoflag &= ~(O_WRONLY);
			*ymode |= (S_IWRITE | S_IREAD);
		break;
		}
	}
	return 0;
}
#endif

/*
** @ mount a device by device name
**/
int mount(const char *devname, const char *path, int readonly) {
	mtd_dev_t *dev;
	
	/* get device by device name */
	dev = mtd_search_by_devname(devname);
	
	if (dev) {
		/* record the mount point */
		strcpy(dev->mount_point, path);
		
		switch (dev->fstype) {
#if (MTD_FATFS)
		/* FATFS mount */
		case FS_TYPE_FAT:
			/* max driver number */
			if (fatfs_dev_index == _DRIVES) {
				return -1;
			}
			
			/* malloc space for FATFS struct */
			dev->device = malloc(sizeof(FATFS));
			if (dev == NULL) {
				return -1;
			}
			
			/* do f_mount for FATFS */
			if (f_mount(fatfs_dev_index, (FATFS *)dev->device) != FR_OK) {
				return -1;
			}
			
			/* save the mtd device for FATFS io functions */
			if (ff_save_mtddev(fatfs_dev_index, dev) != 0) {
				return -1;
			}
			dev->fatdev_index = fatfs_dev_index;
			dev->readonly = readonly;
			dev->is_mounted = TRUE;
			fatfs_dev_index ++;
		break;
#endif

#if (MTD_YAFFS2)	
		/* YAFFS mount */
		case FS_TYPE_YAFFS:
			/* save device name */
			dev->flash_dev.param.name = malloc(strlen(dev->mount_point));
			strcpy(dev->flash_dev.param.name, &dev->mount_point[1]);
			
			/* add device */
			yaffs_add_device(&dev->flash_dev);
			
			/* mount device */
			yaffs_mount2(dev->mount_point, readonly);
			dev->readonly = readonly;
			dev->is_mounted = TRUE;
		break;
#endif
		default:
			return -1;
		}
		return 0;
	}
	else {
		return -1;
	}
}

/*
** @ unmount a device by mount point
**/
int unmount(const char *mount_point) {
	mtd_dev_t *dev;
	
	dev = mtd_search_by_mntpt(mount_point);
	if (dev) {
		switch(dev->fstype) {
#if (MTD_FATFS)
		case FS_TYPE_FAT:
			if (f_mount(dev->fatdev_index, NULL) != FR_OK) {
				return -1;
			}
			else {
				fatfs_dev_index --;
			}
			if (dev->device) {
				free(dev->device);
			}
		break;
#endif
			
#if (MTD_YAFFS2)
		case FS_TYPE_YAFFS:
			if (yaffs_unmount(dev->mount_point) != 0) {
				return -1;
			}
		break;	
#endif
		default:
			return -1;
		}
		memset(dev->mount_point, 0, sizeof(dev->mount_point));
		dev->is_mounted = FALSE;
#if (MTD_YAFFS2)
		if (dev->fstype == FS_TYPE_YAFFS) {
			if (dev->flash_dev.param.name) 
				free(dev->flash_dev.param.name);
		}
#endif
		return 0;
	}
	else {
		return -1;
	}
}

/*
** @ open file by file name and mode
** @ mode: r | w | a | +
** @ example: fopen("/mmc/readonly.bin", "r");
**/
FILE *fopen(const char *fname, const char *mode) {
	char root_point[255];
	FILE *fh;
	int n;
	
#if (MTD_FATFS)
	unsigned int ff_flag;
	unsigned int ff_seekend;
#endif

#if (MTD_YAFFS2)	
	unsigned int yoflg;
	unsigned int ymode;
#endif
	
	mtd_dev_t *dev;
	
	/* get file top path */
	n = get_file_path(fname, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	/* malloc space for file struct */
	fh = malloc(sizeof(FILE));
	if (fh == NULL) {
		return fh;
	}	
	
	switch (dev->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT:	
		/* malloc space for FATFS FIL struct */
		fh->handle = malloc(sizeof(FIL));
		if (fh == NULL) {
			return NULL;
		}
		
		/* convert the file open flags */
		ff_seekend = stdflg_to_ffflg(mode, &ff_flag);
		
		/* open file with FATFS */
		if (f_open(fh->handle, &fname[n], ff_flag) != FR_OK) {
			free(fh->handle);
			free(fh);
			return NULL;
		}
		
		/* seek end if needed */
		if (ff_seekend) {
			f_lseek(fh->handle, (unsigned long)((FIL *)fh->handle)->fsize);
		}
		
		/* resave file size and file system type */
		fh->fsize = ((FIL *)fh->handle)->fsize;
		fh->fstype = FS_TYPE_FAT;
		fh->isdir = FALSE;
		return fh;
#endif

#if (MTD_YAFFS2)		
	/* YAFFS */
	case FS_TYPE_YAFFS:	
		stdflg_to_yfsflg(mode, &yoflg, &ymode);
		
		fh->handle = (void *)yaffs_open(fname, yoflg, ymode);
		
		if ((int)fh->handle < 0) {
			break;
		}
		fh->fsize = yaffs_fsize((int)fh->handle);
		fh->fstype = FS_TYPE_YAFFS;
		fh->isdir = FALSE;
		return fh;
#endif
	default:
		break;
	}
	
	if (fh) {
		free(fh);
	}
	return NULL;
}

/*
** @ fdopen() dummy yet
**/
FILE *fdopen(int fh, char *mode) {
	return NULL;
}

/*
** @ Close file by FILE *
**/
//extern int closePrevSdReadMultipleBlocksTransfer(void);
int fclose(FILE *fh) { 
	if (fh == NULL) 
		return -1;
	
	switch (fh->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		f_close((FIL *)fh->handle);
		if (fh->handle)
			free(fh->handle);
		if (fh)
			free(fh);
		
		// Due to lasy stop tansfer command
//		if (closePrevSdReadMultipleBlocksTransfer() != 0) {
//			return -1;
//		} else {
//			return 0;
//		}
		return 0;
#endif
	
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		yaffs_close((int)fh->handle);
		break;
#endif
	default:
		break;
	}
	return -1;
}

/*
** @ Read block of data from stream 
**/
size_t fread( void * ptr, size_t size, size_t count, FILE * stream) {
	unsigned int nbytes = 0;
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		f_read(stream->handle, ptr, size * count, &nbytes);
		return nbytes;
#endif
	
#if (MTD_YAFFS2)
	/* YAFFS */
	case FS_TYPE_YAFFS:
		nbytes = yaffs_read((int)stream->handle, ptr, size * count);
		break;
#endif
	
	default:
		break;
	}
	return nbytes;
}

/*
** @ Write block of data to stream
**/
size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream) {
	unsigned int nbytes = 0;
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		f_write(stream->handle, ptr, size * count, &nbytes);
		break;
#endif
	
#if (MTD_YAFFS2)
	/* YAFFS */
	case FS_TYPE_YAFFS:
		nbytes = yaffs_write((int)stream->handle, ptr, size * count);
		break;
#endif
	default:
		return 0;
	}
	if (nbytes != (count * size)) {
		return nbytes;
	}
	return nbytes;
}

/*
** @ Reposition stream position indicator
** @ offset: Number of bytes to offset from origin
** @ origin: SEEK_SET SEEK_CUR SEEK_END
**/
int fseek(FILE * stream, long int offset, int origin) {	
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		switch(origin) {
		case SEEK_SET:
		break;
		case SEEK_CUR:
			offset = offset + ((FIL *)stream->handle)->fptr;
		break;
		case SEEK_END:
			offset = offset + stream->fsize ;
		break;
		}
		if (f_lseek(stream->handle, offset) != FR_OK) {
			return -1;
		}
		break;
#endif
		
#if (MTD_YAFFS2)
	/* YAFFS */
	case FS_TYPE_YAFFS:
		if ((int)yaffs_lseek((int)stream->handle, offset, origin) == -1) {
			return -1;
		}
		break;
#endif
	default:
		return -1;
	}
	return 0;
}

/*
** @ Returns the current value of the position indicator of the stream
**/
long int ftell(FILE * stream) {
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		return ((FIL *)stream->handle)->fptr;
#endif
	
#if (MTD_YAFFS2)	
	/* YAFFS */
	case FS_TYPE_YAFFS:
		return yaffs_tell((int)stream->handle);
#endif
	default:
		break;
	}
	return -1;
}

/*
** @ Flush stream
**/
int fflush(FILE * stream) {
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)	
	/* FATFS */
	case FS_TYPE_FAT: 
		if (f_sync(stream->handle) != FR_OK) {
			return -1;
		}
		break;
#endif

#if (MTD_YAFFS2)
	/* YAFFS */
	case FS_TYPE_YAFFS:
		if (yaffs_fsync((int)stream->handle) == -1) {
			return -1;
		}
		break;
#endif
	default:
		return -1;
	}
	return 0;
}

/*
** @ Check End-of-File indicator
**/
int feof(FILE * stream) {
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT: 
		return ((((FIL *)(stream->handle))->fptr) == (stream->fsize) ? 1 : 0);
#endif
	
#if (MTD_YAFFS2)
	/* YAFFS */
	case FS_TYPE_YAFFS:
		return ((ftell(stream) == yaffs_fsize((int)stream->handle)) ? 1 : 0);
#endif
	default:
		break;
	}
	return -1;
}

/*
** @ Rename file
**/
int rename ( const char * oldname, const char * newname ) {
	char root_point[128];
	int n;
	mtd_dev_t *dev;
	
	/* get file top path */
	n = get_file_path(oldname, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	switch (dev->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT:	
		if (f_rename(&oldname[n], &newname[n]) != FR_OK) {
			return -1;
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		if (yaffs_rename(oldname, newname) != 0) {
			return -1;
		}
	break;
#endif
		
	default:
		return -1;
	}
	return 0;
}

/*
** @ Remove file
**/
int remove(const char *pathname) {
	char root_point[128];
	int n;
	mtd_dev_t *dev;
	
	/* get file top path */
	n = get_file_path(pathname, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	switch (dev->fstype) {
#if (MTD_FATFS)
	/* FATFS */
	case FS_TYPE_FAT:
		if (f_unlink(&pathname[n]) != FR_OK) {
			return -1;
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		if (yaffs_unlink(pathname) != 0) {
			return -1;
		}
	break;
#endif
	default:
		return -1;
	}
	return 0;
}

/*
** @ Get character from stream
**/
int fgetc (FILE * stream) {
	unsigned char buffer;
	
	if (stream == NULL) 
		return 0;
	
	switch (stream->fstype) {
#if (MTD_FATFS || MTD_YAFFS2)
	/* FATFS */
	case FS_TYPE_FAT:
	case FS_TYPE_YAFFS:
		if (fread(&buffer, 1, sizeof(buffer), stream) != sizeof(buffer)) {
			return -1;
		}
		break;
#endif
		
#if (MTD_STDIN)
	/* STDIN */
	case FS_TYPE_STDIN:
 		buffer = STD_getc();
		break;
#endif
	default:
		return -1;
	}
	return buffer;
}

/*
** @ Get wide character from stream (2bytes)
**/
wint_t fgetwc(FILE *stream) {
	wint_t buffer;
	
	if (fread((char *)&buffer, 1, sizeof(wint_t), stream) != sizeof(wint_t)) {
		return -1;
	} 
	else {
		return buffer;
	}
}

/*
** @ Get wide string from stream
**/
wchar_t *fgetws(wchar_t *ws, int n, FILE *stream) {
	wchar_t *ptr = ws;
	
	while (-- n) {
		if (fread((char *)ptr, 1, sizeof(wchar_t), stream) != sizeof(wchar_t)) {
			return NULL;
		}
		if (*(ptr ++) == L'\n') {
			break;
		}
	}
	*ptr = L'\0';
	return ws; 
}

/*
** @ Get string from stream
**/
char *fgets ( char * str, int num, FILE * stream ) {
	int c;
	int count = 0;
	char *buf = str;
	
	/* skip the /r and /n */
	do {
		c = fgetc(stream);
		if(c == -1) {
			return NULL;
		}
	} while((c == 0x0A) || (c == 0x0D)); 
	
	/* read word */
	do {
		*(buf ++) = (char)c;
		count ++;
		c = fgetc(stream);
	} while((c != 0x0A) && (c != 0x0D) && (c != -1) && (count < num));
	*buf = 0;
	return str;
}

/*
** @ Check error indicator, dummy yet
**/
int ferror(FILE *f) {
	return 0;
}

/*
** @ called after reading a character from the stream, dummy yet
**/
int __backspace(FILE *stream) {
	return 0;
}

/*
** @ printf for the file, dummy yet
**/
int fprintf ( FILE * stream, const char * format, ... ) {
	return 0;
}

/*
** @ Put character to a file
**/
int fputc ( int character, FILE * stream ) {
#if (MTD_STDOUT)
	unsigned char cBuf = character;
#endif
	
	switch (stream->fstype) {
	/* FATFS */
	case FS_TYPE_FAT:
		break;
	/* YAFFS */
	case FS_TYPE_YAFFS:
		break;
	/* STDOUT */
#if (MTD_STDOUT)
	case FS_TYPE_STDOUT:
		STD_putc(cBuf);
		break;
#endif
	}
	return character;
}

/*
** @ Put string to a file
**/
int fputs ( const char * str, FILE * stream ) {
	switch (stream->fstype) {
	/* FATFS */
	case FS_TYPE_FAT:
		break;
	case FS_TYPE_YAFFS:
		break;
	case FS_TYPE_STDOUT:
// 		UART_write(STDOUT_UART_CHANNEL, (char *)str, strlen(str));
		break;
	}
	return 0;
}

int file_exists(const char *fn) {
	int ret = 0;
	
	FILE *fh = fopen(fn, "r");
	if (fh)
		ret = 1;
	fclose(fh);
	return ret;
}

/*
** @ Reopen a file, dummy yet
**/
FILE * freopen ( const char * filename, const char * mode, FILE * stream ) {
	return NULL;
}

/*
** @ Scanf for the file, dummy yet
**/
int fscanf ( FILE * stream, const char * format, ... ) {
	return 0;
}

/*
** @ Set file position, dummy yet
**/
int fsetpos ( FILE * stream, const fpos_t * pos ) {
	return 0;
}

/*
** @ Make a directory
**/
int mkdir(const char *dirp) {
	char root_point[128];
	int n;
	mtd_dev_t *dev;
	
	/* get file top path */
	n = get_file_path(dirp, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	switch(dev->fstype) {
#if (MTD_FATFS)
	case FS_TYPE_FAT:
		if (f_mkdir(&dirp[n]) != FR_OK) {
			return -1;
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		if (yaffs_mkdir(dirp, (S_IREAD | S_IWRITE)) == -1) {
			return -1;
		}
	break;
#endif
	default:
		return -1;
	}
	return 0;
}

/*
** @ Remoce a direcory
**/
int rmdir(const char *dirp) {
	char root_point[128];
	int n;
	mtd_dev_t *dev;
	
	/* get file top path */
	n = get_file_path(dirp, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	switch(dev->fstype) {
#if (MTD_FATFS)
	case FS_TYPE_FAT:
		if (f_unlink(&dirp[n]) != FR_OK) {
			return -1;
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		if (yaffs_rmdir(dirp) == -1) {
			return -1;
		}
	break;
#endif
	default:
		return -1;
	}
	return 0;
}

/*
** @ Open the directory
**/
DIR *opendir(const char *dirp) {
	char root_point[128];
	int n;
	mtd_dev_t *dev;
	DIR *dir = NULL;
	char *nptr;
	
	/* get file top path */
	n = get_file_path(dirp, root_point, sizeof(root_point));
	
	/* get device by mount pointer */
	dev =  mtd_search_by_mntpt(root_point);
	if (dev == NULL) {
		return NULL;
	}
	
	/* malloc an directory */
	dir = malloc(sizeof(DIR));
	if (dir == NULL) { 
		return NULL;
	}
	memset(dir, 0, sizeof(DIR));
	
	switch(dev->fstype) {
#if (MTD_FATFS)
	case FS_TYPE_FAT:
		/* malloc and FATDIR handle */
		dir->handle = malloc(sizeof(FATDIR));
		if (dir->handle == NULL) {
			free(dir);
			return NULL;
		}
		
		if (strcmp(root_point, dirp) == 0) {
			root_point[1] = 0;
			nptr = root_point;
		}
		else {
			nptr = (char *)&dirp[n];
		}
		
		if (f_opendir ((FATDIR *)dir->handle, nptr) != FR_OK) {
			free(dir->handle);
			free(dir);
			return NULL;
		}
		else {
			dir->fstype = FS_TYPE_FAT;
			strcpy(dir->name, dirp);
			dir->fnode = NULL;
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		dir->handle = yaffs_opendir(dirp);
		if (dir->handle == NULL) {
			free(dir);
			return NULL;
		}
		else {
			dir->fstype = FS_TYPE_YAFFS;
			strcpy(dir->name, dirp);
			dir->fnode = NULL;
		}		
	break;
#endif
	
	default:
		return NULL;
	}
	return dir;
}

/*
** @ Read a directory
**/
struct dirent *readdir(DIR *dir) {
#if (MTD_FATFS)
	FILINFO finfo;
#endif
	FILE *fh;
#if (MTD_YAFFS2)
	struct yaffs_dirent *ydirinfo = NULL;
#endif	
	struct dirent *dirinfo = NULL;
	
	switch(dir->fstype) {
#if (MTD_FATFS)
	case FS_TYPE_FAT:
		if (f_readdir((FATDIR *)dir->handle, &finfo) != FR_OK) {
			return NULL;
		} 
		else {
			if (dir->fnode == NULL) {
				dir->fnode = malloc(sizeof(struct dirent));
			}
#if _USE_LFN
			if ( (strlen(finfo.lfname) == 0) && (strlen(finfo.fname) == 0) ) {
				if(dir->fnode) {
					free(dir->fnode);
					dir->fnode = NULL;
				}
			} else {
				if (strlen(finfo.lfname) == 0) {
					strcpy(dir->fnode->name, finfo.fname);
				} else {
					strcpy(dir->fnode->name, finfo.lfname);
				}
				
				if (finfo.fattrib & AM_DIR) {
					dir->fnode->ftype = FS_TYPE_DIR;
				} 
				else {
					dir->fnode->ftype = FS_TYPE_FILE;
				}
				dir->fnode->fsize = finfo.fsize;
				dirinfo = dir->fnode;
			}
#else
			if (finfo.fname[0] == 0) {
				if(dir->fnode) {
					free(dir->fnode);
					dir->fnode = NULL;
				}
			} else {
				strcpy(dir->fnode->name, finfo.fname);
				
				if (finfo.fattrib & AM_DIR) {
					dir->fnode->ftype = FS_TYPE_DIR;
				} 
				else {
					dir->fnode->ftype = FS_TYPE_FILE;
				}
				dir->fnode->fsize = finfo.fsize;
				dirinfo = dir->fnode;
			}
#endif			
		}		
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		ydirinfo = yaffs_readdir((yaffs_DIR *)dir->handle);
		if (ydirinfo == NULL) {
			return NULL;
		}
		else {
			if (dir->fnode == NULL) {
				dir->fnode = malloc(sizeof(struct dirent));
			} 
			if (ydirinfo->d_name[0] == 0) {
				if (dir->fnode) {
					free(dir->fnode);
					dir->fnode = NULL;
				}
			} else {
				strcpy(dir->fnode->name, ydirinfo->d_name);
// 				These do not word!
// 				if (ydirinfo->d_type & DT_DIR) {
// 					dir->fnode->ftype = FS_TYPE_DIR;
// 				} 
// 				else {
// 					dir->fnode->ftype = FS_TYPE_FILE;
// 				}
				dirinfo = dir->fnode;
			}
		}		
	break;
#endif
	default:
		break;
	}
	
	if (dirinfo) {
		strcpy(dirinfo->full_path, dir->name);
		strcat(dirinfo->full_path, "/");
		strcat(dirinfo->full_path, dirinfo->name);
		
		/* do something with YAFFS2 */
		if (dir->fstype == FS_TYPE_YAFFS) {
			fh = fopen(dirinfo->full_path, "r");
			if (fh) {
			/* it's a file */
				dir->fnode->ftype = FS_TYPE_FILE;
				dir->fnode->fsize = fh->fsize;
			} 
			else {
			/* it's a directory */
				dir->fnode->ftype = FS_TYPE_DIR;
				dir->fnode->fsize = 0;
			}
			fclose(fh);
		}
	}
	return dirinfo;
}

/*
** @ Close a directory
**/
int closedir(DIR *dir) {
	
	if (dir == NULL)
		return -1;
	
	switch (dir->fstype) {
#if (MTD_FATFS)
	case FS_TYPE_FAT:
		if (dir->handle) {
			free(dir->handle);
		}
	break;
#endif
		
#if (MTD_YAFFS2)
	case FS_TYPE_YAFFS:
		yaffs_closedir((yaffs_DIR *)dir->handle);
	break;
#endif
	default:
		break;
	}

	if (dir->fnode) {
		free(dir->fnode);
	}
	
	if (dir) {
		free(dir);
	}
	
	return 0;
}

/*
** @ format a device
**/
int format(const char *devname) {
	mtd_dev_t *dev;
	
	/* get device by device name */
	dev = mtd_search_by_devname(devname);
	
	if (dev) {
		switch(dev->fstype) {
#if (MTD_FATFS)
		case FS_TYPE_FAT:
			if (f_mkfs(dev->fatdev_index, 0, 2048) != FR_OK) {
				return -1;
			}				
		break;
#endif
			
#if (MTD_YAFFS2)
		case FS_TYPE_YAFFS:
			if (yaffs_format(&dev->flash_dev) != 0) {
				return -1;
			}
		break;
#endif
		default:
			return -1;
		}
	}
	return 0;
}

/*
** @ vFS support
**/
#if (OLD_VFS_SUPPORT)

#define MAX_FILE_HANDLE		32
#define FHINDEX_SHITF		4

typedef struct {
	FILE *fh;
	int used;
} fsh_t;

static fsh_t FSH[MAX_FILE_HANDLE] = {0};

int umount(const char *path) {
	return unmount(path);
}

int vfopen(const char *path, const char *mode) {
	int i;
	FILE *fh;
	
	fh = fopen(path, mode);
	if (fh == NULL) {
		return -1;
	} 
	else {
		for(i = FHINDEX_SHITF; i < MAX_FILE_HANDLE; i ++) {
			if (FSH[i].used == 0) {
				FSH[i].fh = fh;
				FSH[i].used = TRUE;
				return i;
			}
		}			
		/* get nothing */
		return -1;
	}
}

int vfread(int fh, void *buf, unsigned long len) {
	return fread(buf, 1, len, FSH[fh].fh);
}

int vfwrite(int fh, const void *buf, unsigned long len) {
	return fwrite(buf, 1, len, FSH[fh].fh);
}

int vfclose(int fh) {
	int err;
	
	err = fclose(FSH[fh].fh);
	FSH[fh].fh = NULL;
	FSH[fh].used = 0;
	return err;
}

long vfseek(int fh, long offset, int dir) {
	return fseek(FSH[fh].fh, offset, dir);
}

long vftell(int fh) {
	return ftell(FSH[fh].fh);
}

void vfsync(int fh) {
	fflush(FSH[fh].fh);
}

int vfgetc(int fh) {
	return fgetc(FSH[fh].fh);
}

int vfgetwc(int fh) {
	return fgetwc(FSH[fh].fh);
}

char *vfgetws(char *s, int n, int fh) {
	return (char *)fgetws((wchar_t *)s, n, FSH[fh].fh);
}

FS_DIR *vopendir(const char *path) {
	FS_DIR *dir = NULL;
	
	dir = malloc(sizeof(FS_DIR));
	if (dir == NULL) {
		return NULL;
	}
	dir->std_dir = opendir(path);
	if (dir->std_dir == NULL) {
		free(dir);
		return NULL;
	}
	dir->fs_dirent = NULL;
	return dir;
}

FS_DIRENT *vreaddir(FS_DIR *dir) {
	struct dirent *dirinfo;
	
	dirinfo = readdir(dir->std_dir);
	if (dirinfo == NULL) {
		return NULL;
	}
	if (dir->fs_dirent == NULL) {
		dir->fs_dirent = malloc(sizeof(FS_DIRENT));
	}
	if (dir->fs_dirent == NULL) {
		return NULL;
	}
	dir->fs_dirent->d_reclen = sizeof(FS_DIRENT);
	dir->fs_dirent->d_name = dirinfo->name;
	if (dirinfo->ftype == FS_TYPE_DIR) {
		dir->fs_dirent->d_type = FS_DTYPE_DIR;
	}
	else {
		dir->fs_dirent->d_type = FS_DTYPE_FILE;
	}
	return dir->fs_dirent;
}

int vclosedir(FS_DIR *dir) {
	if (dir->fs_dirent) {
		free(dir->fs_dirent);
	}
	closedir(dir->std_dir);
	if (dir) {
		free(dir);
	}
	return 0;
}

int vmkdir(const char *path) {
	return mkdir(path);
}

int vrmdir(const char *path) {
	return rmdir(path);
}

int vrmfile(const char *path) {
	return remove(path);
}

int vrename(const char *oldPath, const char *newPath) {
	return rename(oldPath, newPath);
}

int vfeof(int fh) {
	return feof(FSH[fh].fh);
}
#endif /* if (OLD_VFS_SUPPORT) */

//********************************************************
//					DEBUG FUNCTIONS
//********************************************************
#if (DEBUG_FUNC)

int remove_test(const char *path) {
	FILE *fh;
	
	fh = fopen(path, "w");
	if (fh) {
		fclose(fh);
	}
	else {
		return -1;
	}
	
	if (remove(path) != 0) {
		return -1;
	}
	
	fh = fopen(path, "r");
	
	if (fh) {
		fclose(fh);
		return -1;
	}
	
	return 0;
	
}

int rename_test(const char *oldname, const char *newname) {
	FILE *fh;
	
	fh = fopen(oldname, "w");
	if (fh) {
		fclose(fh);
	}
	else {
		return -1;
	}
	
	if (rename(oldname, newname) != 0) {
		return -1;
	}
	
	fh = fopen(newname, "r");
	if (fh) {
		fclose(fh);
		return 0;
	}
	else {
		return -1;
	}	
}

void mkdirs(const char *root) {
	FILE *fh;
	char name_buf[128];
	char buffer[] = "123456";
	
// 	sprintf(name_buf, "%s/test", root);
// 	mkdir(name_buf);
	
	sprintf(name_buf, "%s/1", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/2", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/3", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/4", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/5", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/6.txt", root);
	mkdir(name_buf);
	
	sprintf(name_buf, "%s/1.txt", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}
	
	sprintf(name_buf, "%s/2.txt", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}
	
	sprintf(name_buf, "%s/3.txt", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}
	
	sprintf(name_buf, "%s/4.txt", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}
	
	sprintf(name_buf, "%s/5.txt", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}
	
	sprintf(name_buf, "%s/6", root);
	fh = fopen(name_buf, "w");
	if (fh) {
		fwrite(buffer, 1, sizeof(buffer), fh);
		fclose(fh);
	}	
}

int fdir_test(const char *pdir, int mkdir) {
	struct dirent *dirinfo;
	DIR *dir;
	
	if (mkdir) {
		mkdirs(pdir);
	}
	
	dir = opendir(pdir);
	TRACE_DEBUG("%s", pdir);
	while (1) {
		dirinfo = readdir(dir);
		if (dirinfo == NULL) {
			break;
		}
		else {
			TRACE_DEBUG("%s", dirinfo->name);
			if (dirinfo->ftype == FS_TYPE_DIR) {
				TRACE_DEBUG_WP("  [DIR]");
			}
			else {
				TRACE_DEBUG_WP("  [FIL]");
			}
			TRACE_DEBUG_WP("   [%d]", dirinfo->fsize);
		}
	}
	closedir(dir);
	return 0;
}

	
int fseek_test(const char *src_fname, const char *dst_fname) {
	FILE *fsrc;
	FILE *fdst;
	char buffer[16];
	const char srcbuf[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	unsigned int pos = 0;
	
	fsrc = fopen(src_fname, "w");
	fwrite(srcbuf, 1, sizeof(srcbuf), fsrc);
	fclose(fsrc);
	
	fsrc = fopen(src_fname, "r");
	fdst = fopen(dst_fname, "w+");
	
	if ((fsrc == NULL) || (fdst == NULL)) {
		TRACE_ERROR("open file error");
		fclose(fsrc);
		fclose(fdst);
		return -1;
	}
	
	/* seek 4bytes from beginning */
	fseek(fsrc, 4, SEEK_SET);
	pos = ftell(fsrc);
	fread(buffer, 1, 4, fsrc);
	fwrite(buffer, 1, 4, fdst);
	
	/* seek 4bytes from current */
	fseek(fsrc, 4, SEEK_CUR);
	pos = ftell(fsrc);
	fread(buffer, 1, 4, fsrc);
	fwrite(buffer, 1, 4, fdst);
	
	/* seek 4bytes from tail */
	fseek(fsrc, -4, SEEK_END);
	pos = ftell(fsrc);
	fread(buffer, 1, 4, fsrc);
	fwrite(buffer, 1, 4, fdst);
	
	/* read back all dst data */
	fseek(fdst, 0, SEEK_SET);
	fread(buffer, 1, sizeof(buffer), fdst);
	
	fclose(fsrc);
	fclose(fdst);
	return 0;	
}
#endif /* if (DEBUG_FUNC) */

//extern int MTD_read_block(int block_num,  char *buffer, int blocks);
//extern int MTD_write_block(int block_num, char *buffer, int blocks);
//extern int MTD_device_init(void);
//extern int MTD_device_deinit(void);

/*
** @ Lowlevel Filesystem init
**/
int fs_lowlevel_init() {
	mtd_dev_t mmc;
	mtd_dev_t nand;
	
	memset(&mmc, 0, sizeof(mmc));
	memset(&nand, 0, sizeof(mtd_dev_t));
	
#if (MTD_STDIN || MTD_STDOUT || FS_TYPE_STDERR)
	STD_device_init(921600);
#endif 


#if (MTD_FATFS)
	/* MMC */
//	mmc.name = "mmc";
//	mmc.fstype = FS_TYPE_FAT;
//	mmc.MTD_read_block	=  	MTD_read_block;
//	mmc.MTD_write_block =  	MTD_write_block;
//	mmc.MTD_device_init = 	MTD_device_init;
//	mmc.MTD_device_deinit = MTD_device_deinit;
//	mtd_register_dev(&mmc);
#endif
	
#if (MTD_YAFFS2)
	/* NAND READONLY */
	nand.fstype = FS_TYPE_YAFFS;
	nand.name = "nand1";
	nand.flash_dev.param.total_bytes_per_chunk = 2048;
	nand.flash_dev.param.chunks_per_block = 64;
	nand.flash_dev.param.n_reserved_blocks = 5;
	nand.flash_dev.param.inband_tags = 0;
	nand.flash_dev.param.start_block = 88;
	nand.flash_dev.param.end_block = 887;
	nand.flash_dev.param.is_yaffs2 = 1;
	nand.flash_dev.param.use_nand_ecc = 0;
	nand.flash_dev.param.wide_tnodes_disabled = 0;
	nand.flash_dev.param.refresh_period = 1000;
	nand.flash_dev.param.n_caches = 10; // Use caches
	nand.flash_dev.driver_context = NAND_open();  // Used to identify the device in fstat.
	nand.flash_dev.param.write_chunk_fn = NULL; 	// NANDWriteChunk;
	nand.flash_dev.param.read_chunk_fn = NULL;	// NANDReadChunk;
	nand.flash_dev.param.write_chunk_tags_fn = 	NANDWriteChunkTags;
	nand.flash_dev.param.read_chunk_tags_fn = 	NANDReadChunkTags;
	nand.flash_dev.param.erase_fn = 			NANDErase;
	nand.flash_dev.param.initialise_flash_fn = 	NANDInitFlash;
	nand.flash_dev.param.deinitialise_flash_fn =NANDDeinitFlash;
	nand.flash_dev.param.bad_block_fn = 		NANDMarkBlockBad;
	nand.flash_dev.param.query_block_fn = 		NANDQueryBlock;
	nand.flash_dev.param.enable_xattr = 1;
	mtd_register_dev(&nand);
	
	/* NAND READWRITE */
// 	nand.name = "nand2";
// 	nand.flash_dev.param.start_block = 889;
// 	nand.flash_dev.param.end_block = 1023;
// 	mtd_register_dev(&nand);
#endif
	return 0;
}
