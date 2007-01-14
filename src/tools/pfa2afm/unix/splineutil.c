#include "pfaedit.h"
#include <stdio.h>
#include <math.h>
#include "splinefont.h"
#include "psfont.h"
#include "ustring.h"
#include "string.h"
#include "ctype.h"

void LineListFree(LineList *ll) {
    LineList *next;

    while ( ll!=NULL ) {
	next = ll->next;
	free(ll);
	ll = next;
    }
}

void LinearApproxFree(LinearApprox *la) {
    LinearApprox *next;

    while ( la!=NULL ) {
	next = la->next;
	LineListFree(la->lines);
	free(la);
	la = next;
    }
}

void SplineFree(Spline *spline) {
    LinearApproxFree(spline->approx);
    free(spline);
}

void SplinePointFree(SplinePoint *sp) {
    free(sp);
}

void SplineRefigure(Spline *spline) {
    SplinePoint *from = spline->from, *to = spline->to;

    spline->dx = from->me.x; spline->dy = from->me.y;
    if ( from->nonextcp ) from->nextcp = from->me;
    if ( to->noprevcp ) to->prevcp = to->me;
    if ( from->nonextcp && to->noprevcp ) {
	spline->islinear = true;
	spline->cx = to->me.x-from->me.x;
	spline->cy = to->me.y-from->me.y;
    } else {
	/* from p. 393 (Operator Details, curveto) Postscript Lang. Ref. Man. (Red book) */
	spline->cx = 3*(from->nextcp.x-from->me.x);
	spline->cy = 3*(from->nextcp.y-from->me.y);
	spline->bx = 3*(to->prevcp.x-from->nextcp.x)-spline->cx;
	spline->by = 3*(to->prevcp.y-from->nextcp.y)-spline->cy;
	spline->ax = to->me.x-from->me.x-spline->cx-spline->bx;
	spline->ay = to->me.y-from->me.y-spline->cy-spline->by;
	if ( spline->ay==0 && spline->ax==0 && spline->by==0 && spline->bx==0 )
	    spline->islinear = true;	/* I'm not sure if this can happen */
    }
    LinearApproxFree(spline->approx);
    spline->approx = NULL;
}

Spline *SplineMake(SplinePoint *from, SplinePoint *to) {
    Spline *spline = calloc(1,sizeof(Spline));

    spline->from = from; spline->to = to;
    from->next = to->prev = spline;
    SplineRefigure(spline);
return( spline );
}

static Spline *RegenSpline(Spline *spline) {
    SplinePoint *from = spline->from, *to = spline->to;

    spline->dx = from->me.x; spline->dy = from->me.y;
    if ( from->nonextcp ) from->nextcp = from->me;
    if ( to->noprevcp ) to->prevcp = to->me;
    if ( from->nonextcp && to->noprevcp ) {
	spline->islinear = true;
	spline->cx = to->me.x-from->me.x;
	spline->cy = to->me.y-from->me.y;
    } else {
	/* from p. 393 (Operator Details, curveto) Postscript Lang. Ref. Man. (Red book) */
	spline->cx = 3*(from->nextcp.x-from->me.x);
	spline->cy = 3*(from->nextcp.y-from->me.y);
	spline->bx = 3*(to->prevcp.x-from->nextcp.x)-spline->cx;
	spline->by = 3*(to->prevcp.y-from->nextcp.y)-spline->cy;
	spline->ax = to->me.x-from->me.x-spline->cx-spline->bx;
	spline->ay = to->me.y-from->me.y-spline->cy-spline->by;
    }
return( spline );
}

static double SolveCubic(double a, double b, double c, double d, double err, double t0) {
    /* find t between t0 and .5 where at^3+bt^2+ct+d == +/- err */
    double t, val, offset;
    int first;

    offset=a;
    if ( a<0 ) offset = -a;
    if ( b<0 ) offset -= b; else offset += b;
    if ( c<0 ) offset -= c; else offset += c;
    offset += 1;		/* Make sure it isn't 0 */
    offset = err/(10.*offset);
    if ( offset<.00001 ) offset = .00001; else if ( offset>.01 ) offset = .01;

    first = 1;
    for ( t=t0+offset; t<.5; t += offset ) {
	val = ((a*t+b)*t+c)*t+d;
	if ( val>=err || val<=-err )
    break;
	first = 0;
    }
    if ( !first )
	t -= offset;
    if ( t>.5 ) t=.5;

return( t );
}

static double SolveCubicBack(double a, double b, double c, double d, double err, double t0) {
    /* find t between .5 and t0 where at^3+bt^2+ct+d == +/- err */
    double t, val, offset;
    int first;

    offset=a;
    if ( a<0 ) offset = -a;
    if ( b<0 ) offset -= b; else offset += b;
    if ( c<0 ) offset -= c; else offset += c;
    offset += 1;		/* Make sure it isn't 0 */
    offset = err/(10.*offset);
    if ( offset<.00001 ) offset = .00001; else if ( offset>.01 ) offset = .01;

    first = 1;
    for ( t=t0-offset; t>.5; t -= offset ) {
	val = ((a*t+b)*t+c)*t+d;
	if ( val>=err || val<=-err )
    break;
	first = 0;
    }
    if ( !first )
	t -= offset;
    if ( t<.5 ) t=.5;

return( t );
}
    
/* Remove line segments which are just one point long */
/* Merge colinear line segments */
/* Merge two segments each of which involves a single pixel change in one dimension (cut corners) */ 
static void SimplifyLineList(LineList *prev) {
    LineList *next, *lines;

    if ( prev->next==NULL )
return;
    lines = prev->next;
    while ( (next = lines->next)!=NULL ) {
	if ( (prev->here.x==lines->here.x && prev->here.y == lines->here.y ) ||
		( prev->here.x==lines->here.x && lines->here.x==next->here.x ) ||
		( prev->here.y==lines->here.y && lines->here.y==next->here.y ) ||
		(( prev->here.x==next->here.x+1 || prev->here.x==next->here.x-1 ) &&
		 ( prev->here.y==next->here.y+1 || prev->here.y==next->here.y-1 )) ) {
	    lines->here = next->here;
	    lines->next = next->next;
	    free(next);
	} else {
	    prev = lines;
	    lines = next;
	}
    }
    if ( prev!=NULL &&
	    prev->here.x==lines->here.x && prev->here.y == lines->here.y ) {
	prev->next = NULL;
	free(lines);
    }

    while ( (next = lines->next)!=NULL ) {
	if ( prev->here.x!=next->here.x ) {
	    double slope = (next->here.y-prev->here.y) / (double) (next->here.x-prev->here.x);
	    double inter = prev->here.y - slope*prev->here.x;
	    int y = rint(lines->here.x*slope + inter);
	    if ( y == lines->here.y ) {
		lines->here = next->here;
		lines->next = next->next;
		free(next);
	    } else
		lines = next;
	} else
	    lines = next;
    }
}

LinearApprox *SplineApproximate(Spline *spline, double scale) {
    LinearApprox *test;
    LineList *cur, *last=NULL, *prev;
    double tx,ty,t;
    double slpx, slpy;
    double intx, inty;

    for ( test = spline->approx; test!=NULL && test->scale!=scale; test = test->next );
    if ( test!=NULL )
return( test );

    test = calloc( 1,sizeof(LinearApprox));
    test->scale = scale;
    test->next = spline->approx;
    spline->approx = test;

    cur = calloc( 1,sizeof(LineList) );
    cur->here.x = rint(spline->from->me.x*scale);
    cur->here.y = rint(spline->from->me.y*scale);
    test->lines = last = cur;
    if ( spline->islinear ) {
	cur = calloc( 1,sizeof(LineList) );
	cur->here.x = rint(spline->to->me.x*scale);
	cur->here.y = rint(spline->to->me.y*scale);
	last->next = cur;
    } else {
	/* find t so that (xt,yt) is a half pixel off from (cx*t+dx,cy*t+dy) */
	/* min t of scale*(ax*t^3+bx*t^2)==.5, scale*(ay*t^3+by*t^2)==.5 */
	/* I do this from both ends in. this is because I miss essential */
	/*  symmetry if I go from one end to the other. */
	/* first start at 0 and go to .5, the first linear approximation is easy */
	/*  it's just the function itself ignoring higher orders, so the error*/
	/*  is just the higher orders */
	tx = SolveCubic(spline->ax,spline->bx,0,0,.5/scale,0);
	ty = SolveCubic(spline->ay,spline->by,0,0,.5/scale,0);
	t = (tx<ty)?tx:ty;
	cur = calloc( 1,sizeof(LineList) );
	cur->here.x = rint( (((spline->ax*t+spline->bx)*t+spline->cx)*t + spline->dx)*scale );
	cur->here.y = rint( (((spline->ay*t+spline->by)*t+spline->cy)*t + spline->dy)*scale );
	last->next = cur;
	last = cur;
	while ( t<.5 ) {
	    slpx = (3*spline->ax*t+2*spline->bx)*t+spline->cx;
	    slpy = (3*spline->ay*t+2*spline->by)*t+spline->cy;
	    intx = ((spline->ax*t+spline->bx)*t+spline->cx-slpx)*t + spline->dx;
	    inty = ((spline->ay*t+spline->by)*t+spline->cy-slpy)*t + spline->dy;
	    tx = SolveCubic(spline->ax,spline->bx,spline->cx-slpx,spline->dx-intx,.5/scale,t);
	    ty = SolveCubic(spline->ay,spline->by,spline->cy-slpy,spline->dy-inty,.5/scale,t);
	    t = (tx<ty)?tx:ty;
	    cur = calloc(1,sizeof(LineList));
	    cur->here.x = rint( (((spline->ax*t+spline->bx)*t+spline->cx)*t + spline->dx)*scale );
	    cur->here.y = rint( (((spline->ay*t+spline->by)*t+spline->cy)*t + spline->dy)*scale );
	    last->next = cur;
	    last = cur;
	}

	/* Now start at t=1 and work back to t=.5 */
	prev = NULL;
	cur = calloc( 1,sizeof(LineList) );
	cur->here.x = rint(spline->to->me.x*scale);
	cur->here.y = rint(spline->to->me.y*scale);
	prev = cur;
	t=1.0;
	while ( 1 ) {
	    slpx = (3*spline->ax*t+2*spline->bx)*t+spline->cx;
	    slpy = (3*spline->ay*t+2*spline->by)*t+spline->cy;
	    intx = ((spline->ax*t+spline->bx)*t+spline->cx-slpx)*t + spline->dx;
	    inty = ((spline->ay*t+spline->by)*t+spline->cy-slpy)*t + spline->dy;
	    tx = SolveCubicBack(spline->ax,spline->bx,spline->cx-slpx,spline->dx-intx,.5/scale,t);
	    ty = SolveCubicBack(spline->ay,spline->by,spline->cy-slpy,spline->dy-inty,.5/scale,t);
	    t = (tx>ty)?tx:ty;
	    cur = calloc( 1,sizeof(LineList) );
	    cur->here.x = rint( (((spline->ax*t+spline->bx)*t+spline->cx)*t + spline->dx)*scale );
	    cur->here.y = rint( (((spline->ay*t+spline->by)*t+spline->cy)*t + spline->dy)*scale );
	    cur->next = prev;
	    prev = cur;
	    if ( t<=.5 )
	break;
	}
	last->next = cur;
	SimplifyLineList(test->lines);
    }
    if ( test->lines->next==NULL ) {
	test->oneline = 1;
	test->onepoint = 1;
    } else if ( test->lines->next->next == NULL ) {
	test->oneline = 1;
    }
return( test );
}

static void SplineFindBounds(Spline *sp, DBounds *bounds) {
    double t, b2_fourac, x,y;

    /* first try the end points */
    if ( sp->to->me.x<bounds->minx ) bounds->minx = sp->to->me.x;
    if ( sp->to->me.x>bounds->maxx ) bounds->maxx = sp->to->me.x;

    /* then try the extrema of the spline (assuming they are between t=(0,1) */
    if ( sp->ax!=0 ) {
	b2_fourac = 4*sp->bx*sp->bx - 12*sp->ax*sp->cx;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t = (-2*sp->bx + b2_fourac) / (6*sp->ax);
	    if ( t>0 && t<1 ) {
		x = ((sp->ax*t+sp->bx)*t+sp->cx)*t + sp->dx;
		if ( x<bounds->minx ) bounds->minx = x;
		if ( x>bounds->maxx ) bounds->maxx = x;
	    }
	    t = (-2*sp->bx - b2_fourac) / (6*sp->ax);
	    if ( t>0 && t<1 ) {
		x = ((sp->ax*t+sp->bx)*t+sp->cx)*t + sp->dx;
		if ( x<bounds->minx ) bounds->minx = x;
		if ( x>bounds->maxx ) bounds->maxx = x;
	    }
	}
    } else if ( sp->bx!=0 ) {
	t = -sp->cx/(2.0*sp->bx);
	if ( t>0 && t<1 ) {
	    x = (sp->bx*t+sp->cx)*t + sp->dx;
	    if ( x<bounds->minx ) bounds->minx = x;
	    if ( x>bounds->maxx ) bounds->maxx = x;
	}
    }

    /* same for y */
    if ( sp->to->me.y<bounds->miny ) bounds->miny = sp->to->me.y;
    if ( sp->to->me.y>bounds->maxy ) bounds->maxy = sp->to->me.y;

    if ( sp->ay!=0 ) {
	b2_fourac = 4*sp->by*sp->by - 12*sp->ay*sp->cy;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t = (-2*sp->by + b2_fourac) / (6*sp->ay);
	    if ( t>0 && t<1 ) {
		y = ((sp->ay*t+sp->by)*t+sp->cy)*t + sp->dy;
		if ( y<bounds->miny ) bounds->miny = y;
		if ( y>bounds->maxy ) bounds->maxy = y;
	    }
	    t = (-2*sp->by - b2_fourac) / (6*sp->ay);
	    if ( t>0 && t<1 ) {
		y = ((sp->ay*t+sp->by)*t+sp->cy)*t + sp->dy;
		if ( y<bounds->miny ) bounds->miny = y;
		if ( y>bounds->maxy ) bounds->maxy = y;
	    }
	}
    } else if ( sp->by!=0 ) {
	t = -sp->cy/(2.0*sp->by);
	if ( t>0 && t<1 ) {
	    y = (sp->by*t+sp->cy)*t + sp->dy;
	    if ( y<bounds->miny ) bounds->miny = y;
	    if ( y>bounds->maxy ) bounds->maxy = y;
	}
    }
}

static void SplineSetFindBounds(SplinePointList *spl, DBounds *bounds) {
    Spline *spline, *first;

    for ( ; spl!=NULL; spl = spl->next ) {
	first = NULL;
	if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 ) {
	    bounds->minx = bounds->maxx = spl->first->me.x;
	    bounds->miny = bounds->maxy = spl->first->me.y;
	} else {
	    if ( spl->first->me.x<bounds->minx ) bounds->minx = spl->first->me.x;
	    if ( spl->first->me.x>bounds->maxx ) bounds->maxx = spl->first->me.x;
	    if ( spl->first->me.y<bounds->miny ) bounds->miny = spl->first->me.y;
	    if ( spl->first->me.y>bounds->maxy ) bounds->maxy = spl->first->me.y;
	}
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    SplineFindBounds(spline,bounds);
	    if ( first==NULL ) first = spline;
	}
    }
}

void SplineCharFindBounds(SplineChar *sc,DBounds *bounds) {
    RefChar *rf;

    /* a char with no splines (ie. a space) must have an lbearing of 0 */
    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    for ( rf=sc->refs; rf!=NULL; rf = rf->next )
	SplineSetFindBounds(rf->splines,bounds);
    
    SplineSetFindBounds(sc->splines,bounds);
}

void SplineFontFindBounds(SplineFont *sf,DBounds *bounds) {
    RefChar *rf;
    int i;

    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    for ( i = 0; i<sf->charcnt; ++i ) {
	SplineChar *sc = sf->chars[i];
	for ( rf=sc->refs; rf!=NULL; rf = rf->next )
	    SplineSetFindBounds(rf->splines,bounds);

	SplineSetFindBounds(sc->splines,bounds);
    }
}

static void CatagorizePoint(SplinePoint *sp) {

    sp->pointtype = pt_corner;
    if ( sp->next==NULL && sp->prev==NULL )
	;
    else if ( sp->next==NULL ) {
	sp->pointtype = sp->noprevcp ? pt_corner : pt_curve;
    } else if ( sp->prev==NULL ) {
	sp->pointtype = sp->nonextcp ? pt_corner : pt_curve;
    } else if ( sp->nonextcp && sp->noprevcp ) {
	;
    } else if ( !sp->nonextcp && !sp->noprevcp ) {
	if ( sp->nextcp.y==sp->prevcp.y && sp->nextcp.y==sp->me.y )
	    sp->pointtype = pt_curve;
	else if ( sp->nextcp.x!=sp->prevcp.x ) {
	    double slope = (sp->nextcp.y-sp->prevcp.y)/(sp->nextcp.x-sp->prevcp.x);
	    double y = slope*(sp->me.x-sp->prevcp.x) + sp->prevcp.y - sp->me.y;
	    if ( y<1 && y>-1 )
		sp->pointtype = pt_curve;
	} else if ( sp->me.x == sp->nextcp.x )
	    sp->pointtype = pt_curve;
    } else if ( sp->nonextcp ) {
	if ( sp->next->to->me.x!=sp->prevcp.x ) {
	    double slope = (sp->next->to->me.y-sp->prevcp.y)/(sp->next->to->me.x-sp->prevcp.x);
	    double y = slope*(sp->me.x-sp->prevcp.x) + sp->prevcp.y - sp->me.y;
	    if ( y<1 && y>-1 )
		sp->pointtype = pt_tangent;
	} else if ( sp->me.x == sp->prevcp.x )
	    sp->pointtype = pt_tangent;
    } else {
	if ( sp->nextcp.x!=sp->prev->from->me.x ) {
	    double slope = (sp->nextcp.y-sp->prev->from->me.y)/(sp->nextcp.x-sp->prev->from->me.x);
	    double y = slope*(sp->me.x-sp->prev->from->me.x) + sp->prev->from->me.y - sp->me.y;
	    if ( y<1 && y>-1 )
		sp->pointtype = pt_tangent;
	} else if ( sp->me.x == sp->nextcp.x )
	    sp->pointtype = pt_tangent;
    }
}

static void CatagorizePoints(SplineChar *sc) {
    Spline *spline, *first, *last=NULL;
    SplinePointList *spl;

    for ( spl = sc->splines; spl!=NULL; spl = spl->next ) {
	first = NULL;
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    CatagorizePoint(spline->from);
	    last = spline;
	    if ( first==NULL ) first = spline;
	}
	if ( spline==NULL && last!=NULL )
	    CatagorizePoint(last->to);
    }
}

static SplineChar *Type1BytesToSplines(unsigned char *type1, int len,
	struct chars *subrs, char *name) {
    double stack[30]; int sp=0, v;
    SplineChar *ret = calloc(1,sizeof(SplineChar));
    SplinePointList *cur=NULL, *oldcur;
    RefChar *r1, *r2, *rlast=NULL;
    BasePoint current, oldcurrent;
    double dx, dy, dx2, dy2, dx3, dy3;
    SplinePoint *pt;
    /* subroutines may be nested to a depth of 10 */
    struct substate { unsigned char *type1; int len; } pcstack[11];
    int pcsp=0;
    Hints *hint, *hp;
    double pops[30];
    int popsp=0;

    ret->name = copy(name);
    current.x = current.y = 0;
    while ( len>0 ) {
	--len;
	if ( (v = *type1++)>=32 ) {
	    if ( v<=246) {
		stack[sp++] = v - 139;
	    } else if ( v<=250 ) {
		stack[sp++] = (v-247)*256 + *type1++ + 108;
		--len;
	    } else if ( v<=254 ) {
		stack[sp++] = -(v-251)*256 - *type1++ - 108;
		--len;
	    } else {
		stack[sp++] = (*type1<<24) | (type1[1]<<16) | (type1[2]<<8) | type1[3];
		type1 += 4;
		len -= 4;
	    }
	} else if ( v==12 ) {
	    v = *type1++;
	    --len;
	    switch ( v ) {
	      case 0: /* dotsection */
		sp = 0;
	      break;
	      case 1: /* vstem3 */	/* specifies three v hints zones at once */
		if ( sp<6 ) fprintf(stderr, "Stack underflow on vstem3 in %s\n", name );
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[0] + ret->lsidebearing;
		hint->width = stack[1];
		if ( ret->vstem==NULL )
		    ret->vstem = hp = hint;
		else {
		    for ( hp=ret->vstem; hp->next!=NULL; hp = hp->next );
		    hp->next = hint;
		    hp = hint;
		}
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[2] + ret->lsidebearing;
		hint->width = stack[3];
		hp->next = hint; hp = hint;
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[4] + ret->lsidebearing;
		hint->width = stack[5];
		hp->next = hint; 
		sp = 0;
	      break;
	      case 2: /* hstem3 */	/* specifies three h hints zones at once */
		if ( sp<6 ) fprintf(stderr, "Stack underflow on hstem3 in %s\n", name );
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[0];
		hint->width = stack[1];
		if ( ret->hstem==NULL )
		    ret->hstem = hp = hint;
		else {
		    for ( hp=ret->hstem; hp->next!=NULL; hp = hp->next );
		    hp->next = hint;
		    hp = hint;
		}
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[2];
		hint->width = stack[3];
		hp->next = hint; hp = hint;
		hint = calloc(1,sizeof(Hints));
		hint->base = stack[4];
		hint->width = stack[5];
		hp->next = hint; 
		sp = 0;
	      break;
	      case 6: /* seac */	/* build accented characters */
		if ( sp<5 ) fprintf(stderr, "Stack underflow on seac in %s\n", name );
		/* stack[0] must be the lsidebearing of the accent. I'm not sure why */
		r1 = calloc(1,sizeof(RefChar));
		r2 = calloc(1,sizeof(RefChar));
		r2->transform[0] = 1; r2->transform[3]=1;
		r2->transform[4] = stack[1] - (stack[0]-ret->lsidebearing);
		r2->transform[5] = stack[2];
		/* the translation of the accent here is said to be relative */
		/*  to the origins of the base character. I think they place */
		/*  the origin at the left bearing. And they don't mean the  */
		/*  base char at all, they mean the current char's lbearing  */
		/*  (which is normally the same as the base char's, except   */
		/*  when I has a big accent (like diaerisis) */
		r1->transform[0] = 1; r1->transform[3]=1;
		r1->adobe_enc = stack[3];
		r2->adobe_enc = stack[4];
		r1->local_enc = r2->local_enc = -1;
		r1->next = r2;
		if ( rlast!=NULL ) rlast->next = r1;
		else ret->refs = r1;
		rlast = r2;
		sp = 0;
	      break;
	      case 7: /* sbw */		/* generalized width/sidebearing command */
		if ( sp<4 ) fprintf(stderr, "Stack underflow on sbw in %s\n", name );
		ret->lsidebearing = stack[0];
		/* stack[1] is lsidebearing y (only for vertical writing styles, CJK) */
		ret->width = stack[2];
		/* stack[3] is height (for vertical writing styles, CJK) */
		sp = 0;
	      break;
	      case 12: /* div */
		if ( sp<2 ) fprintf(stderr, "Stack underflow on div in %s\n", name );
		stack[sp-2] /= stack[sp-1];
		--sp;
	      break;
	      case 16: /* callothersubr */
		/* stack[sp-1] is the number of the thing to call in the othersubr array */
		/* stack[sp-2] is the number of args to grab off our stack and put on the */
		/*  real postscript stack */
		if ( sp<2 || sp < 2+stack[sp-2] ) {
		    fprintf(stderr, "Stack underflow on callothersubr in %s\n", name );
		    sp = 0;
		} else {
		    int tot = stack[sp-2], k;
		    popsp = 0;
		    for ( k=sp-3; k>=sp-2-tot; --k )
			pops[popsp++] = stack[k];
		    /* othersubrs 0-3 must be interpretted. 0-2 are Flex, 3 is Hint Replacement */
		    if ( stack[sp-1]==3 )
			/* we aren't capabable of hint replacement so we punt */
			/*  we punt by putting 3 on the stack (T1 spec page 70) */
			pops[popsp-1] = 3;
		    else if ( stack[sp-1]==1 ) {
			/* We punt for flex too. This is a bit harder */
			/* Essentially what we want to do is draw a line from */
			/*  where we are at the beginning to where we are at */
			/*  the end. So we save the beginning here (this starts*/
			/*  the flex sequence), we ignore all calls to othersub*/
			/*  2, and when we get to othersub 0 we put everything*/
			/*  back to where it should be and free up whatever */
			/*  extranious junk we created along the way and draw */
			/*  our line. */
			/* Let's punt a little less, and actually figure out */
			/*  the appropriate rrcurveto commands and put in a */
			/*  dished serif */
			oldcurrent = current;
			oldcur = cur;
		    } else if ( stack[sp-1]==2 )
			/* No op */;
		    else if ( stack[sp-1]==0 ) {
			SplinePointList *spl = oldcur->next;
			if ( spl!=NULL && spl->next!=NULL &&
				spl->next->next!=NULL &&
				spl->next->next->next!=NULL &&
				spl->next->next->next->next!=NULL &&
				spl->next->next->next->next->next!=NULL &&
				spl->next->next->next->next->next->next!=NULL ) {
			    BasePoint old_nextcp, mid_prevcp, mid, mid_nextcp,
				    end_prevcp, end;
			    old_nextcp	= spl->next->first->me;
			    mid_prevcp	= spl->next->next->first->me;
			    mid		= spl->next->next->next->first->me;
			    mid_nextcp	= spl->next->next->next->next->first->me;
			    end_prevcp	= spl->next->next->next->next->next->first->me;
			    end		= spl->next->next->next->next->next->next->first->me;
			    cur = oldcur;
			    if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
				cur->last->nextcp = old_nextcp;
				cur->last->nonextcp = false;
				pt = calloc(1,sizeof(SplinePoint));
				pt->prevcp = mid_prevcp;
				pt->me = mid;
				pt->nextcp = mid_nextcp;
			        pt->flex = pops[2];
				SplineMake(cur->last,pt);
				cur->last = pt;
				pt = calloc(1,sizeof(SplinePoint));
				pt->prevcp = end_prevcp;
				pt->me = end;
				pt->nonextcp = true;
				SplineMake(cur->last,pt);
				cur->last = pt;
			    } else
				fprintf(stderr, "No previous point on path in curveto from flex 0 in %s\n", name );
			} else {
			    /* Um, something's wrong. Let's just draw a line */
			    /* do the simple method, which consists of creating */
			    /*  the appropriate line */
			    pt = calloc(1,sizeof(SplinePoint));
			    pt->me.x = pops[1]; pt->me.y = pops[0];
			    pt->noprevcp = true; pt->nonextcp = true;
			    SplinePointListFree(oldcur->next); oldcur->next = NULL;
			    cur = oldcur;
			    if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
				SplineMake(cur->last,pt);
				cur->last = pt;
			    } else
				fprintf(stderr, "No previous point on path in lineto from flex 0 in %s\n", name );
			}
			--popsp;
		    }
		    sp = k+1;
		}
	      break;
	      case 17: /* pop */
		/* pops something from the postscript stack and pushes it on ours */
		/* used to get a return value from an othersubr call */
		/* Bleah. Adobe wants the pops to return the arguments if we */
		/*  don't understand the call. What use is the subroutine then?*/
		if ( popsp<=0 )
		    fprintf(stderr, "Pop stack underflow on pop in %s\n", name );
		else
		    stack[sp++] = pops[--popsp];
	      break;
	      case 33: /* setcurrentpoint */
		if ( sp<2 ) fprintf(stderr, "Stack underflow on setcurrentpoint in %s\n", name );
		else {
		    current.x = stack[0];
		    current.y = stack[1];
		}
		sp = 0;
	      break;
	    }
	} else switch ( v ) {
	  case 1: /* hstem */
	    if ( sp<2 ) fprintf(stderr, "Stack underflow on hstem in %s\n", name );
	    /* stack[0] is absolute y for start of horizontal hint */
	    /*	(actually relative to the y specified as lsidebearing y in sbw*/
	    /* stack[1] is relative y for height of hint zone */
	    hint = calloc(1,sizeof(Hints));
	    hint->base = stack[0];
	    hint->width = stack[1];
	    if ( ret->hstem==NULL )
		ret->hstem = hint;
	    else {
		for ( hp=ret->hstem; hp->next!=NULL; hp = hp->next );
		hp->next = hint;
	    }
	    sp = 0;
	  break;
	  case 3: /* vstem */
	    if ( sp<2 ) fprintf(stderr, "Stack underflow on vstem in %s\n", name );
	    /* stack[0] is absolute x for start of horizontal hint */
	    /*	(actually relative to the x specified as lsidebearing in h/sbw*/
	    /* stack[1] is relative x for height of hint zone */
	    hint = calloc(1,sizeof(Hints));
	    hint->base = stack[0] + ret->lsidebearing;
	    hint->width = stack[1];
	    if ( ret->vstem==NULL )
		ret->vstem = hint;
	    else {
		for ( hp=ret->vstem; hp->next!=NULL; hp = hp->next );
		hp->next = hint;
	    }
	    sp = 0;
	  break;
	  case 14: /* endchar */
	  break;
	  case 13: /* hsbw (set left sidebearing and width) */
	    if ( sp<2 ) fprintf(stderr, "Stack underflow on hsbw in %s\n", name );
	    ret->lsidebearing = stack[0];
	    current.x = stack[0];		/* sets the current point too */
	    ret->width = stack[1];
	    sp = 0;
	  break;
	  case 9: /* closepath */
	    sp = 0;
	    if ( cur!=NULL && cur->first!=NULL && cur->first!=cur->last ) {
		if ( cur->first->me.x==cur->last->me.x && cur->first->me.y==cur->last->me.y ) {
		    SplinePoint *oldlast = cur->last;
		    cur->first->prevcp = oldlast->prevcp;
		    cur->first->noprevcp = false;
		    oldlast->prev->from->next = NULL;
		    cur->last = oldlast->prev->from;
		    free(oldlast->prev);
		    free(oldlast);
		}
		SplineMake(cur->last,cur->first);
		cur->last = cur->first;
	    }
	  break;
	  case 5: /* rlineto */
	  case 6: /* hlineto */
	  case 7: /* vlineto */
	  case 21: /* rmoveto */
	  case 22: /* hmoveto */
	  case 4: /* vmoveto */
	    dx = dy = 0;
	    if ( v==5 || v==21 ) {
		if ( sp<2 ) fprintf(stderr, "Stack underflow on rlineto/rmoveto in %s\n", name );
		dx = stack[0];
		dy = stack[1];
	    } else if ( v==6 || v==22 ) {
		if ( sp<1 ) fprintf(stderr, "Stack underflow on hlineto/hmoveto in %s\n", name );
		dx = stack[0];
	    } else /*if ( v==7 || v==4 )*/ {
		if ( sp<1 ) fprintf(stderr, "Stack underflow on vlineto/vmoveto in %s\n", name );
		dy = stack[0];
	    }
	    current.x += dx; current.y += dy;
	    pt = calloc(1,sizeof(SplinePoint));
	    pt->me = current;
	    pt->noprevcp = true; pt->nonextcp = true;
	    if ( v==4 || v==21 || v==22 ) {
		SplinePointList *spl = calloc(1,sizeof(SplinePointList));
		spl->first = spl->last = pt;
		if ( cur!=NULL )
		    cur->next = spl;
		else
		    ret->splines = spl;
		cur = spl;
	    } else {
		if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
		    SplineMake(cur->last,pt);
		    cur->last = pt;
		} else
		    fprintf(stderr, "No previous point on path in lineto in %s\n", name );
	    }
	    sp = 0;
	  break;
	  case 8: /* rrcurveto */
	  case 31: /* hvcurveto */
	  case 30: /* vhcurveto */
	    dx = dy = dx2 = dy2 = dx3 = dy3 = 0;
	    if ( v==8 ) {
		if ( sp<6 )
		    fprintf(stderr, "Stack underflow on rrcurveto in %s\n", name );
		dx = stack[0];
		dy = stack[1];
		dx2 = stack[2];
		dy2 = stack[3];
		dx3 = stack[4];
		dy3 = stack[5];
	    } else if ( v==31 ) {
		if ( sp<4 ) fprintf(stderr, "Stack underflow on hvcurveto in %s\n", name );
		dx = stack[0];
		dx2 = stack[1];
		dy2 = stack[2];
		dy3 = stack[3];
	    } else /*if ( v==30 )*/ {
		if ( sp<4 ) fprintf(stderr, "Stack underflow on vhcurveto in %s\n", name );
		dy = stack[0];
		dx2 = stack[1];
		dy2 = stack[2];
		dx3 = stack[3];
	    }
	    if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
		current.x += dx; current.y += dy;
		cur->last->nextcp = current;
		cur->last->nonextcp = false;
		current.x += dx2; current.y += dy2;
		pt = calloc(1,sizeof(SplinePoint));
		pt->prevcp = current;
		current.x += dx3; current.y += dy3;
		pt->me = current;
		pt->nonextcp = true;
		SplineMake(cur->last,pt);
		cur->last = pt;
	    } else
		fprintf(stderr, "No previous point on path in curveto in %s\n", name );
	    sp = 0;
	  break;
	  case 10: /* callsubr */
	    /* stack[sp-1] contains the number of the subroutine to call */
	    if ( sp<1 ) fprintf(stderr, "Stack underflow on callsubr in %s\n", name );
	    else if ( pcsp>10 ) fprintf( stderr, "Too many subroutine calls in %s\n", name );
	    if ( subrs==NULL || stack[sp-1]>=subrs->cnt ||
		    subrs->values[(int) stack[sp-1]]==NULL )
		fprintf(stderr,"Subroutine number out of bounds in %s\n", name );
	    else {
		pcstack[pcsp].type1 = type1;
		pcstack[pcsp].len = len;
		++pcsp;
		type1 = subrs->values[(int) stack[sp-1]];
		len = subrs->lens[(int) stack[sp-1]];
	    }
	    if ( --sp<0 ) sp = 0;
	  break;
	  case 11: /* return */
	    /* return from a subr outine */
	    if ( pcsp<1 ) fprintf(stderr, "return when not in subroutine in %s\n", name );
	    else {
		--pcsp;
		type1 = pcstack[pcsp].type1;
		len = pcstack[pcsp].len;
	    }
	  break;
	}
    }
    if ( pcsp!=0 ) fprintf(stderr, "end of subroutine reached with no return in %s\n", name );
    CatagorizePoints(ret);
return( ret );
}

static int CharsNotInEncoding(FontDict *fd) {
    int i, cnt, j;
    for ( i=cnt=0; i<fd->chars->cnt; ++i ) {
	if ( fd->chars->keys[i]!=NULL ) {
	    for ( j=0; j<256; ++j )
		if ( fd->encoding[j]!=NULL &&
			strcmp(fd->encoding[j],fd->chars->keys[i])==0 )
	    break;
	    if ( j==256 )
		++cnt;
	}
    }
return( cnt );
}

static int LookupCharString(char *encname,struct chars *chars) {
    int k;

    for ( k=0; k<chars->cnt; ++k ) {
	if ( chars->keys[k]!=NULL )
	    if ( strcmp(encname,chars->keys[k])==0 )
return( k );
    }
return( -1 );
}

static int UnicodeNameLookup(char *name) {
    extern int psunicodenames_cnt;
    extern char *psunicodenames[];
    int i;

    if ( name==NULL )
return( -1 );

    if ( *name=='u' && name[1]=='n' && name[2]=='i' &&
	    (isdigit(name[3]) || (name[3]>='A' && name[3]<='F')) &&
	    (isdigit(name[4]) || (name[4]>='A' && name[4]<='F')) &&
	    (isdigit(name[5]) || (name[5]>='A' && name[5]<='F')) &&
	    (isdigit(name[6]) || (name[6]>='A' && name[6]<='F')) &&
	    name[7]=='\0' ) {
return( strtol(name+3,NULL,16));
    }
    for ( i=0; i<psunicodenames_cnt; ++i ) if ( psunicodenames[i]!=NULL )
	if ( strcmp(name,psunicodenames[i])==0 )
return( i );
    if ( strcmp(name,"nbspace")==0 )
return( 0xa0 );

return( -1 );
}

SplinePointList *SplinePointListCopy(SplinePointList *base, int allpoints ) {
    SplinePointList *head=NULL, *last, *cur;
    SplinePoint *pt, *cpt, *first;
    Spline *spline;

    for ( ; base!=NULL; base = base->next ) {
	cur = calloc(1,sizeof(SplinePointList));
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;

	first = NULL;
	for ( pt=base->first; pt!=NULL && pt!=first; pt = pt->next->to ) {
	    cpt = malloc(sizeof(SplinePoint));
	    *cpt = *pt;
	    cpt->next = cpt->prev = NULL;
	    if ( cur->first==NULL )
		cur->first = cur->last = cpt;
	    else {
		spline = malloc(sizeof(Spline));
		*spline = *pt->prev;
		spline->from = cur->last;
		cur->last->next = spline;
		cpt->prev = spline;
		spline->to = cpt;
		spline->approx = NULL;
		cur->last = cpt;
	    }
	    if ( first==NULL ) first = pt;
	}
	if ( pt==first ) {
	    cpt = cur->first;
	    spline = malloc(sizeof(Spline));
	    *spline = *pt->prev;
	    spline->from = cur->last;
	    cur->last->next = spline;
	    cpt->prev = spline;
	    spline->to = cpt;
	    spline->approx = NULL;
	    cur->last = cpt;
	}
    }
return( head );
}

static void TransformPoint(SplinePoint *sp, double transform[6]) {
    BasePoint p;
    p.x = transform[0]*sp->me.x + transform[2]*sp->me.y + transform[4];
    p.y = transform[1]*sp->me.x + transform[3]*sp->me.y + transform[5];
    sp->me = p;
    if ( !sp->nonextcp ) {
	p.x = transform[0]*sp->nextcp.x + transform[2]*sp->nextcp.y + transform[4];
	p.y = transform[1]*sp->nextcp.x + transform[3]*sp->nextcp.y + transform[5];
	sp->nextcp = p;
    } else
	sp->nextcp = sp->me;
    if ( !sp->noprevcp ) {
	p.x = transform[0]*sp->prevcp.x + transform[2]*sp->prevcp.y + transform[4];
	p.y = transform[1]*sp->prevcp.x + transform[3]*sp->prevcp.y + transform[5];
	sp->prevcp = p;
    } else
	sp->prevcp = sp->me;
}

SplinePointList *SplinePointListTransform(SplinePointList *base, double transform[6], int allpoints ) {
    Spline *spline, *first, *last;
    SplinePointList *spl;

    for ( spl = base; spl!=NULL; spl = spl->next ) {
	first = NULL; last = NULL;
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    if ( allpoints || spline->from->selected )
		TransformPoint(spline->from,transform);
	    if ( last!=NULL && ( allpoints || spline->from->selected || last->from->selected ))
		RegenSpline(last);
	    last = spline;
	    if ( first==NULL ) first = spline;
	}
	if ( spline==NULL && last!=NULL ) {
	    if ( allpoints || last->to->selected ) {
		TransformPoint(last->to,transform);
		RegenSpline(last);
	    }
	} else if ( last!=NULL ) {
	    if ( allpoints || last->to->selected || last->from->selected ) {
		RegenSpline(last);
	    }
	}
    }
return( base );
}

static void InstanciateReference(SplineFont *sf, RefChar *topref, RefChar *refs,
	double transform[6]) {
    double trans[6];
    RefChar *rf;
    SplineChar *rsc;
    SplinePointList *spl, *new;
    int i;

    if ( refs->local_enc==-1 ) {
	for ( i=0; i<sf->charcnt; ++i )
	    if ( strcmp(sf->encoding[i],AdobeStandardEncoding[refs->adobe_enc])==0 )
	break;
	if ( i!=sf->charcnt && !sf->chars[i]->ticked )
	    refs->local_enc = i;
	else
return;
    } else if ( sf->chars[refs->local_enc]->ticked )
return;

    rsc = sf->chars[refs->local_enc];
    rsc->ticked = true;

    for ( rf=rsc->refs; rf!=NULL; rf = rf->next ) {
	trans[0] = transform[0]*rf->transform[0] +
		    transform[1]*rf->transform[2];
	trans[1] = transform[0]*rf->transform[1] +
		    transform[1]*rf->transform[3];
	trans[2] = transform[2]*rf->transform[0] +
		    transform[3]*rf->transform[2];
	trans[3] = transform[2]*rf->transform[1] +
		    transform[3]*rf->transform[3];
	trans[4] = transform[4]*rf->transform[0] +
		    transform[5]*rf->transform[2] +
		    rf->transform[4];
	trans[5] = transform[4]*rf->transform[1] +
		    transform[5]*rf->transform[3] +
		    rf->transform[5];
	InstanciateReference(sf,topref,rf,trans);
    }
    rsc->ticked = false;

    for ( spl = rsc->splines; spl!=NULL; spl = spl->next ) {
	new = SplinePointListTransform(SplinePointListCopy(spl,true),transform,true);
	new->next = topref->splines;
	topref->splines = new;
    }
}
	
SplineFont *SplineFontFromPSFont(FontDict *fd) {
    SplineFont *sf = calloc(1,sizeof(SplineFont));
    int em;
    int i;
    RefChar *refs;

    sf->fontname = copy(fd->fontname);
    if ( fd->fontinfo!=NULL ) {
	if ( sf->fontname==NULL ) sf->fontname = copy(fd->fontinfo->fullname);
	if ( sf->fontname==NULL ) sf->fontname = copy(fd->fontinfo->familyname);
	sf->fullname = copy(fd->fontinfo->fullname);
	sf->familyname = copy(fd->fontinfo->familyname);
	sf->weight = copy(fd->fontinfo->weight);
	sf->version = copy(fd->fontinfo->version);
	sf->copyright = copy(fd->fontinfo->notice);
	sf->italicangle = fd->fontinfo->italicangle;
	sf->upos = fd->fontinfo->underlineposition;
	sf->uwidth = fd->fontinfo->underlinethickness;
	sf->ascent = fd->fontinfo->ascent;
	sf->descent = fd->fontinfo->descent;
    }
    em = rint(1/fd->fontmatrix[0]);
    if ( sf->ascent==0 && sf->descent!=0 )
	sf->ascent = em-sf->descent;
    else if ( sf->ascent==0 )
	sf->ascent = 8*em/10;
    sf->descent = em-sf->ascent;
    sf->charcnt = 256+CharsNotInEncoding(fd);

    sf->encoding = calloc(sf->charcnt,sizeof(char *));
    sf->chars = calloc(sf->charcnt,sizeof(SplineChar *));
    for ( i=0; i<256; ++i ) if ( fd->encoding[i]==NULL )
	fd->encoding[i] = copy(".notdef");
    for ( i=0; i<256; ++i )
	sf->encoding[i] = copy(fd->encoding[i]);
    if ( sf->charcnt>256 ) {
	int k, j;
	for ( k=0; k<fd->chars->cnt; ++k ) {
	    if ( fd->chars->keys[k]!=NULL ) {
		for ( j=0; j<256; ++j )
		    if ( fd->encoding[j]!=NULL &&
			    strcmp(fd->encoding[j],fd->chars->keys[k])==0 )
		break;
		if ( j==256 )
		    sf->encoding[i++] = copy(fd->chars->keys[k]);
	    }
	}
    }
    for ( i=0; i<sf->charcnt; ++i ) {
	int k= -1;
	k = LookupCharString(sf->encoding[i],fd->chars);
	if ( k==-1 )
	    k = LookupCharString(".notdef",fd->chars);
	if ( k!=-1 )
	    sf->chars[i] = Type1BytesToSplines(fd->chars->values[k],fd->chars->lens[k],fd->private->subrs,sf->encoding[i]);
	else	/* 0 500 hsbw endchar */
	    sf->chars[i] = Type1BytesToSplines((unsigned char *)"\213\370\210\015\016",5,fd->private->subrs,".notdef");
	sf->chars[i]->enc = i;
	sf->chars[i]->unicodeenc = UnicodeNameLookup(sf->encoding[i]);
	sf->chars[i]->parent = sf;
    }
    for ( i=0; i<sf->charcnt; ++i ) for ( refs = sf->chars[i]->refs; refs!=NULL; refs=refs->next ) {
	sf->chars[i]->ticked = true;
	InstanciateReference(sf, refs, refs, refs->transform);
	SplineSetFindBounds(refs->splines,&refs->bb);
	sf->chars[i]->ticked = false;
    }

return( sf );
}

double SplineSolveY(Spline *sp, double tmin, double tmax, double sought_y) {
    /* We want to find t so that Yspline(t) = sought_y */
    /*  the curve must be monotonic */
    /* returns t which is near sought_y or -1 */
    double slope;
    double new_t, newer_t;
    double found_y;
    double t_ymax, t_ymin;
    double ymax, ymin;
    int up;

    if ( sp->ay==0 && sp->by==0 ) {
	if ( sp->cy==0 )
return( sought_y==sp->dy ? 0 : -1 );
	new_t = (sought_y-sp->dy)/sp->cy;
	if ( new_t >= 0 && new_t<=1 )
return( new_t );
return( -1 );
    }

    ymax = ((sp->ay*tmax + sp->by)*tmax + sp->cy)*tmax + sp->dy;
    ymin = ((sp->ay*tmin + sp->by)*tmin + sp->cy)*tmin + sp->dy;
    if ( ymax<ymin ) {
	double temp = ymax;
	ymax = ymin;
	ymin = temp;
	t_ymax = tmin;
	t_ymin = tmax;
	up = false;
    } else {
	t_ymax = tmax;
	t_ymin = tmin;
	up = true;
    }
    if ( sought_y<ymin || sought_y>ymax )
return( -1 );

    slope = (3.0*sp->ay*t_ymin+2.0*sp->by)*t_ymin + sp->cy;
    if ( slope==0 )	/* we often start at a point of inflection */
	new_t = t_ymin + (t_ymax-t_ymin)* (sought_y-ymin)/(ymax-ymin);
    else {
	new_t = t_ymin + (sought_y-ymin)/slope;
	if (( new_t>t_ymax && up ) ||
		(new_t<t_ymax && !up) ||
		(new_t<t_ymin && up ) ||
		(new_t>t_ymin && !up ))
	    new_t = t_ymin + (t_ymax-t_ymin)* (sought_y-ymin)/(ymax-ymin);
    }

    while ( 1 ) {
	found_y = ((sp->ay*new_t+sp->by)*new_t+sp->cy)*new_t + sp->dy ;
	if ( found_y>sought_y-.001 && found_y<sought_y+.001 ) {
return( new_t );
	}
	newer_t = (t_ymax-t_ymin) * (sought_y-found_y)/(ymax-ymin);
	if ( found_y > sought_y ) {
	    ymax = found_y;
	    t_ymax = new_t;
	} else {
	    ymin = found_y;
	    t_ymin = new_t;
	}
	if ( t_ymax==t_ymin )
return( -1 );
	new_t = newer_t;
	if (( new_t>t_ymax && up ) ||
		(new_t<t_ymax && !up) ||
		(new_t<t_ymin && up ) ||
		(new_t>t_ymin && !up ))
	    new_t = (t_ymax+t_ymin)/2;
    }
}

double SplineSolveX(Spline *sp, double tmin, double tmax, double sought_x) {
    /* We want to find t so that Yspline(t) = sought_y */
    /*  the curve must be monotonic */
    /* returns t which is near sought_x or -1 */
    double slope;
    double new_t, newer_t;
    double found_x;
    double t_xmax, t_xmin;
    double xmax, xmin;
    int up;

    if ( sp->ax==0 && sp->bx==0 ) {
	if ( sp->cx==0 )
return( sought_x==sp->dx ? 0 : -1 );
	new_t = (sought_x-sp->dx)/sp->cx;
	if ( new_t >= 0 && new_t<=1 )
return( new_t );
return( -1 );
    }

    xmax = ((sp->ax*tmax + sp->bx)*tmax + sp->cx)*tmax + sp->dx;
    xmin = ((sp->ax*tmin + sp->bx)*tmin + sp->cx)*tmin + sp->dx;
    if ( xmax<xmin ) {
	double temp = xmax;
	xmax = xmin;
	xmin = temp;
	t_xmax = tmin;
	t_xmin = tmax;
	up = false;
    } else {
	t_xmax = tmax;
	t_xmin = tmin;
	up = true;
    }
    if ( sought_x<xmin || sought_x>xmax )
return( -1 );

    slope = (3.0*sp->ax*t_xmin+2.0*sp->bx)*t_xmin + sp->cx;
    if ( slope==0 )	/* we often start at a point of inflection */
	new_t = t_xmin + (t_xmax-t_xmin)* (sought_x-xmin)/(xmax-xmin);
    else {
	new_t = t_xmin + (sought_x-xmin)/slope;
	if (( new_t>t_xmax && up ) ||
		(new_t<t_xmax && !up) ||
		(new_t<t_xmin && up ) ||
		(new_t>t_xmin && !up ))
	    new_t = t_xmin + (t_xmax-t_xmin)* (sought_x-xmin)/(xmax-xmin);
    }

    while ( 1 ) {
	found_x = ((sp->ax*new_t+sp->bx)*new_t+sp->cx)*new_t + sp->dx ;
	if ( found_x>sought_x-.001 && found_x<sought_x+.001 ) {
return( new_t );
	}
	newer_t = (t_xmax-t_xmin) * (sought_x-found_x)/(xmax-xmin);
	if ( found_x > sought_x ) {
	    xmax = found_x;
	    t_xmax = new_t;
	} else {
	    xmin = found_x;
	    t_xmin = new_t;
	}
	if ( t_xmax==t_xmin )
return( -1 );
	new_t = newer_t;
	if (( new_t>t_xmax && up ) ||
		(new_t<t_xmax && !up) ||
		(new_t<t_xmin && up ) ||
		(new_t>t_xmin && !up ))
	    new_t = (t_xmax+t_xmin)/2;
    }
}

void SplineFindYInflections(Spline *sp, double *_t1, double *_t2 ) {
    double t1= -1, t2= -1;
    double b2_fourac;

    /* Find the points of inflection on the curve discribing y behavior */
    /*  Set to -1 if there are none or if they are outside the range [0,1] */
    /*  Order them so that t1<t2 */
    /*  If only one valid inflection it will be t1 */
    if ( sp->ay!=0 ) {
	/* cubic, possibly 2 inflections (possibly none) */
	b2_fourac = 4*sp->by*sp->by - 12*sp->ay*sp->cy;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t1 = (-2*sp->by - b2_fourac) / (6*sp->ay);
	    t2 = (-2*sp->by + b2_fourac) / (6*sp->ay);
	    if ( t1<t2 ) { double temp = t1; t1 = t2; t2 = temp; }
	    else if ( t1==t2 ) t2 = -1;
	    if ( t2<0 || t2>1 ) t2 = -1;
	    if ( t1<0 || t1>1 ) { t1 = t2; t2 = -1; }
	}
    } else if ( sp->by!=0 ) {
	/* Quadratic, at most one inflection */
	t1 = -sp->cy/(2.0*sp->by);
	if ( t1<0 || t1>1 ) t1 = -1;
    } else /*if ( sp->cy!=0 )*/ {
	/* linear, no points of inflection */
    }
    *_t1 = t1; *_t2 = t2;
}

void SplineFindXInflections(Spline *sp, double *_t1, double *_t2 ) {
    double t1= -1, t2= -1;
    double b2_fourac;

    /* Find the points of inflection on the curve discribing x behavior */
    /*  Set to -1 if there are none or if they are outside the range [0,1] */
    /*  Order them so that t1<t2 */
    /*  If only one valid inflection it will be t1 */
    if ( sp->ax!=0 ) {
	/* cubic, possibly 2 inflections (possibly none) */
	b2_fourac = 4*sp->bx*sp->bx - 12*sp->ax*sp->cx;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t1 = (-2*sp->bx - b2_fourac) / (6*sp->ax);
	    t2 = (-2*sp->bx + b2_fourac) / (6*sp->ax);
	    if ( t1<t2 ) { double temp = t1; t1 = t2; t2 = temp; }
	    else if ( t1==t2 ) t2 = -1;
	    if ( t2<0 || t2>1 ) t2 = -1;
	    if ( t1<0 || t1>1 ) { t1 = t2; t2 = -1; }
	}
    } else if ( sp->bx!=0 ) {
	/* Quadratic, at most one inflection */
	t1 = -sp->cx/(2.0*sp->bx);
	if ( t1<0 || t1>1 ) t1 = -1;
    } else /*if ( sp->cx!=0 )*/ {
	/* linear, no points of inflection */
    }
    *_t1 = t1; *_t2 = t2;
}

void SplinePointListFree(SplinePointList *spl) {
    SplinePointList *next;

    while ( spl!=NULL ) {
	next = spl->next;
	/* free contents */;
	free( spl );
	spl = next;
    }
}

void SplineFontFree(SplineFont *sf) {
}
