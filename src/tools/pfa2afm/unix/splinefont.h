#ifndef _SPLINEFONT_H
#define _SPLINEFONT_H

#include "basics.h"

typedef struct ipoint {
    int x;
    int y;
} IPoint;

typedef struct basepoint {
    double x;
    double y;
} BasePoint;

typedef struct dbounds {
    double minx, maxx;
    double miny, maxy;
} DBounds;

enum pointtype { pt_curve, pt_corner, pt_tangent };
typedef struct splinepoint {
    BasePoint me;
    BasePoint nextcp;		/* control point */
    BasePoint prevcp;		/* control point */
    unsigned int nonextcp:1;
    unsigned int noprevcp:1;
    unsigned int nextcpdef:1;
    unsigned int prevcpdef:1;
    unsigned int selected:1;	/* for UI */
    unsigned int pointtype:2;
    uint16 flex;		/* This is a flex serif have to go through icky flex output */
    struct spline *next;
    struct spline *prev;
} SplinePoint;

typedef struct linelist {
    IPoint here;
    struct linelist *next;
} LineList;

typedef struct linearapprox {
    double scale;
    unsigned int oneline: 1;
    unsigned int onepoint: 1;
    struct linelist *lines;
    struct linearapprox *next;
} LinearApprox;

typedef struct spline {
    unsigned int islinear: 1;
    SplinePoint *from, *to;
    double ax, bx, cx, dx;
    double ay, by, cy, dy;
    struct linearapprox *approx;
    /* Posible optimizations:
	Precalculate bounding box
	Precalculate points of inflection
    */
} Spline;

typedef struct splinepointlist {
    SplinePoint *first, *last;
    struct splinepointlist *next;
} SplinePointList;

typedef struct refchar {
    int adobe_enc, local_enc;
    double transform[6];	/* transformation matrix (first 2 rows of a 3x3 matrix, missing row is 0,0,1) */
    SplinePointList *splines;
    struct refchar *next;
    unsigned int selected: 1;
    DBounds bb;
} RefChar;

typedef struct hints {
    double base, width;
    double b1, b2, e1, e2;
    double ab, ae;
    unsigned int adjustb: 1;
    unsigned int adjuste: 1;
    struct hints *next;
} Hints;

typedef struct splinechar {
    char *name;
    int enc, unicodeenc;
    int width, lsidebearing;
    SplinePointList *splines;
    Hints *hstem;		/* hstem hints have a vertical offset but run horizontally */
    Hints *vstem;		/* vstem hints have a horizontal offset but run vertically */
    RefChar *refs;
    struct charview *views;
    struct splinefont *parent;
    unsigned int changed: 1;
    unsigned int changedsincelasthinted: 1;
    unsigned int ticked: 1;	/* For reference character processing */
} SplineChar;

typedef struct splinefont {
    char *fontname, *fullname, *familyname, *weight;
    char *copyright;
    char *filename;
    char *version;
    double italicangle, upos, uwidth;
    int ascent, descent;
    int charcnt;
    char **encoding;
    SplineChar **chars;
    unsigned int changed: 1;
} SplineFont;

typedef struct bdfchar {
    SplineChar *sc;
    int xmin,xmax,ymin,ymax;
    int width;
    int enc;
    int bytes_per_line;
    uint8 *bitmap;
} BDFChar;

typedef struct bdffont {
    SplineFont *sf;
    BDFChar **chars;		/* an array of sf->charcnt entries */
    int pixelsize;
    int ascent, descent;
} BDFFont;

extern void LineListFree(LineList *ll);
extern void LinearApproxFree(LinearApprox *la);
extern void SplineFree(Spline *spline);
extern void SplinePointFree(SplinePoint *sp);
extern void SplineRefigure(Spline *spline);
extern Spline *SplineMake(SplinePoint *from, SplinePoint *to);
extern LinearApprox *SplineApproximate(Spline *spline, double scale);
struct fontdict;
extern SplineFont *SplineFontFromPSFont(struct fontdict *fd);
extern void SplinePointListFree(SplinePointList *spl);
extern void SplineFontFree(SplineFont *sf);
extern void SplineCharFindBounds(SplineChar *sc,DBounds *bounds);
extern void SplineFontFindBounds(SplineFont *sf,DBounds *bounds);
extern SplinePointList *SplinePointListCopy(SplinePointList *base, int allpoints );
extern SplinePointList *SplinePointListTransform(SplinePointList *base, double transform[6], int allpoints );
extern BDFChar *SplineCharRasterize(SplineChar *sc, int pixelsize);
extern BDFFont *SplineFontRasterize(SplineFont *sf, int pixelsize);
extern void BDFCharFree(BDFChar *bdfc);
extern void BDFFontFree(BDFFont *bdf);
extern void BDFFontDump(char *filename,BDFFont *font, char *encodingname);
extern double SplineSolveY(Spline *sp, double tmin, double tmax, double sought_y);
extern double SplineSolveX(Spline *sp, double tmin, double tmax, double sought_x);
extern void SplineFindYInflections(Spline *sp, double *_t1, double *_t2 );
extern void SplineFindXInflections(Spline *sp, double *_t1, double *_t2 );
extern SplinePoint *SplineBisect(Spline *spline, double t);
extern SplinePointList *SplinePointListMerge(SplinePointList *spl);

extern void FindBlues( SplineFont *sf, double blues[14], double otherblues[10]);
extern void FindHStems( SplineFont *sf, double snaps[12], int cnt[12]);
extern void FindVStems( SplineFont *sf, double snaps[12], int cnt[12]);
extern void SplineCharAutoHint( SplineChar *sc);
extern void SplineFontAutoHint( SplineFont *sf);
extern int AfmSplineFont(FILE *afm, SplineFont *sf);
#endif
