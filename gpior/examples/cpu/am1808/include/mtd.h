#ifndef _MTD_H_
#define _MTD_H_

#include "yaffs_list.h"
#include "yaffs_guts.h"


#define MAX_MTDDEV_NUM	5


enum fs_t { FS_TYPE_FAT, FS_TYPE_YAFFS, FS_TYPE_STDIN, FS_TYPE_STDOUT, FS_TYPE_STDERR };


typedef struct {
	struct list_head list;
	void *device;
	char *name;
	char mount_point[128];
	enum fs_t fstype;
	int  is_mounted;
	int  readonly;
	
	/* SD/MMC/USB members */
	int fatdev_index;
	int (*MTD_read_block)(int block_num,  char *buffer, int blocks);
	int (*MTD_write_block)(int block_num, char *buffer, int blocks);
	int (*MTD_device_init)(void);
	int (*MTD_device_deinit)(void);
	
	/* NAND members */
	struct yaffs_dev flash_dev;
} mtd_dev_t;


extern int mtd_register_dev(mtd_dev_t *dev);
extern mtd_dev_t *mtd_search_by_devname(const char *devname);
extern mtd_dev_t *mtd_search_by_mntpt(const char *mntpt);
extern int mtd_unregister_dev(const char *name);


#endif /* ifndef _MTD_H_ */
