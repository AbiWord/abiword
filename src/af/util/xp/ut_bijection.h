#ifndef UT_BIJECTION_H
#define UT_BIJECTION_H

#include <stdlib.h>
class UT_Bijection
{
public:

    UT_Bijection();
    ~UT_Bijection();
    
    /* NULL is returned if item not found. */
    const char *lookupByTarget(const char *s) const;
    const char *lookupBySource(const char *s) const;
    void  add(const char *f, const char *s);    
    
    struct pair_data
    {
		const char *s1,*s2;
    };
    
    /*the array is terminated by record with s1 or s2 == NULL*/    
    void add(const pair_data* items);
    size_t size() const;
    const char* nth1(size_t idx) const;
    const char* nth2(size_t idx) const;    
    void clear();

private:
    int m_n;
    char **m_first;
    char **m_second;
};

#endif
