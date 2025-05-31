/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2002, Randy Reddig & seaw0lf
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the names of the copyright holders nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCidentAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   ----------------------------------------------------------------------------- */

#include <pmpmesh/pm_internal.hpp>
#include <pmpmesh/pmpmesh.hpp>
#include <sstream>

/* mdc model format */
const char *MDC_MAGIC = "IDPC";
const int MDC_version = 2;

/* mdc vertex scale */
const float MDC_SCALE           = ( 1.0f / 64.0f );
const float MDC_MAX_OFS         = 127.0f;
const float MDC_DIST_SCALE      = 0.05f;

/* mdc decoding normal table */
double mdcNormals[ 256 ][ 3 ] =
{
	{ 1.000000, 0.000000, 0.000000 },
	{ 0.980785, 0.195090, 0.000000 },
	{ 0.923880, 0.382683, 0.000000 },
	{ 0.831470, 0.555570, 0.000000 },
	{ 0.707107, 0.707107, 0.000000 },
	{ 0.555570, 0.831470, 0.000000 },
	{ 0.382683, 0.923880, 0.000000 },
	{ 0.195090, 0.980785, 0.000000 },
	{ -0.000000, 1.000000, 0.000000 },
	{ -0.195090, 0.980785, 0.000000 },
	{ -0.382683, 0.923880, 0.000000 },
	{ -0.555570, 0.831470, 0.000000 },
	{ -0.707107, 0.707107, 0.000000 },
	{ -0.831470, 0.555570, 0.000000 },
	{ -0.923880, 0.382683, 0.000000 },
	{ -0.980785, 0.195090, 0.000000 },
	{ -1.000000, -0.000000, 0.000000 },
	{ -0.980785, -0.195090, 0.000000 },
	{ -0.923880, -0.382683, 0.000000 },
	{ -0.831470, -0.555570, 0.000000 },
	{ -0.707107, -0.707107, 0.000000 },
	{ -0.555570, -0.831469, 0.000000 },
	{ -0.382684, -0.923880, 0.000000 },
	{ -0.195090, -0.980785, 0.000000 },
	{ 0.000000, -1.000000, 0.000000 },
	{ 0.195090, -0.980785, 0.000000 },
	{ 0.382684, -0.923879, 0.000000 },
	{ 0.555570, -0.831470, 0.000000 },
	{ 0.707107, -0.707107, 0.000000 },
	{ 0.831470, -0.555570, 0.000000 },
	{ 0.923880, -0.382683, 0.000000 },
	{ 0.980785, -0.195090, 0.000000 },
	{ 0.980785, 0.000000, -0.195090 },
	{ 0.956195, 0.218245, -0.195090 },
	{ 0.883657, 0.425547, -0.195090 },
	{ 0.766809, 0.611510, -0.195090 },
	{ 0.611510, 0.766809, -0.195090 },
	{ 0.425547, 0.883657, -0.195090 },
	{ 0.218245, 0.956195, -0.195090 },
	{ -0.000000, 0.980785, -0.195090 },
	{ -0.218245, 0.956195, -0.195090 },
	{ -0.425547, 0.883657, -0.195090 },
	{ -0.611510, 0.766809, -0.195090 },
	{ -0.766809, 0.611510, -0.195090 },
	{ -0.883657, 0.425547, -0.195090 },
	{ -0.956195, 0.218245, -0.195090 },
	{ -0.980785, -0.000000, -0.195090 },
	{ -0.956195, -0.218245, -0.195090 },
	{ -0.883657, -0.425547, -0.195090 },
	{ -0.766809, -0.611510, -0.195090 },
	{ -0.611510, -0.766809, -0.195090 },
	{ -0.425547, -0.883657, -0.195090 },
	{ -0.218245, -0.956195, -0.195090 },
	{ 0.000000, -0.980785, -0.195090 },
	{ 0.218245, -0.956195, -0.195090 },
	{ 0.425547, -0.883657, -0.195090 },
	{ 0.611510, -0.766809, -0.195090 },
	{ 0.766809, -0.611510, -0.195090 },
	{ 0.883657, -0.425547, -0.195090 },
	{ 0.956195, -0.218245, -0.195090 },
	{ 0.923880, 0.000000, -0.382683 },
	{ 0.892399, 0.239118, -0.382683 },
	{ 0.800103, 0.461940, -0.382683 },
	{ 0.653281, 0.653281, -0.382683 },
	{ 0.461940, 0.800103, -0.382683 },
	{ 0.239118, 0.892399, -0.382683 },
	{ -0.000000, 0.923880, -0.382683 },
	{ -0.239118, 0.892399, -0.382683 },
	{ -0.461940, 0.800103, -0.382683 },
	{ -0.653281, 0.653281, -0.382683 },
	{ -0.800103, 0.461940, -0.382683 },
	{ -0.892399, 0.239118, -0.382683 },
	{ -0.923880, -0.000000, -0.382683 },
	{ -0.892399, -0.239118, -0.382683 },
	{ -0.800103, -0.461940, -0.382683 },
	{ -0.653282, -0.653281, -0.382683 },
	{ -0.461940, -0.800103, -0.382683 },
	{ -0.239118, -0.892399, -0.382683 },
	{ 0.000000, -0.923880, -0.382683 },
	{ 0.239118, -0.892399, -0.382683 },
	{ 0.461940, -0.800103, -0.382683 },
	{ 0.653281, -0.653282, -0.382683 },
	{ 0.800103, -0.461940, -0.382683 },
	{ 0.892399, -0.239117, -0.382683 },
	{ 0.831470, 0.000000, -0.555570 },
	{ 0.790775, 0.256938, -0.555570 },
	{ 0.672673, 0.488726, -0.555570 },
	{ 0.488726, 0.672673, -0.555570 },
	{ 0.256938, 0.790775, -0.555570 },
	{ -0.000000, 0.831470, -0.555570 },
	{ -0.256938, 0.790775, -0.555570 },
	{ -0.488726, 0.672673, -0.555570 },
	{ -0.672673, 0.488726, -0.555570 },
	{ -0.790775, 0.256938, -0.555570 },
	{ -0.831470, -0.000000, -0.555570 },
	{ -0.790775, -0.256938, -0.555570 },
	{ -0.672673, -0.488726, -0.555570 },
	{ -0.488725, -0.672673, -0.555570 },
	{ -0.256938, -0.790775, -0.555570 },
	{ 0.000000, -0.831470, -0.555570 },
	{ 0.256938, -0.790775, -0.555570 },
	{ 0.488725, -0.672673, -0.555570 },
	{ 0.672673, -0.488726, -0.555570 },
	{ 0.790775, -0.256938, -0.555570 },
	{ 0.707107, 0.000000, -0.707107 },
	{ 0.653281, 0.270598, -0.707107 },
	{ 0.500000, 0.500000, -0.707107 },
	{ 0.270598, 0.653281, -0.707107 },
	{ -0.000000, 0.707107, -0.707107 },
	{ -0.270598, 0.653282, -0.707107 },
	{ -0.500000, 0.500000, -0.707107 },
	{ -0.653281, 0.270598, -0.707107 },
	{ -0.707107, -0.000000, -0.707107 },
	{ -0.653281, -0.270598, -0.707107 },
	{ -0.500000, -0.500000, -0.707107 },
	{ -0.270598, -0.653281, -0.707107 },
	{ 0.000000, -0.707107, -0.707107 },
	{ 0.270598, -0.653281, -0.707107 },
	{ 0.500000, -0.500000, -0.707107 },
	{ 0.653282, -0.270598, -0.707107 },
	{ 0.555570, 0.000000, -0.831470 },
	{ 0.481138, 0.277785, -0.831470 },
	{ 0.277785, 0.481138, -0.831470 },
	{ -0.000000, 0.555570, -0.831470 },
	{ -0.277785, 0.481138, -0.831470 },
	{ -0.481138, 0.277785, -0.831470 },
	{ -0.555570, -0.000000, -0.831470 },
	{ -0.481138, -0.277785, -0.831470 },
	{ -0.277785, -0.481138, -0.831470 },
	{ 0.000000, -0.555570, -0.831470 },
	{ 0.277785, -0.481138, -0.831470 },
	{ 0.481138, -0.277785, -0.831470 },
	{ 0.382683, 0.000000, -0.923880 },
	{ 0.270598, 0.270598, -0.923880 },
	{ -0.000000, 0.382683, -0.923880 },
	{ -0.270598, 0.270598, -0.923880 },
	{ -0.382683, -0.000000, -0.923880 },
	{ -0.270598, -0.270598, -0.923880 },
	{ 0.000000, -0.382683, -0.923880 },
	{ 0.270598, -0.270598, -0.923880 },
	{ 0.195090, 0.000000, -0.980785 },
	{ -0.000000, 0.195090, -0.980785 },
	{ -0.195090, -0.000000, -0.980785 },
	{ 0.000000, -0.195090, -0.980785 },
	{ 0.980785, 0.000000, 0.195090 },
	{ 0.956195, 0.218245, 0.195090 },
	{ 0.883657, 0.425547, 0.195090 },
	{ 0.766809, 0.611510, 0.195090 },
	{ 0.611510, 0.766809, 0.195090 },
	{ 0.425547, 0.883657, 0.195090 },
	{ 0.218245, 0.956195, 0.195090 },
	{ -0.000000, 0.980785, 0.195090 },
	{ -0.218245, 0.956195, 0.195090 },
	{ -0.425547, 0.883657, 0.195090 },
	{ -0.611510, 0.766809, 0.195090 },
	{ -0.766809, 0.611510, 0.195090 },
	{ -0.883657, 0.425547, 0.195090 },
	{ -0.956195, 0.218245, 0.195090 },
	{ -0.980785, -0.000000, 0.195090 },
	{ -0.956195, -0.218245, 0.195090 },
	{ -0.883657, -0.425547, 0.195090 },
	{ -0.766809, -0.611510, 0.195090 },
	{ -0.611510, -0.766809, 0.195090 },
	{ -0.425547, -0.883657, 0.195090 },
	{ -0.218245, -0.956195, 0.195090 },
	{ 0.000000, -0.980785, 0.195090 },
	{ 0.218245, -0.956195, 0.195090 },
	{ 0.425547, -0.883657, 0.195090 },
	{ 0.611510, -0.766809, 0.195090 },
	{ 0.766809, -0.611510, 0.195090 },
	{ 0.883657, -0.425547, 0.195090 },
	{ 0.956195, -0.218245, 0.195090 },
	{ 0.923880, 0.000000, 0.382683 },
	{ 0.892399, 0.239118, 0.382683 },
	{ 0.800103, 0.461940, 0.382683 },
	{ 0.653281, 0.653281, 0.382683 },
	{ 0.461940, 0.800103, 0.382683 },
	{ 0.239118, 0.892399, 0.382683 },
	{ -0.000000, 0.923880, 0.382683 },
	{ -0.239118, 0.892399, 0.382683 },
	{ -0.461940, 0.800103, 0.382683 },
	{ -0.653281, 0.653281, 0.382683 },
	{ -0.800103, 0.461940, 0.382683 },
	{ -0.892399, 0.239118, 0.382683 },
	{ -0.923880, -0.000000, 0.382683 },
	{ -0.892399, -0.239118, 0.382683 },
	{ -0.800103, -0.461940, 0.382683 },
	{ -0.653282, -0.653281, 0.382683 },
	{ -0.461940, -0.800103, 0.382683 },
	{ -0.239118, -0.892399, 0.382683 },
	{ 0.000000, -0.923880, 0.382683 },
	{ 0.239118, -0.892399, 0.382683 },
	{ 0.461940, -0.800103, 0.382683 },
	{ 0.653281, -0.653282, 0.382683 },
	{ 0.800103, -0.461940, 0.382683 },
	{ 0.892399, -0.239117, 0.382683 },
	{ 0.831470, 0.000000, 0.555570 },
	{ 0.790775, 0.256938, 0.555570 },
	{ 0.672673, 0.488726, 0.555570 },
	{ 0.488726, 0.672673, 0.555570 },
	{ 0.256938, 0.790775, 0.555570 },
	{ -0.000000, 0.831470, 0.555570 },
	{ -0.256938, 0.790775, 0.555570 },
	{ -0.488726, 0.672673, 0.555570 },
	{ -0.672673, 0.488726, 0.555570 },
	{ -0.790775, 0.256938, 0.555570 },
	{ -0.831470, -0.000000, 0.555570 },
	{ -0.790775, -0.256938, 0.555570 },
	{ -0.672673, -0.488726, 0.555570 },
	{ -0.488725, -0.672673, 0.555570 },
	{ -0.256938, -0.790775, 0.555570 },
	{ 0.000000, -0.831470, 0.555570 },
	{ 0.256938, -0.790775, 0.555570 },
	{ 0.488725, -0.672673, 0.555570 },
	{ 0.672673, -0.488726, 0.555570 },
	{ 0.790775, -0.256938, 0.555570 },
	{ 0.707107, 0.000000, 0.707107 },
	{ 0.653281, 0.270598, 0.707107 },
	{ 0.500000, 0.500000, 0.707107 },
	{ 0.270598, 0.653281, 0.707107 },
	{ -0.000000, 0.707107, 0.707107 },
	{ -0.270598, 0.653282, 0.707107 },
	{ -0.500000, 0.500000, 0.707107 },
	{ -0.653281, 0.270598, 0.707107 },
	{ -0.707107, -0.000000, 0.707107 },
	{ -0.653281, -0.270598, 0.707107 },
	{ -0.500000, -0.500000, 0.707107 },
	{ -0.270598, -0.653281, 0.707107 },
	{ 0.000000, -0.707107, 0.707107 },
	{ 0.270598, -0.653281, 0.707107 },
	{ 0.500000, -0.500000, 0.707107 },
	{ 0.653282, -0.270598, 0.707107 },
	{ 0.555570, 0.000000, 0.831470 },
	{ 0.481138, 0.277785, 0.831470 },
	{ 0.277785, 0.481138, 0.831470 },
	{ -0.000000, 0.555570, 0.831470 },
	{ -0.277785, 0.481138, 0.831470 },
	{ -0.481138, 0.277785, 0.831470 },
	{ -0.555570, -0.000000, 0.831470 },
	{ -0.481138, -0.277785, 0.831470 },
	{ -0.277785, -0.481138, 0.831470 },
	{ 0.000000, -0.555570, 0.831470 },
	{ 0.277785, -0.481138, 0.831470 },
	{ 0.481138, -0.277785, 0.831470 },
	{ 0.382683, 0.000000, 0.923880 },
	{ 0.270598, 0.270598, 0.923880 },
	{ -0.000000, 0.382683, 0.923880 },
	{ -0.270598, 0.270598, 0.923880 },
	{ -0.382683, -0.000000, 0.923880 },
	{ -0.270598, -0.270598, 0.923880 },
	{ 0.000000, -0.382683, 0.923880 },
	{ 0.270598, -0.270598, 0.923880 },
	{ 0.195090, 0.000000, 0.980785 },
	{ -0.000000, 0.195090, 0.980785 },
	{ -0.195090, -0.000000, 0.980785 },
	{ 0.000000, -0.195090, 0.980785 }
};

/* mdc model frame information */
class mdcFrame_t
{
public:
	float bounds[ 2 ][ 3 ];
	float localOrigin[ 3 ];
	float radius;
	char creator[ 16 ];
};

/* mdc model tag information */
class mdcTag_t
{
public:
	short xyz[3];
	short angles[3];
};

/* mdc surface mdc (one object mesh) */
class mdcSurface_t
{
public:
	char magic[ 4 ];
	char name[ 64 ];                /* polyset name */
	int flags;
	int numCompFrames;              /* all surfaces in a model should have the same */
	int numBaseFrames;              /* ditto */
	int num_shaders;                 /* all model surfaces should have the same */
	int numVerts;
	int numTriangles;
	int ofsTriangles;
	int ofsShaders;                 /* offset from start of mdcSurface_t */
	int ofsSt;                      /* texture coords are common for all frames */
	int ofsXyzNormals;              /* numVerts * numBaseFrames */
	int ofsXyzCompressed;           /* numVerts * numCompFrames */

	int ofsFrameBaseFrames;         /* numFrames */
	int ofsFrameCompFrames;         /* numFrames */
	int ofsEnd;                     /* next surface follows */
};

class mdcShader_t
{
public:
	char name[ 64 ];
	int shaderIndex;            /* for ingame use */
};

class mdcTriangle_t
{
public:
	int indices[ 3 ];
};

class mdcTexCoord_t
{
public:
	float st[ 2 ];
};

class mdcVertex_t
{
public:
	short xyz[ 3 ];
	short normal;
};

class mdcXyzCompressed_t
{
public:
	unsigned int ofsVec;        /* offset direction from the last base frame */
};


/* mdc model file mdc class */
class mdc_t
{
public:
	char magic[ 4 ];            /* MDC_MAGIC */
	int version;
	char name[ 64 ];            /* model name */
	int flags;
	int numFrames;
	int numTags;
	int num_surfaces;
	int numSkins;               /* number of skins for the mesh */
	int ofsFrames;              /* offset for first frame */
	int ofsTagNames;            /* numTags */
	int ofsTags;                /* numFrames * numTags */
	int ofsSurfaces;            /* first surface, others follow */
	int ofsEnd;                 /* end of file */
};

/*
   _mdc_canload()
   validates a Return to Castle Wolfenstein model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one class only.
 */

static int _mdc_canload( PM_PARAMS_CANLOAD ){
	const mdc_t *mdc;


	/* sanity check */
	if ( (pmm::size_type) bufSize < ( sizeof( *mdc ) * 2 ) ) {
		return pmm::pmv_error_size;
	}

	/* set as mdc */
	mdc = (const mdc_t*) buffer;

	/* check mdc magic */
	if ( *( (const int*) mdc->magic ) != *( (const int*) MDC_MAGIC ) ) {
		return pmm::pmv_error_ident;
	}

	/* check mdc version */
	if ( _pico_little_long( mdc->version ) != MDC_version ) {
		return pmm::pmv_error_version;
	}

	/* file seems to be a valid mdc */
	return pmm::pmv_ok;
}



/*
   _mdc_load()
   loads a Return to Castle Wolfenstein mdc model file.
 */

static pmm::model_t *_mdc_load( PM_PARAMS_LOAD ){
	int i, j;
	pmm::ub8_t          *bb, *bb0;
	mdc_t               *mdc;
	mdcSurface_t        *surface;
	mdcShader_t         *shader;
	mdcTexCoord_t       *texCoord;
	mdcFrame_t          *frame;
	mdcTriangle_t       *triangle;
	mdcVertex_t         *vertex;
	mdcXyzCompressed_t  *vertexComp = nullptr;
	short               *mdcShort, *mdcCompVert = nullptr;
	double lat, lng;

	pmm::model_t         *picoModel;
	pmm::surface_t       *picoSurface;
	pmm::shader_t        *picoShader;
	pmm::vec3_t xyz, normal;
	pmm::vec2_t st;
	pmm::color_t color;


	/* -------------------------------------------------
	   mdc loading
	   ------------------------------------------------- */


	/* set as mdc */
	bb0 = bb = (pmm::ub8_t*) pmm::man.pp_m_new( bufSize );
	memcpy( bb, buffer, bufSize );
	mdc = (mdc_t*) bb;

	/* check ident and version */
	if ( *( (int*) mdc->magic ) != *( (int*) MDC_MAGIC ) || _pico_little_long( mdc->version ) != MDC_version ) {
		/* not an mdc file (todo: set error) */
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	/* swap mdc */
	mdc->version = _pico_little_long( mdc->version );
	mdc->numFrames = _pico_little_long( mdc->numFrames );
	mdc->numTags = _pico_little_long( mdc->numTags );
	mdc->num_surfaces = _pico_little_long( mdc->num_surfaces );
	mdc->numSkins = _pico_little_long( mdc->numSkins );
	mdc->ofsFrames = _pico_little_long( mdc->ofsFrames );
	mdc->ofsTags = _pico_little_long( mdc->ofsTags );
	mdc->ofsTagNames = _pico_little_long( mdc->ofsTagNames );
	mdc->ofsSurfaces = _pico_little_long( mdc->ofsSurfaces );
	mdc->ofsEnd = _pico_little_long( mdc->ofsEnd );

	/* do frame check */
	if ( mdc->numFrames < 1 ) {
		pmm::man.pp_print(pmm::pl_error, "MDC with 0 frames");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	if ( frameNum < 0 || frameNum >= mdc->numFrames ) {
		pmm::man.pp_print(pmm::pl_error, "Invalid or out-of-range MDC frame specified");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	/* swap frames */
	frame = (mdcFrame_t*) ( bb + mdc->ofsFrames );
	for ( i = 0; i < mdc->numFrames; i++, frame++ )
	{
		frame->radius = _pico_little_float( frame->radius );
		for ( j = 0; j < 3; j++ )
		{
			frame->bounds[ 0 ][ j ] = _pico_little_float( frame->bounds[ 0 ][ j ] );
			frame->bounds[ 1 ][ j ] = _pico_little_float( frame->bounds[ 1 ][ j ] );
			frame->localOrigin[ j ] = _pico_little_float( frame->localOrigin[ j ] );
		}
	}

	/* swap surfaces */
	surface = (mdcSurface_t*) ( bb + mdc->ofsSurfaces );
	for ( i = 0; i < mdc->num_surfaces; i++ )
	{
		/* swap surface mdc */
		surface->flags = _pico_little_long( surface->flags );
		surface->numBaseFrames = _pico_little_long( surface->numBaseFrames );
		surface->numCompFrames = _pico_little_long( surface->numCompFrames );
		surface->num_shaders = _pico_little_long( surface->num_shaders );
		surface->numTriangles = _pico_little_long( surface->numTriangles );
		surface->ofsTriangles = _pico_little_long( surface->ofsTriangles );
		surface->numVerts = _pico_little_long( surface->numVerts );
		surface->ofsShaders = _pico_little_long( surface->ofsShaders );
		surface->ofsSt = _pico_little_long( surface->ofsSt );
		surface->ofsXyzNormals = _pico_little_long( surface->ofsXyzNormals );
		surface->ofsXyzCompressed = _pico_little_long( surface->ofsXyzCompressed );
		surface->ofsFrameBaseFrames = _pico_little_long( surface->ofsFrameBaseFrames );
		surface->ofsFrameCompFrames = _pico_little_long( surface->ofsFrameCompFrames );
		surface->ofsEnd = _pico_little_long( surface->ofsEnd );

		/* swap triangles */
		triangle = (mdcTriangle_t*) ( (pmm::ub8_t*) surface + surface->ofsTriangles );
		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			/* sea: swaps fixed */
			triangle->indices[ 0 ] = _pico_little_long( triangle->indices[ 0 ] );
			triangle->indices[ 1 ] = _pico_little_long( triangle->indices[ 1 ] );
			triangle->indices[ 2 ] = _pico_little_long( triangle->indices[ 2 ] );
		}

		/* swap st coords */
		texCoord = (mdcTexCoord_t*) ( (pmm::ub8_t*) surface + surface->ofsSt );
		for ( j = 0; j < surface->numVerts; j++, texCoord++ )
		{
			texCoord->st[ 0 ] = _pico_little_float( texCoord->st[ 0 ] );
			texCoord->st[ 1 ] = _pico_little_float( texCoord->st[ 1 ] );
		}

		/* swap xyz/normals */
		vertex = (mdcVertex_t*) ( (pmm::ub8_t*) surface + surface->ofsXyzNormals );
		for ( j = 0; j < ( surface->numVerts * surface->numBaseFrames ); j++, vertex++ )
		{
			vertex->xyz[ 0 ] = _pico_little_short( vertex->xyz[ 0 ] );
			vertex->xyz[ 1 ] = _pico_little_short( vertex->xyz[ 1 ] );
			vertex->xyz[ 2 ] = _pico_little_short( vertex->xyz[ 2 ] );
			vertex->normal   = _pico_little_short( vertex->normal );
		}

		/* swap xyz/compressed */
		vertexComp = (mdcXyzCompressed_t*) ( (pmm::ub8_t*) surface + surface->ofsXyzCompressed );
		for ( j = 0; j < ( surface->numVerts * surface->numCompFrames ); j++, vertexComp++ )
		{
			vertexComp->ofsVec  = _pico_little_long( vertexComp->ofsVec );
		}

		/* swap base frames */
		mdcShort = (short *) ( (pmm::ub8_t*) surface + surface->ofsFrameBaseFrames );
		for ( j = 0; j < mdc->numFrames; j++, mdcShort++ )
		{
			*mdcShort   = _pico_little_short( *mdcShort );
		}

		/* swap compressed frames */
		mdcShort = (short *) ( (pmm::ub8_t*) surface + surface->ofsFrameCompFrames );
		for ( j = 0; j < mdc->numFrames; j++, mdcShort++ )
		{
			*mdcShort   = _pico_little_short( *mdcShort );
		}

		/* get next surface */
		surface = (mdcSurface_t*) ( (pmm::ub8_t*) surface + surface->ofsEnd );
	}

	/* -------------------------------------------------
	   pico model creation
	   ------------------------------------------------- */

	/* create new pico model */
	picoModel = pmm::pp_new_model();
	if ( picoModel == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model");
		pmm::man.pp_m_delete( bb0 );
		return nullptr;
	}

	/* do model setup */
	pmm::pp_set_model_frame_num( picoModel, frameNum );
	pmm::pp_set_model_num_frames( picoModel, mdc->numFrames ); /* sea */
	pmm::pp_set_model_name( picoModel, fileName );
	pmm::pp_set_model_file_name( picoModel, fileName );

	/* mdc surfaces become picomodel surfaces */
	surface = (mdcSurface_t*) ( bb + mdc->ofsSurfaces );

	/* run through mdc surfaces */
	for ( i = 0; i < mdc->num_surfaces; i++ )
	{
		/* allocate new pico surface */
		picoSurface = pmm::pp_new_surface( picoModel );
		if ( picoSurface == nullptr ) {
			pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model surface");
			pmm::pp_free_model( picoModel ); /* sea */
			pmm::man.pp_m_delete( bb0 );
			return nullptr;
		}

		/* mdc model surfaces are all triangle meshes */
		pmm::pp_set_surface_type( picoSurface, pmm::st_triangles );

		/* set surface name */
		pmm::pp_set_surface_name( picoSurface, surface->name );

		/* create new pico shader -sea */
		picoShader = pmm::pp_new_shader( picoModel );
		if ( picoShader == nullptr ) {
			pmm::man.pp_print(pmm::pl_error, "Unable to allocate a new model shader");
			pmm::pp_free_model( picoModel );
			pmm::man.pp_m_delete( bb0 );
			return nullptr;
		}

		/* detox and set shader name */
		shader = (mdcShader_t*) ( (pmm::ub8_t*) surface + surface->ofsShaders );
		_pico_setfext( shader->name, "" );
		_pico_unixify( shader->name );
		pmm::pp_set_shader_name( picoShader, shader->name );

		/* associate current surface with newly created shader */
		pmm::pp_set_surface_shader( picoSurface, picoShader );

		/* copy indices */
		triangle = (mdcTriangle_t *) ( (pmm::ub8_t*) surface + surface->ofsTriangles );

		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 0 ), (pmm::index_t) triangle->indices[ 0 ] );
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 1 ), (pmm::index_t) triangle->indices[ 1 ] );
			pmm::pp_set_surface_index( picoSurface, ( j * 3 + 2 ), (pmm::index_t) triangle->indices[ 2 ] );
		}

		/* copy vertices */
		texCoord = (mdcTexCoord_t*) ( (pmm::ub8_t *) surface + surface->ofsSt );
		mdcShort = (short *) ( (pmm::ub8_t *) surface + surface->ofsXyzNormals ) + ( (int)*( (short *) ( (pmm::ub8_t *) surface + surface->ofsFrameBaseFrames ) + frameNum ) * surface->numVerts * 4 );
		if ( surface->numCompFrames > 0 ) {
			mdcCompVert = (short *) ( (pmm::ub8_t *) surface + surface->ofsFrameCompFrames ) + frameNum;
			if ( *mdcCompVert >= 0 ) {
				vertexComp = (mdcXyzCompressed_t *) ( (pmm::ub8_t *) surface + surface->ofsXyzCompressed ) + ( *mdcCompVert * surface->numVerts );
			}
		}
		_pico_set_color( color, 255, 255, 255, 255 );

		for ( j = 0; j < surface->numVerts; j++, texCoord++, mdcShort += 4 )
		{
			/* set vertex origin */
			xyz[ 0 ] = MDC_SCALE * mdcShort[ 0 ];
			xyz[ 1 ] = MDC_SCALE * mdcShort[ 1 ];
			xyz[ 2 ] = MDC_SCALE * mdcShort[ 2 ];

			/* add compressed ofsVec */
			if ( surface->numCompFrames > 0 && *mdcCompVert >= 0 ) {
				xyz[ 0 ] += ( (float) ( ( vertexComp->ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				xyz[ 1 ] += ( (float) ( ( vertexComp->ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				xyz[ 2 ] += ( (float) ( ( vertexComp->ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				pmm::pp_set_surface_xyz( picoSurface, j, xyz );

				normal[ 0 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 0 ];
				normal[ 1 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 1 ];
				normal[ 2 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 2 ];
				pmm::pp_set_surface_normal( picoSurface, j, normal );

				vertexComp++;
			}
			else
			{
				pmm::pp_set_surface_xyz( picoSurface, j, xyz );

				/* decode lat/lng normal to 3 float normal */
				lat = (float) ( ( *( mdcShort + 3 ) >> 8 ) & 0xff );
				lng = (float) ( *( mdcShort + 3 ) & 0xff );
				lat *= PICO_PI / 128;
				lng *= PICO_PI / 128;
				normal[ 0 ] = (pmm::vec_t) cos( lat ) * (pmm::vec_t) sin( lng );
				normal[ 1 ] = (pmm::vec_t) sin( lat ) * (pmm::vec_t) sin( lng );
				normal[ 2 ] = (pmm::vec_t) cos( lng );
				pmm::pp_set_surface_normal( picoSurface, j, normal );
			}

			/* set st coords */
			st[ 0 ] = texCoord->st[ 0 ];
			st[ 1 ] = texCoord->st[ 1 ];
			pmm::pp_set_surface_st( picoSurface, 0, j, st );

			/* set color */
			pmm::pp_set_surface_color( picoSurface, 0, j, color );
		}

		/* get next surface */
		surface = (mdcSurface_t*) ( (pmm::ub8_t*) surface + surface->ofsEnd );
	}

	/* return the new pico model */
	pmm::man.pp_m_delete( bb0 );
	return picoModel;
}



/* pico file format module definition */
extern const pmm::module_t picoModuleMDC =
{
	"1.3",                          /* module version string */
	"RtCW MDC",                     /* module display name */
	"Arnout van Meer",              /* author's name */
	"2002 Arnout van Meer",         /* module copyright */
	{
		"mdc", nullptr, nullptr, nullptr     /* default extensions to use */
	},
	_mdc_canload,                   /* validation routine */
	_mdc_load,                      /* load routine */
	nullptr,                           /* save validation routine */
	nullptr                            /* save routine */
};
