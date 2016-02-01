#ifndef _HEX_LOG_H_
#define _HEX_LOG_H_


#include "types.h"


extern int LOG_open(const char *FileName, unsigned int MaxLogFileSize);
extern int LOG_d(const char *format, ...);
extern int LOG_e(const char *format, ...);
extern int LOG_w(const char *format, ...);
extern int LOG_i(const char *format, ...);
extern int LOG_f(const char *format, ...);
extern int LOG_n(const char *format, ...);
extern int LOG_close(void);


#endif /* ifndef _HEX_LOG_H_ */
