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

/* external modules */
extern const pmm::module_t picoModuleMD3;
extern const pmm::module_t picoModule3DS;
extern const pmm::module_t picoModuleASE;
extern const pmm::module_t picoModuleOBJ;
extern const pmm::module_t picoModuleMS3D;
extern const pmm::module_t picoModuleMDC;
extern const pmm::module_t picoModuleMD2;
extern const pmm::module_t picoModuleFM;
extern const pmm::module_t picoModuleLWO;
extern const pmm::module_t picoModuleTerrain;
extern const pmm::module_t picoModuleMDL;

/* list of all supported file format modules */
const pmm::module_t *picoModules[] =
{
	&picoModuleMD3,     /* quake3 arena md3 */
	&picoModule3DS,     /* autodesk 3ds */
	&picoModuleASE,     /* autodesk ase */
	&picoModuleMS3D,    /* milkshape3d */
	&picoModuleMDC,     /* return to castle wolfenstein mdc */
	&picoModuleMD2,     /* quake2 md2 */
	&picoModuleFM,      /* heretic2 fm */
	&picoModuleLWO,     /* lightwave object */
	&picoModuleTerrain, /* picoterrain object */
	&picoModuleOBJ,     /* wavefront object */
	&picoModuleMDL,     /* quake mdl */
	nullptr                /* arnold */
};



/*
   pmm::pp_module_list()
   returns a pointer to the module list and optionally stores
   the number of supported modules in 'numModules'. Note that
   this param can be nullptr when the count is not needed.
 */

const pmm::module_t **pmm::pp_module_list( int *numModules ){
	/* get module count */
	if ( numModules != nullptr ) {
		for ( ( *numModules ) = 0; picoModules[ *numModules ] != nullptr; ( *numModules )++ ) ;
	}

	/* return list of modules */
	return (const pmm::module_t**) picoModules;
}
