#include "mtd.h"

#include <string.h>
#include <stdlib.h>

static mtd_dev_t mtd_devs[MAX_MTDDEV_NUM] = {0};

/*
** @ register a device
**/
int mtd_register_dev(mtd_dev_t *dev) {
// 	static int entry_init = 0;
// 	
// 	if (entry_init == 0) {
// 		memset(&mtd_dev_entry, 0, sizeof(mtd_dev_t));
// 		INIT_LIST_HEAD(&mtd_dev_entry.list);
// 		entry_init = 1;
// 	}
// 	INIT_LIST_HEAD(&dev->list);
// 	if (mtd_dev_entry == NULL) {
// 		mtd_dev_entry = dev;
// 	}
// 	else {
// 		list_add(&dev->list, &mtd_dev_entry->list);
// 	}
	int i;
	
	for (i = 0; i < MAX_MTDDEV_NUM; i ++) {
		if (mtd_devs[i].name == NULL) {
			memcpy(&mtd_devs[i], dev, sizeof(mtd_dev_t));
			return 0;
		}			
	}
	return -1;
}

/*
** @ unregister a device
**/
int mtd_unregister_dev(const char *name) {
	int i;

	for (i = 0; i < MAX_MTDDEV_NUM; i ++) {
		if (strcmp(mtd_devs[i].name, name) == 0) {
			memset(&mtd_devs[i], 0, sizeof(mtd_dev_t));
			return 0;
		}
	}
	return -1;
}

/*
** @ search device by device name
**/
mtd_dev_t *mtd_search_by_devname(const char *devname) {
// 	struct list_head *list;
// 	mtd_dev_t *dev;
// 	
// 	list_for_each(list, &mtd_dev_entry->list) {
// 		dev = list_entry(list, mtd_dev_t, list);
// 		if (strcmp(dev->name, devname) == 0) {
// 			return dev;
// 		}
// 	}
	int i;

	for (i = 0; i < MAX_MTDDEV_NUM; i ++) {
		if (strcmp(mtd_devs[i].name, devname) == 0) {
			return &mtd_devs[i];
		}
	}
	return NULL;
}

/*
** @ search device by mount point
**/
mtd_dev_t *mtd_search_by_mntpt(const char *mntpt) {
// 	struct list_head *list;
// 	mtd_dev_t *dev;
// 	
// 	list_for_each(list, &mtd_dev_entry->list) {
// 		dev = list_entry(list, mtd_dev_t, list);
// 		if (strcmp(dev->mount_point, mntpt) == 0) {
// 			return dev;
// 		}
// 	}
	int i;

	for (i = 0; i < MAX_MTDDEV_NUM; i ++) {
		if (strcmp(mtd_devs[i].mount_point, mntpt) == 0) {
			return &mtd_devs[i];
		}
	}
	return NULL;
}


// int MTD_test() {
// 	mtd_dev_t dev[5];
// 	mtd_dev_t *pdev;
// 	int i;
// 	
// 	strcpy(dev[0].name, "sm");
// 	strcpy(dev[1].name, "ol");
// 	strcpy(dev[2].name, "gj");
// 	strcpy(dev[3].name, "kj");
// 	strcpy(dev[4].name, "xj");
// 	for (i = 0; i < 5; i ++) {
// 		mtd_register_dev(&dev[i]);
// 	}
// 	
// 	pdev = mtd_search_by_devname("sm");
// 	pdev = mtd_search_by_devname("ol");
// 	pdev = mtd_search_by_devname("gj");
// 	pdev = mtd_search_by_devname("kj");
// 	pdev = mtd_search_by_devname("xj");
// 	
// 	for (i = 0; i < 5; i ++) {
// 		mtd_unregister_dev(&dev[i]);
// 	}
// 	return 0;
// }
