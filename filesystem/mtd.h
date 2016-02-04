#ifndef _MTD_H_
#define _MTD_H_



/* Enable the NAND Flash YAFFS2 */
#define MTD_YAFFS2	0

/* Enable the SD-CARD FATFS */
#define MTD_FATFS	1

/* Enable the Stander input */
#define MTD_STDIN	0

/* Enable the Stander output */
#define MTD_STDOUT	0

/* Enable the Stadard error output */
#define MTD_STDERR	0

/* Max device number */
#define MAX_MTDDEV_NUM	1


#if (MTD_YAFFS2)
#include "yaffs_guts.h"
#endif


enum fs_t { FS_TYPE_FAT, FS_TYPE_YAFFS, FS_TYPE_STDIN, FS_TYPE_STDOUT, FS_TYPE_STDERR };


#if (MTD_STDIN || MTD_STDOUT || FS_TYPE_STDERR)
	/* Standard input and output */
extern int STD_device_init(unsigned int baudrate);
extern int STD_device_deinit(void);
#	if (MTD_STDIN)
extern int STD_getc(void);
#	endif
#	if (MTD_STDOUT || FS_TYPE_STDERR)
extern int STD_putc(char c);
#	endif
#endif


typedef struct {
// 	struct list_head list;
	void *device;
	char *name;
	char mount_point[8];
	enum fs_t fstype;
	int  is_mounted;
	int  readonly;
	
	/* SD/MMC/USB members */
#if (MTD_FATFS)
	int fatdev_index;
	int (*MTD_read_block)(int block_num,  char *buffer, int blocks);
	int (*MTD_write_block)(int block_num, char *buffer, int blocks);
	int (*MTD_device_init)(void);
	int (*MTD_device_deinit)(void);
#endif
	
	/* NAND members */
#if (MTD_YAFFS2)
	struct yaffs_dev flash_dev;
#endif
} mtd_dev_t;


extern int mtd_register_dev(mtd_dev_t *dev);
extern mtd_dev_t *mtd_search_by_devname(const char *devname);
extern mtd_dev_t *mtd_search_by_mntpt(const char *mntpt);
extern int mtd_unregister_dev(const char *name);


#endif /* ifndef _MTD_H_ */
