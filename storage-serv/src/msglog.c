/**
 *====================================================================
 * 	   @file	msglog.c
 *    @brief	
 * 
 *  @internal
 *   @created	2009/8/14 15:27:47
 *   @version   1.0.0.0
 *   compiler   gcc
 *   platform 	linux/debian
 *
 *    @author	taomee 
 *   modifyer   aceway
 *	  company   TaoMee,Inc. ShangHai CN (www.taomee.com)
 *
 *	copyright:	2009 TaoMee, Inc. ShangHai CN. All right reserved.
 *====================================================================
 */
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <strings.h>
#include <malloc.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "msglog.h"

/** 
 * @brief  standard data format
 *     arg1(4byte) arg2(4byte) arg3(4byte) ...
 *     log message format    
 *     level(1byte) reserved(1byte) textlen(2byte) text...
 * @param   
 * @return  0 sucess
            -1 
 */
int msglog(const char *logfile, uint32_t type, uint32_t timestamp, const void *data, int len) 
{
    message_header_t *h;
    int fd, s;

    s = sizeof(message_header_t)+len;
    h = (message_header_t *)malloc(s);
    bzero(h, s);
    h->len = s;
    h->hlen = sizeof(message_header_t);
    h->type = type;
    h->timestamp = timestamp;

    if(len>0) memcpy((char *)(h+1), data, len);

    signal(SIGXFSZ, SIG_IGN);
    fd = open(logfile, O_WRONLY|O_APPEND, 0666);
    if(fd<0) 
    {
        fd = open(logfile, O_WRONLY|O_APPEND|O_CREAT, 0666);
        int ret=fchmod(fd,0777);
        if ((ret!=0)||(fd<0))
        {
            return -1;
        }
    }

    write(fd, (char *)h, s);
    close(fd);

    signal(SIGXFSZ, SIG_DFL);
    free(h);
    return 0;
}
