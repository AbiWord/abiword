#ifndef UT_PAIR_H
#define UT_PAIR_H

#include <stdlib.h>

typedef void* pair_type;

class UT_Pair
{
public:
    UT_Pair(const pair_type car, const pair_type cdr);
    ~UT_Pair();

	inline const pair_type& car() const { return car_; }
	inline const pair_type& cdr() const { return cdr_; }
	inline pair_type& car() { return car_; }
	inline pair_type& cdr() { return cdr_; }

private:
    pair_type car_, cdr_;
};

#endif
