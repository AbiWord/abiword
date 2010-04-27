// A bijection is a one-to-one mapping.
// This particular bijection maps strings to strings.
// It also copies anything which is put into it.

#include "ut_bijection.h"
#include <stdlib.h>
#include "ut_string.h"
#include <string.h>

UT_Bijection::UT_Bijection()
{
    m_n = 0;
    m_first = m_second = NULL;
}

void UT_Bijection::clear()
{
    if (!m_n)
		return;

    for(int i=0;i<m_n;++i) 
	{
		g_free(m_first[i]);
		g_free(m_second[i]);	
    };

    g_free(m_first);
    g_free(m_second);
    m_first = m_second = NULL;
    m_n = 0;
}

UT_Bijection::~UT_Bijection()
{
    clear();
}

const char *UT_Bijection::lookupByTarget(const char *s)	 const
{
	if (s == NULL) {
		return NULL;
	}
    for(int i=0; i<m_n; ++i) {
	if (!strcmp(s,m_second[i]))
		return m_first[i];
    };
    return NULL;
}

const char *UT_Bijection::lookupBySource(const char *s) const
{
	if (s == NULL) {
		return NULL;
	}
    for(int i=0; i<m_n; ++i) {
	if (!strcmp(s,m_first[i]))
		return m_second[i];
    };
    return NULL;
}

void  UT_Bijection::add(const char *f, const char *s)
{
    m_first = static_cast<char**>(g_try_realloc(static_cast<void*>(m_first),(m_n+1)*sizeof(char*)));
    m_first[m_n] = g_strdup(f);

    m_second = static_cast<char**>(g_try_realloc(static_cast<void*>(m_second),(m_n+1)*sizeof(char*)));
    m_second[m_n] = g_strdup(s);
    ++m_n;
}

void UT_Bijection::add(const pair_data* items)
{
    for(;items->s1 && items->s2;++items)
		add(items->s1, items->s2);
}

size_t UT_Bijection::size() const
{
    return m_n;
}

const char* UT_Bijection::nth1(size_t idx) const
{
    return ( idx>(size()-1) ) ? NULL : m_first[idx];
}

const char* UT_Bijection::nth2(size_t idx) const
{
    return ( idx>(size()-1) ) ? NULL : m_second[idx];
}
