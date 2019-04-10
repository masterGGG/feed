/**
 * =====================================================================================
 *       @file  stat_analysis.h
 *      @brief  header file for statistic analysis of storage server
 *
 *     Created  07/05/2011 01:51:21 PM 
 *    Revision  1.0.0.0
 *    Compiler  gcc/g++
 *     Company  TaoMee.Inc, ShangHai.
 *   Copyright  Copyright (c) 2011, TaoMee.Inc, ShangHai.
 *
 *     @author  lc (l.c.), lc@taomee.com
 * =====================================================================================
 */
#ifndef  __STAT_ANALYSIS_CPP__
#define  __STAT_ANALYSIS_CPP__
#include  <sys/time.h>
#include  "util.h"
//storage feeds hit ratio statistic           0x0D000411 (db_num, mem_num)
//storage passive feeds hit ratio statistic   0x0D000412 (db_num, mem_num)

typedef struct storage_stat_hits {
    uint32_t mem_num;
    uint32_t db_num;
} storage_hits;
const int storage_hits_size = sizeof(storage_hits);

//time class in microseconds units
class c_time
{
    public:
        explicit c_time();
        explicit c_time(long int sec, long int usec);
        void set();
        long int operator-(const c_time &rhs);

    private:
        struct timeval tv;
};

bool stat_init(const char * logdir, const char * logpre);

#define     stat_log(fmt, args...)      write_common_log(fmt"\n", ##args)
void write_common_log(const char* fmt, ...);

void stat_uninit();

#ifdef TRACE_DEBUG 
inline char *get_fidstatid(feedid_t *fid) {
    static char statidbuf[256] = {0};
    snprintf(statidbuf, sizeof(statidbuf), "%u_%u_%u_%u", fid->mimi, fid->cmd_id, fid->app_id, fid->timestamp);
    return statidbuf;
}
    #define     STAT_START()        stat_timestart.set()
    #define     STAT_ENDTIME(fid, type)      do { stat_timeend.set();\
                        stat_log("{\"type\":1,\"timestamp\":%d, \"feedid\":\"%s\",\"%s\":%ld}", \
                        time(0), get_fidstatid(fid), type, stat_timeend - stat_timestart);\
                }while(0);

    #define     STAT_END2TIME(type)      do { stat_timeend.set();\
                        stat_log("{\"type\":1,\"timestamp\":%d,\"%s\":%ld}", time(0), type, stat_timeend - stat_timestart);\
                }while(0);

    #define     STAT_PKGLEN(fid, len)      do {\
                       stat_log("{\"type\":1,\"timestamp\":%d,\"feedid\":\"%s\",\"pkglen\":%d}", time(0), get_fidstatid(fid), len);\
                }while(0);
#else 
    #define     STAT_START()        
    #define     STAT_ENDTIME(type)  
    #define     STAT_END2TIME(fid, type) 
    #define     STAT_PKGLEN(len)      
#endif

inline void storage_hits_log(uint32_t msgid, time_t *base_time, storage_hits *hit)
{
    time_t now_timer = time(0);
    if ((*base_time + 60) <= now_timer) {
        if (!(hit->mem_num == 0 && hit->db_num == 0)) {
            msglog(gconfig::get_msg_file_path(), msgid, now_timer, &hit, storage_hits_size);
        }
        *base_time = now_timer;
        hit->mem_num = 0;
        hit->db_num = 0;
    }
}
#endif  /*__STAT_ANALYSIS_CPP__*/
