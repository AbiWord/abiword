#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;
extern FILE *outputfile;
extern int breakcount;
extern int verpadding;

int flushbreaks(int newpap)
    {
    int silent=0,i;
    error(erroroutput,"flushing breaks %d\n",breakcount);

    if (breakcount == 1)
        oprintf(silent,"<br>\n");
    else if (breakcount > 1)
        {
        if (verpadding == 1)
            {
            for (i=0;i<breakcount;i++)
                oprintf(silent,"<br>\n");
            }
        else
            {
            oprintf(silent,"<p>\n");
            if (verpadding != 2)
                if (breakcount > 2)
                    for (i=2;i<breakcount;i++)
                        oprintf(silent,"<br>\n");
            }
        }
    else
        return 0;
    breakcount=0;
    return 1;
    }


void do_indent(pap *apap)
    {
    S16 paraindent;
    static int oldparaindent;
    int i;
    error(erroroutput,"the left margin is %d twirps\n",apap->leftmargin);

    if (apap == NULL)
        return;

    paraindent = apap->leftmargin;
    paraindent /= TWIRPS_PER_BQ;
    error(erroroutput,"no of dirs is %d\n",paraindent);
    if (paraindent < 0)
        paraindent = 0;

    if (paraindent < oldparaindent)
        {
        for(i=paraindent;i<oldparaindent;i++)
            fprintf(outputfile,"\n</dir>");
        }
    else if (paraindent > oldparaindent)
        {
        for (i=oldparaindent;i<paraindent;i++)
            fprintf(outputfile,"\n<dir>");
        }
    oldparaindent = paraindent;
    if (apap->firstline >0)
        {
        fprintf(outputfile,"<img height=1 width=%d src=\"%s/clear.gif\">",apap->firstline/TWIRPS_PER_H_PIXEL,patterndir());
        }
    }


