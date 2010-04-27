
#ifndef __TF_TEST_H__
#define __TF_TEST_H__

#include <time.h>

class TF_Test
{
    typedef void MainFunc();
    const char *descr, *idstr;
    MainFunc *main;
    TF_Test *next;
    static TF_Test *first, *last;
    static int fails, runs;
    static time_t start_time;
    
    static void alarm_handler(int sig);
   
public:
    TF_Test(const char *_descr, const char *_idstr, MainFunc *_main);
    static int run_all(const char * const *prefixes = NULL);
    static void start(const char *file, int line, const char *condstr);
    static void check(bool cond);
    static inline bool start_check(const char *file, int line,
				   const char *condstr, bool cond)
        { start(file, line, condstr); check(cond); return cond; }
    static bool start_check_eq(const char *file, int line,
			       const char *a, const char *b);
    static bool start_check_eq(const char *file, int line, int a, int b);
};



#define TFPASS(cond) \
    TF_Test::start_check(__FILE__, __LINE__, #cond, (cond))
#define TFPASSEQ(a, b) \
    TF_Test::start_check_eq(__FILE__, __LINE__, (a), (b))

#define TFFAIL(cond) \
    TF_Test::start_check(__FILE__, __LINE__, "NOT(" #cond ")", !(cond))

#define TFTEST_MAIN3(descr, ff, ll) \
    static void _tftest_main_##ll(); \
    static TF_Test _tftest_##ll(descr, ff, _tftest_main_##ll); \
    static void _tftest_main_##ll()
#define TFTEST_MAIN2(descr, ff, ll) TFTEST_MAIN3(descr, ff, ll)
#define TFTEST_MAIN(descr) TFTEST_MAIN2(descr, __FILE__, __LINE__)

#endif
