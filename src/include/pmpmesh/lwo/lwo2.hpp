#pragma once

/*
   ======================================================================
   lwo2.hpp

   Definitions for LWO2 files.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include <functional>
#include "../platfdefs.hpp"

/* chunk and subchunk IDs */

#define LWID_( a,b,c,d ) ( ( ( a ) << 24 ) | ( ( b ) << 16 ) | ( ( c ) << 8 ) | ( d ) )

#define ID_FORM  LWID_( 'F','O','R','M' )
#define ID_LWO2  LWID_( 'L','W','O','2' )
#define ID_LWOB  LWID_( 'L','W','O','B' )

/* top-level chunks */
#define ID_LAYR  LWID_( 'L','A','Y','R' )
#define ID_TAGS  LWID_( 'T','A','G','S' )
#define ID_PNTS  LWID_( 'P','N','T','S' )
#define ID_BBOX  LWID_( 'B','B','O','X' )
#define ID_VMAP  LWID_( 'V','M','A','P' )
#define ID_VMAD  LWID_( 'V','M','A','D' )
#define ID_POLS  LWID_( 'P','O','L','S' )
#define ID_PTAG  LWID_( 'P','T','A','G' )
#define ID_ENVL  LWID_( 'E','N','V','L' )
#define ID_CLIP  LWID_( 'C','L','I','P' )
#define ID_SURF  LWID_( 'S','U','R','F' )
#define ID_DESC  LWID_( 'D','E','S','C' )
#define ID_TEXT  LWID_( 'T','E','X','T' )
#define ID_ICON  LWID_( 'I','C','O','N' )

/* polygon types */
#define ID_FACE  LWID_( 'F','A','C','E' )
#define ID_CURV  LWID_( 'C','U','R','V' )
#define ID_PTCH  LWID_( 'P','T','C','H' )
#define ID_MBAL  LWID_( 'M','B','A','L' )
#define ID_BONE  LWID_( 'B','O','N','E' )

/* polygon tags */
#define ID_SURF  LWID_( 'S','U','R','F' )
#define ID_PART  LWID_( 'P','A','R','T' )
#define ID_SMGP  LWID_( 'S','M','G','P' )

/* envelopes */
#define ID_PRE   LWID_( 'P','R','E',' ' )
#define ID_POST  LWID_( 'P','O','S','T' )
#define ID_KEY   LWID_( 'K','E','Y',' ' )
#define ID_SPAN  LWID_( 'S','P','A','N' )
#define ID_TCB   LWID_( 'T','C','B',' ' )
#define ID_HERM  LWID_( 'H','E','R','M' )
#define ID_BEZI  LWID_( 'B','E','Z','I' )
#define ID_BEZ2  LWID_( 'B','E','Z','2' )
#define ID_LINE  LWID_( 'L','I','N','E' )
#define ID_STEP  LWID_( 'S','T','E','P' )

/* clips */
#define ID_STIL  LWID_( 'S','T','I','L' )
#define ID_ISEQ  LWID_( 'I','S','E','Q' )
#define ID_ANIM  LWID_( 'A','N','I','M' )
#define ID_XREF  LWID_( 'X','R','E','F' )
#define ID_STCC  LWID_( 'S','T','C','C' )
#define ID_TIME  LWID_( 'T','I','M','E' )
#define ID_CONT  LWID_( 'C','O','N','T' )
#define ID_BRIT  LWID_( 'B','R','I','T' )
#define ID_SATR  LWID_( 'S','A','T','R' )
#define ID_HUE   LWID_( 'H','U','E',' ' )
#define ID_GAMM  LWID_( 'G','A','M','M' )
#define ID_NEGA  LWID_( 'N','E','G','A' )
#define ID_IFLT  LWID_( 'I','F','L','T' )
#define ID_PFLT  LWID_( 'P','F','L','T' )

/* surfaces */
#define ID_COLR  LWID_( 'C','O','L','R' )
#define ID_LUMI  LWID_( 'L','U','M','I' )
#define ID_DIFF  LWID_( 'D','I','F','F' )
#define ID_SPEC  LWID_( 'S','P','E','C' )
#define ID_GLOS  LWID_( 'G','L','O','S' )
#define ID_REFL  LWID_( 'R','E','F','L' )
#define ID_RFOP  LWID_( 'R','F','O','P' )
#define ID_RIMG  LWID_( 'R','I','M','G' )
#define ID_RSAN  LWID_( 'R','S','A','N' )
#define ID_TRAN  LWID_( 'T','R','A','N' )
#define ID_TROP  LWID_( 'T','R','O','P' )
#define ID_TIMG  LWID_( 'T','I','M','G' )
#define ID_RIND  LWID_( 'R','I','N','D' )
#define ID_TRNL  LWID_( 'T','R','N','L' )
#define ID_BUMP  LWID_( 'B','U','M','P' )
#define ID_SMAN  LWID_( 'S','M','A','N' )
#define ID_SIDE  LWID_( 'S','I','D','E' )
#define ID_CLRH  LWID_( 'C','L','R','H' )
#define ID_CLRF  LWID_( 'C','L','R','F' )
#define ID_ADTR  LWID_( 'A','D','T','R' )
#define ID_SHRP  LWID_( 'S','H','R','P' )
#define ID_LINE  LWID_( 'L','I','N','E' )
#define ID_LSIZ  LWID_( 'L','S','I','Z' )
#define ID_ALPH  LWID_( 'A','L','P','H' )
#define ID_AVAL  LWID_( 'A','V','A','L' )
#define ID_GVAL  LWID_( 'G','V','A','L' )
#define ID_BLok  LWID_( 'B','L','O','K' )

/* texture layer */
#define ID_TYPE  LWID_( 'T','Y','P','E' )
#define ID_CHAN  LWID_( 'C','H','A','N' )
#define ID_NAME  LWID_( 'N','A','M','E' )
#define ID_ENAB  LWID_( 'E','N','A','B' )
#define ID_OPAC  LWID_( 'O','P','A','C' )
#define ID_FLAG  LWID_( 'F','L','A','G' )
#define ID_PROJ  LWID_( 'P','R','O','J' )
#define ID_STCK  LWID_( 'S','T','C','K' )
#define ID_TAMP  LWID_( 'T','A','M','P' )

/* texture coordinates */
#define ID_TMAP  LWID_( 'T','M','A','P' )
#define ID_AXIS  LWID_( 'A','X','I','S' )
#define ID_CNTR  LWID_( 'C','N','T','R' )
#define ID_size  LWID_( 'S','I','Z','E' )
#define ID_ROTA  LWID_( 'R','O','T','A' )
#define ID_OREF  LWID_( 'O','R','E','F' )
#define ID_FALL  LWID_( 'F','A','L','L' )
#define ID_CSYS  LWID_( 'C','S','Y','S' )

/* image map */
#define ID_IMAP  LWID_( 'I','M','A','P' )
#define ID_IMAG  LWID_( 'I','M','A','G' )
#define ID_WRAP  LWID_( 'W','R','A','P' )
#define ID_WRPW  LWID_( 'W','R','P','W' )
#define ID_WRPH  LWID_( 'W','R','P','H' )
#define ID_VMAP  LWID_( 'V','M','A','P' )
#define ID_AAST  LWID_( 'A','A','S','T' )
#define ID_PIXB  LWID_( 'P','I','X','B' )

/* procedural */
#define ID_PROC  LWID_( 'P','R','O','C' )
#define ID_COLR  LWID_( 'C','O','L','R' )
#define ID_VALU  LWID_( 'V','A','L','U' )
#define ID_FUNC  LWID_( 'F','U','N','C' )
#define ID_FTPS  LWID_( 'F','T','P','S' )
#define ID_ITPS  LWID_( 'I','T','P','S' )
#define ID_ETPS  LWID_( 'E','T','P','S' )

/* gradient */
#define ID_GRAD  LWID_( 'G','R','A','D' )
#define ID_GRST  LWID_( 'G','R','S','T' )
#define ID_GREN  LWID_( 'G','R','E','N' )
#define ID_PNAM  LWID_( 'P','N','A','M' )
#define ID_INAM  LWID_( 'I','N','A','M' )
#define ID_GRPT  LWID_( 'G','R','P','T' )
#define ID_FKEY  LWID_( 'F','K','E','Y' )
#define ID_IKEY  LWID_( 'I','K','E','Y' )

/* shader */
#define ID_SHDR  LWID_( 'S','H','D','R' )
#define ID_DATA  LWID_( 'D','A','T','A' )


/* generic linked list */

class lwNode
{
public:
	lwNode *next, *prev;
	void *data;
};

/* plug-in reference */

class lwPlugin
{
public:
	lwPlugin *next, *prev;
	char          *ord;
	char          *name;
	int flags;
	void          *data;
};

/* envelopes */

class lwKey
{
public:
	lwKey *next, *prev;
	float value;
	float time;
	unsigned int shape;                /* ID_TCB, ID_BEZ2, etc. */
	float tension;
	float continuity;
	float bias;
	float param[ 4 ];
};

class lwEnvelope
{
public:
	lwEnvelope *next, *prev;
	int index;
	int type;
	char          *name;
	lwKey         *key;                /* linked list of keys */
	int nkeys;
	int behavior[ 2 ];                 /* pre and post (extrapolation) */
	lwPlugin      *cfilter;            /* linked list of channel filters */
	int ncfilters;
};

#define BEH_RESET      0
#define BEH_CONSTANT   1
#define BEH_REPEAT     2
#define BEH_OSCILLATE  3
#define BEH_OFFSET     4
#define BEH_LINEAR     5


/* values that can be enveloped */

class lwEParam
{
public:
	float val;
	int eindex;
};

class lwVParam
{
public:
	float val[ 3 ];
	int eindex;
};

/* clips */

class lwClipStill
{
public:
	char          *name;
};

class lwClipSeq
{
public:
	char          *prefix;             /* filename before sequence digits */
	char          *suffix;             /* after digits, e.g. extensions */
	int digits;
	int flags;
	int offset;
	int start;
	int end;
};

class lwClipAnim
{
public:
	char          *name;
	char          *server;             /* anim loader plug-in */
	void          *data;
};

class lwClipXRef
{
public:
	char          *string;
	int index;
	class lwClip *clip;
};

class lwClipCycle
{
public:
	char          *name;
	int lo;
	int hi;
};

class lwClip
{
public:
	lwClip *next, *prev;
	int index;
	unsigned int type;                 /* ID_STIL, ID_ISEQ, etc. */
	union {
		lwClipStill still;
		lwClipSeq seq;
		lwClipAnim anim;
		lwClipXRef xref;
		lwClipCycle cycle;
	}              source;
	float start_time;
	float duration;
	float frame_rate;
	lwEParam contrast;
	lwEParam brightness;
	lwEParam saturation;
	lwEParam hue;
	lwEParam gamma;
	int negative;
	lwPlugin      *ifilter;            /* linked list of image filters */
	int nifilters;
	lwPlugin      *pfilter;            /* linked list of pixel filters */
	int npfilters;
};

/* textures */

class lwTMap
{
public:
	lwVParam size;
	lwVParam center;
	lwVParam rotate;
	lwVParam falloff;
	int fall_type;
	char          *ref_object;
	int coord_sys;
};

class lwImageMap
{
public:
	int cindex;
	int projection;
	char          *vmap_name;
	int axis;
	int wrapw_type;
	int wraph_type;
	lwEParam wrapw;
	lwEParam wraph;
	float aa_strength;
	int aas_flags;
	int pblend;
	lwEParam stck;
	lwEParam amplitude;
};

#define PROJ_PLANAR       0
#define PROJ_CYLINDRICAL  1
#define PROJ_SPHERICAL    2
#define PROJ_CUBIC        3
#define PROJ_FRONT        4

#define WRAP_NONE    0
#define WRAP_EDGE    1
#define WRAP_REPEAT  2
#define WRAP_MIRROR  3

class lwProcedural
{
public:
	int axis;
	float value[ 3 ];
	char          *name;
	void          *data;
};

class lwGradKey
{
public:
	lwGradKey *next, *prev;
	float value;
	float rgba[ 4 ];
};

class lwGradient
{
public:
	char          *paramname;
	char          *itemname;
	float start;
	float end;
	int repeat;
	lwGradKey     *key;                /* array of gradient keys */
	short         *ikey;               /* array of interpolation codes */
};

class lwTexture
{
public:
	lwTexture *next, *prev;
	char          *ord;
	unsigned int type;
	unsigned int chan;
	lwEParam opacity;
	short opac_type;
	short enabled;
	short negative;
	short axis;
	union {
		lwImageMap imap;
		lwProcedural proc;
		lwGradient grad;
	}              param;
	lwTMap tmap;
};

/* values that can be textured */

class lwTParam {
public:
	float val;
	int eindex;
	lwTexture     *tex;                /* linked list of texture layers */
};

class lwCParam
{
public:
	float rgb[ 3 ];
	int eindex;
	lwTexture     *tex;                /* linked list of texture layers */
};


/* surfaces */

class Glow
{
public:
	short enabled;
	short type;
	lwEParam intensity;
	lwEParam size;
};

class lwRMap
{
public:
	lwTParam val;
	int options;
	int cindex;
	float seam_angle;
};

class lwLine
{
public:
	short enabled;
	unsigned short flags;
	lwEParam size;
};

class lwSurface
{
public:
	lwSurface * next, * prev;
	char          *name;
	char          *srcname;
	lwCParam color;
	lwTParam luminosity;
	lwTParam diffuse;
	lwTParam specularity;
	lwTParam glossiness;
	lwRMap reflection;
	lwRMap transparency;
	lwTParam eta;
	lwTParam translucency;
	lwTParam bump;
	float smooth;
	int sideflags;
	float alpha;
	int alpha_mode;
	lwEParam color_hilite;
	lwEParam color_filter;
	lwEParam add_trans;
	lwEParam dif_sharp;
	lwEParam glow;
	lwLine line;
	lwPlugin      *shader;             /* linked list of shaders */
	int nshaders;
};


/* vertex maps */

class lwVMap
{
public:
	lwVMap * next, * prev;
	char          *name;
	unsigned int type;
	int dim;
	int nverts;
	int perpoly;
	int           *vindex;             /* array of point indices */
	int           *pindex;             /* array of polygon indices */
	float        **val;
};

class lwVMapPt
{
public:
	lwVMap        *vmap;
	int index;                         /* vindex or pindex element */
};


/* points and polygons */

class lwPoint
{
public:
	float pos[ 3 ];
	int npols;                         /* number of polygons sharing the point */
	int           *pol;                /* array of polygon indices */
	int nvmaps;
	lwVMapPt      *vm;                 /* array of vmap references */
};

class lwPolVert
{
public:
	int index;                         /* index into the point array */
	float norm[ 3 ];
	int nvmaps;
	lwVMapPt      *vm;                 /* array of vmap references */
};

class lwPolygon
{
public:
	lwSurface     *surf;
	int part;                          /* part index */
	int smoothgrp;                     /* smoothing group */
	int flags;
	unsigned int type;
	float norm[ 3 ];
	int nverts;
	lwPolVert     *v;                  /* array of vertex records */
};

class lwPointList
{
public:
	int count;
	int offset;                        /* only used during reading */
	lwPoint       *pt;                 /* array of points */
};

class lwPolygonList
{
public:
	int count;
	int offset;                        /* only used during reading */
	int vcount;                        /* total number of vertices */
	int voffset;                       /* only used during reading */
	lwPolygon     *pol;                /* array of polygons */
};


/* geometry layers */

class lwLayer
{
public:
	lwLayer *next, *prev;
	char          *name;
	int index;
	int parent;
	int flags;
	float pivot[ 3 ];
	float bbox[ 6 ];
	lwPointList point;
	lwPolygonList polygon;
	int nvmaps;
	lwVMap        *vmap;               /* linked list of vmaps */
};


/* tag strings */

class lwTagList
{
public:
	int count;
	int offset;                        /* only used during reading */
	char         **tag;                /* array of strings */
};


/* an object */

class lwObject
{
public:
	lwLayer       *layer;              /* linked list of layers */
	lwEnvelope    *env;                /* linked list of envelopes */
	lwClip        *clip;               /* linked list of clips */
	lwSurface     *surf;               /* linked list of surfaces */
	lwTagList taglist;
	int nlayers;
	int nenvs;
	int nclips;
	int nsurfs;
};


/* lwo2.cpp */

void lwFreeLayer( lwLayer *layer );
void lwFreeObject( lwObject *object );
lwObject *lwGetObject( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos );
int lwValidateObject( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos );

/* pntspols.cpp */

void lwFreePoints( lwPointList *point );
void lwFreePolygons( lwPolygonList *plist );
int lwGetPoints( picoMemStream_t *fp, int cksize, lwPointList *point );
void lwGetBoundingBox( lwPointList * point, float bbox[] );
int lwAllocPolygons( lwPolygonList *plist, int npols, int nverts );
int lwGetPolygons( picoMemStream_t *fp, int cksize, lwPolygonList *plist, int ptoffset );
void lwGetPolyNormals( lwPointList *point, lwPolygonList *polygon );
int lwGetPointPolygons( lwPointList *point, lwPolygonList *polygon );
int lwResolvePolySurfaces( lwPolygonList *polygon, lwTagList *tlist,
						   lwSurface **surf, int *nsurfs );
void lwGetVertNormals( lwPointList *point, lwPolygonList *polygon );
void lwFreeTags( lwTagList *tlist );
int lwGetTags( picoMemStream_t *fp, int cksize, lwTagList *tlist );
int lwGetPolygonTags( picoMemStream_t *fp, int cksize, lwTagList *tlist,
					  lwPolygonList *plist );

/* vmap.cpp */

void lwFreeVMap( lwVMap *vmap );
lwVMap *lwGetVMap( picoMemStream_t *fp, int cksize, int ptoffset, int poloffset,
				   int perpoly );
int lwGetPointVMaps( lwPointList *point, lwVMap *vmap );
int lwGetPolyVMaps( lwPolygonList *polygon, lwVMap *vmap );

/* clip.cpp */

void lwFreeClip( lwClip *clip );
lwClip *lwGetClip( picoMemStream_t *fp, int cksize );
lwClip *lwFindClip( lwClip *list, int index );

/* envelope.cpp */

void lwFreeEnvelope( lwEnvelope *env );
lwEnvelope *lwGetEnvelope( picoMemStream_t *fp, int cksize );
lwEnvelope *lwFindEnvelope( lwEnvelope *list, int index );
float lwEvalEnvelope( lwEnvelope *env, float time );

/* surface.cpp */

void lwFreePlugin( lwPlugin *p );
void lwFreeTexture( lwTexture *t );
void lwFreeSurface( lwSurface *surf );
int lwGetTHeader( picoMemStream_t *fp, int hsz, lwTexture *tex );
int lwGetTMap( picoMemStream_t *fp, int tmapsz, lwTMap *tmap );
int lwGetImageMap( picoMemStream_t *fp, int rsz, lwTexture *tex );
int lwGetProcedural( picoMemStream_t *fp, int rsz, lwTexture *tex );
int lwGetGradient( picoMemStream_t *fp, int rsz, lwTexture *tex );
lwTexture *lwGetTexture( picoMemStream_t *fp, int bloksz, unsigned int type );
lwPlugin *lwGetShader( picoMemStream_t *fp, int bloksz );
lwSurface *lwGetSurface( picoMemStream_t *fp, int cksize );
lwSurface *lwDefaultSurface( void );

/* lwob.cpp */

lwSurface *lwGetSurface5( picoMemStream_t *fp, int cksize, lwObject *obj );
int lwGetPolygons5( picoMemStream_t *fp, int cksize, lwPolygonList *plist, int ptoffset );
lwObject *lwGetObject5( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos );
int lwValidateObject5( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos );

/* list.cpp */

void lwListFree(void * list, std::function<void(void *)> freeNode);
void lwListAdd( void **list, void *node );
void lwListInsert( void **vlist, void *vitem,
				   int ( *compare )( void *, void * ) );

/* vecmath.cpp */

float dot( float a[], float b[] );
void cross( float a[], float b[], float c[] );
void normalize( float v[] );
#define vecangle( a, b ) ( float ) acos( dot( a, b ) )

/* lwio.cpp */

void  set_flen( int i );
int   get_flen( void );
void *getbytes( picoMemStream_t *fp, int size );
void  skipbytes( picoMemStream_t *fp, int n );
int   getI1( picoMemStream_t *fp );
short getI2( picoMemStream_t *fp );
int   getI4( picoMemStream_t *fp );
unsigned char  getU1( picoMemStream_t *fp );
unsigned short getU2( picoMemStream_t *fp );
unsigned int   getU4( picoMemStream_t *fp );
int   getVX( picoMemStream_t *fp );
float getF4( picoMemStream_t *fp );
char *getS0( picoMemStream_t *fp );
int   sgetI1( unsigned char **bp );
short sgetI2( unsigned char **bp );
int   sgetI4( unsigned char **bp );
unsigned char  sgetU1( unsigned char **bp );
unsigned short sgetU2( unsigned char **bp );
unsigned int   sgetU4( unsigned char **bp );
int   sgetVX( unsigned char **bp );
float sgetF4( unsigned char **bp );
char *sgetS0( unsigned char **bp );

#if !GDEF_ARCH_ENDIAN_BIG
void revbytes( void *bp, int elsize, int elcount );
#else
  #define revbytes( b, s, c )
#endif
