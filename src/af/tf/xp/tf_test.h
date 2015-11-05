/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2004,2005 Hubert Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
/* Original Copyright notice, as part of WvStreams, LGPL licensed */
/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Part of an automated testing framework.  See wvtest.h.
 */


#ifndef __TF_TEST_H__
#define __TF_TEST_H__

#include <time.h>

class TF_Test
{
    typedef void MainFunc();
    const char *m_suite;
    const char *descr, *idstr;
    MainFunc *main;
    TF_Test *next;
    static TF_Test *first, *last;
    static int fails, runs;
    static time_t start_time;

    static void alarm_handler(int sig);

public:
    TF_Test(const char *_suite, const char *_descr,
            const char *_idstr, MainFunc *_main);
    static int run(const char * const *prefixes, const char * suite);
    static int run_all(const char * const *prefixes = NULL);
    static int run_suite(const char * suite);
    static void start(const char *file, int line, const char *condstr);
    static void check(bool cond);
    static inline bool start_check(const char *file, int line,
				   const char *condstr, bool cond)
        { start(file, line, condstr); check(cond); return cond; }
    static bool start_check_eq(const char *file, int line,
			       const char *a, const char *b);
    static bool start_check_eq(const char *file, int line, int a, int b);
    /** send a pulse to reset the alarm */
    static void pulse();
};



#define TFPASS(cond) \
    TF_Test::start_check(__FILE__, __LINE__, #cond, (cond))
#define TFPASSEQ(a, b) \
    TF_Test::start_check_eq(__FILE__, __LINE__, (a), (b))

#define TFFAIL(cond) \
    TF_Test::start_check(__FILE__, __LINE__, "NOT(" #cond ")", !(cond))

#define TFTEST_MAIN3(suite, descr, ff, ll)       \
    static void _tftest_main_##ll(); \
    static TF_Test _tftest_##ll(suite, descr, ff, _tftest_main_##ll);    \
    static void _tftest_main_##ll()
#define TFTEST_MAIN2(suite, descr, ff, ll) \
    TFTEST_MAIN3(suite, descr, ff, ll)
#define TFTEST_MAIN(descr) \
    TFTEST_MAIN2(TFSUITE, descr, __FILE__, __COUNTER__)

#endif
