#include "pfaedit.h"
#include "splinefont.h"
#include "psfont.h"
#include "gdraw.h"
#include <stdarg.h>
#include <ctype.h>

int local_charset = 3;

void GDrawIError(const char *fmt,...) {
    char buf[1025];
    va_list ap;

    strcpy(buf,"Internal Error:\n");
    va_start(ap, fmt);
    vsprintf(buf+strlen(buf), fmt, ap);
    va_end(ap);
    fprintf( stderr, "%s\n", buf );
}

static void ProcessFile(char *psfile, char * afmfile) {
    FontDict *fd = ReadPSFont(psfile);
    SplineFont *sf;
    FILE *afm;
    char buffer[1025];

    if ( fd==NULL )
return;

    sf = SplineFontFromPSFont(fd);
    sf->filename = strdup(psfile);
    PSFontFree(fd);

    afm = fopen(afmfile,"w");
    if ( afm==NULL ) {
	fprintf( stderr, "Error: Can't open afm file: %s\n", buffer );
	exit(1);
    }
    if ( !AfmSplineFont(afm,sf)) {
	fprintf( stderr, "Error: Failed to write afm file: %s\n", buffer );
	exit(1);
    }
    fclose(afm);
}

static void usage(char * name)
{
	fprintf( stderr, "Usage: %s -p psfont -a afmfile\n", name);
	exit(1);
}

int main(int argc, char *argv[]) {
    int i;
    char * psfile = 0;
    char * afmfile = 0;

    for ( i=1; i < argc; ++i )
    {
	if(!strcmp(argv[i],"-p"))
        {
		psfile = argv[++i];
		continue;
	}
	if(!strcmp(argv[i],"-a"))
	{
		afmfile = argv[++i];
		continue;
	}

	fprintf(stderr, "Error: invalid arguments\n");
	usage(argv[0]);
    }

	if(!psfile || !afmfile)
	{
		fprintf(stderr, "Error: invalid arguments\n");
		usage(argv[0]);
	}
    ProcessFile(psfile, afmfile);

return(0);
}
