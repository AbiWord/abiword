

#include "tf_test.h"
#include "ut_vector.h"



TFTEST_MAIN("UT_GenericVector basics")
{
	UT_GenericVector<char *> v;

	TFPASS(v.getItemCount() == 0);
	v.addItem("foo");
	TFPASS(v.getItemCount() == 1);
	v.addItem("bar");
	TFPASS(v.getItemCount() == 2);
	v.addItem("baz");
	TFPASS(v.getItemCount() == 3);

	TFPASS(strcmp(v[1], "bar") == 0);
	TFPASS(strcmp(v.getNthItem(2), "baz") == 0);

	TFPASS(strcmp(v.getFirstItem(), "foo") == 0);
	TFPASS(strcmp(v.getLastItem(), "baz") == 0);
	TFPASS(strcmp(v.back(), "baz") == 0);

	v.push_back("metropolis");
	TFPASS(v.getItemCount() == 4);
	TFPASS(strcmp(v.back(), "metropolis") == 0);
	
	v.insertItemAt("matrix", 3);
	TFPASS(v.getItemCount() == 5);
	TFPASS(strcmp(v[3], "matrix") == 0);
	TFPASS(strcmp(v[4], "metropolis") == 0);

	v.deleteNthItem(3);
	TFPASS(v.getItemCount() == 4);
	TFPASS(strcmp(v[3], "metropolis") == 0);	

	TFPASS(v.size() == 4);

	TFPASS(v.findItem("metropolis") == 3);
	TFPASS(v.findItem("bar") == 1);

	TFPASS(v.pop_back());
	TFPASS(v.getItemCount() == 3);
	TFPASS(strcmp(v.getLastItem(), "baz") == 0);

	v.clear();
	TFPASS(v.getItemCount() == 0);
}

TFTEST_MAIN("vector sorting")
{
}

