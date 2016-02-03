#ifndef _E2PROM_H_
#define _E2PROM_H_

extern int E2PROM_open(unsigned char sub_addr_lenb, unsigned char dev_addr);
extern int E2PROM_read(unsigned intsub_addr, unsigned char * buf, unsigned intlen);
extern int E2PROM_write(unsigned intsub_addr, unsigned char * buf,	unsigned intlen);
extern int E2PROM_close(void);
extern void E2PROM_wp_enable(void);
extern void E2PROM_wp_disable(void);

#endif // ifndef _E2PROM_H_
