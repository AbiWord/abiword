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

// XXX reenable on Windows after fixing it.
#ifndef _WIN32

#include "tf_test.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include <cstdlib>

#include <glib.h>
#include <gio/gio.h>

#ifdef HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
# include <valgrind/valgrind.h>
# define CC_EXTENSION __extension__
#else
# define VALGRIND_COUNT_ERRORS 0
# define VALGRIND_DO_LEAK_CHECK
# define VALGRIND_COUNT_LEAKS(a,b,c,d) (a=b=c=d=0)
# define CC_EXTENSION
#endif

#define MAX_TEST_TIME 40     // max seconds for a single test to run
#define MAX_TOTAL_TIME 120*60 // max seconds for the entire suite to run

static int memerrs()
{
    return (int)(CC_EXTENSION VALGRIND_COUNT_ERRORS);
}

static int memleaks()
{
    int leaked = 0, dubious = 0, reachable = 0, suppressed = 0;
    VALGRIND_DO_LEAK_CHECK;
    VALGRIND_COUNT_LEAKS(leaked, dubious, reachable, suppressed);
    printf("memleaks: sure:%d dubious:%d reachable:%d suppress:%d\n",
       leaked, dubious, reachable, suppressed);

    // dubious+reachable are normally non-zero because of globals...
    // return leaked+dubious+reachable;
    return leaked;
}


TF_Test *TF_Test::first, *TF_Test::last;
int TF_Test::fails, TF_Test::runs;
time_t TF_Test::start_time;


void TF_Test::alarm_handler(int)
{
    printf("\n! TF_Test  Current test took longer than %d seconds!  FAILED\n",
       MAX_TEST_TIME);
    abort();
}

void TF_Test::pulse()
{
    alarm(MAX_TEST_TIME);
}

TF_Test::TF_Test(const char *_suite, const char *_descr,
                 const char *_idstr, MainFunc *_main)
    : m_suite(_suite)
    , descr(_descr)
    , main(_main)
    , next(NULL)
{
    const char *cptr = strrchr(_idstr, '/');
    if (cptr)
        idstr = cptr+1;
    else
        idstr = _idstr;
    if (first)
        last->next = this;
    else
        first = this;
    last = this;
}


static bool prefix_match(const char *s, const char * const *prefixes)
{
    for (const char * const *prefix = prefixes; prefix && *prefix; prefix++)
    {
        if (!strncasecmp(s, *prefix, strlen(*prefix)))
            return true;
    }
    return false;
}


int TF_Test::run(const char * const *prefixes, const char *suite)
{
    int old_valgrind_errs = 0, new_valgrind_errs;
    int old_valgrind_leaks = 0, new_valgrind_leaks;

    signal(SIGALRM, alarm_handler);
    // signal(SIGALRM, SIG_IGN);
    alarm(MAX_TEST_TIME);
    start_time = time(NULL);

    fails = runs = 0;
    for (TF_Test *cur = first; cur; cur = cur->next)
    {
        if ((!prefixes
            || prefix_match(cur->idstr, prefixes)
            || prefix_match(cur->descr, prefixes))
            && (!suite || strcmp(cur->m_suite, suite) == 0))
        {
            printf("%s: Testing \"%s\" in %s:\n", cur->m_suite, cur->descr,
                   cur->idstr);
            cur->main();

            new_valgrind_errs = memerrs();
            TFPASS(new_valgrind_errs == old_valgrind_errs);
            old_valgrind_errs = new_valgrind_errs;

            new_valgrind_leaks = memleaks();
            TFPASS(new_valgrind_leaks == old_valgrind_leaks);
            old_valgrind_leaks = new_valgrind_leaks;

            printf("\n");
        }
    }

    if (prefixes && *prefixes)
        printf("TF_Test: WARNING: only ran tests starting with "
               "specifed prefix(es).\n");
    else if (suite)
        printf("TF_Test: WARNING: only ran suite %s\n", suite);
    else
        printf("TF_Test: ran all tests.\n");
    printf("TF_Test: %d test%s, %d failure%s.\n",
           runs, runs==1 ? "" : "s",
           fails, fails==1 ? "": "s");

    return fails != 0;
}

int TF_Test::run_all(const char * const *prefixes)
{
    return run(prefixes, NULL);
}

int TF_Test::run_suite(const char *suite)
{
    return run(NULL, suite);
}

void TF_Test::start(const char *file, int line, const char *condstr)
{
    // strip path from filename
    const char *file2 = strrchr(file, '/');
    if (!file2)
        file2 = file;
    else
        file2++;

    char *condstr2 = g_strdup(condstr), *cptr;
    for (cptr = condstr2; *cptr; cptr++)
    {
        if (!isprint((unsigned char)*cptr))
            *cptr = '!';
    }

    printf("! %s:%-5d %-40s ", file2, line, condstr2);
    fflush(stdout);

    g_free(condstr2);
}


void TF_Test::check(bool cond)
{
    alarm(MAX_TEST_TIME); // restart per-test timeout
    if (!start_time)
        start_time = time(NULL);

    if (time(NULL) - start_time > MAX_TOTAL_TIME)
    {
        printf("\n! TF_Test   Total run time exceeded %d seconds!  FAILED\n",
               MAX_TOTAL_TIME);
        abort();
    }

    runs++;

    if (cond)
        printf("ok\n");
    else
    {
        printf("FAILED\n");
        fails++;
    }
    fflush(stdout);
}


bool TF_Test::start_check_eq(const char *file, int line,
                const char *a, const char *b)
{
    if (!a)
        a = "";
    if (!b)
        b = "";

    size_t len = strlen(a) + strlen(b) + 8 + 1;
    char *str = new char[len];
    snprintf(str, len, "[%s] == [%s]", a, b);

    start(file, line, str);
    delete[] str;

    bool cond = !strcmp(a, b);
    check(cond);
    return cond;
}


bool TF_Test::start_check_eq(const char *file, int line, int a, int b)
{
    size_t len = 128 + 128 + 8 + 1;
    char *str = new char[len];
    snprintf(str, len, "%d == %d", a, b);

    start(file, line, str);
    delete[] str;

    bool cond = (a == b);
    check(cond);
    return cond;
}

/** get the test src dir. ABI_TEST_SRC_DIR or top_srcdir */
const char* TF_Test::get_test_src_dir()
{
    const char* dir = getenv("ABI_TEST_SRC_DIR");
    if (!dir) {
        dir = getenv("top_srcdir");
    }
    if (!dir) {
        printf("Failed to determine the src dir.\n"
               "Please set ABI_TEST_SRC_DIR or top_srcdir env.\n"
               "Using '.' for now.\n");
        return ".";
    }
    return dir;
}

/** ensure the test data file exists
 * @param file the base filaname
 * @param [out] path the full path of the data file
 * @return true if the file exists.
 */
bool TF_Test::ensure_test_data(const char* file, std::string & path)
{
    path = "file://";
    path += TF_Test::get_test_src_dir();
    path += file;

    bool exists = false;
    GFile *f = g_file_new_for_uri(path.c_str());
    exists = g_file_query_exists(f, NULL);
    g_object_unref(f);

    return exists;
}

#endif /* _WIN32 */
