#include"ut_pair.h"
#include <stdlib.h>
#include "ut_string.h"


UT_Pair::UT_Pair()
{
    m_n = 0;
    m_first = m_second = NULL;
};

void UT_Pair::clear()
{
    if (!m_n)
	return;
    for(int i=0;i<m_n;++i) {
	free(m_first[i]);
	free(m_second[i]);	
    };
    free(m_first);
    free(m_second);
    m_first = m_second = NULL;
    m_n = 0;
};

UT_Pair::~UT_Pair()
{
    clear();
};

const char *UT_Pair::getFirst(const char *s)	 const
{
    for(int i=0; i<m_n; ++i) {
	if (!strcmp(s,m_second[i]))
		return m_first[i];
    };
    return NULL;
};

const char *UT_Pair::getSecond(const char *s) const
{
    for(int i=0; i<m_n; ++i) {
	if (!strcmp(s,m_first[i]))
		return m_second[i];
    };
    return NULL;
};

void  UT_Pair::add(const char *f, const char *s)
{
    m_first = (char**)realloc((void*)m_first,(m_n+1)*sizeof(char*));
    m_first[m_n] = UT_strdup(f);

    m_second = (char**)realloc((void*)m_second,(m_n+1)*sizeof(char*));
    m_second[m_n] = UT_strdup(s);
    ++m_n;
};

void UT_Pair::add(const pair_data* items)
{
    for(;items->s1 && items->s2;++items)
	add(items->s1, items->s2);
};

size_t UT_Pair::size() const
{
    return m_n;
};

const char* UT_Pair::nth1(size_t idx) const
{
    return ( idx>(size()-1) ) ? NULL : m_first[idx];
};

const char* UT_Pair::nth2(size_t idx) const
{
    return ( idx>(size()-1) ) ? NULL : m_second[idx];
};
