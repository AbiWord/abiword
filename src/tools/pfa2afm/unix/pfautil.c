#include <stdlib.h>
#include <string.h>
/*#include "ustring.h"*/

char *copy(const char *str) {
    char *ret;

    if ( str==NULL )
return( NULL );
    ret = malloc(strlen(str)+1);
    strcpy(ret,str);
return( ret );
}
