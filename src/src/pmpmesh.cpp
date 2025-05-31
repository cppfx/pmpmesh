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

#include <pmpmesh/pmpmesh.hpp>
#include <pmpmesh/pm_internal.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

///////////////////////////////////////////////////////////////////////////

int pmm::pp_manager::pp_init()
{
	// todo
	return 1;
}

void pmm::pp_manager::pp_close()
{
	// todo
	return;
}

int pmm::pp_manager::pp_error()
{
	// todo
	return 0;
}

void * pmm::pp_manager::pp_m_new(pmm::size_type bytes) const
{
	if (bytes < 1u)
		return nullptr;
	auto * ptr = new pmm::ub8_t[bytes];
	std::fill_n(ptr, bytes, pmm::ub8_t{0});
	return ptr;
}

void * pmm::pp_manager::pp_k_new(pmm::size_type num, pmm::size_type bytes) const
{
	if (num == 0u || bytes == 0u)
		return nullptr;
	return this->pp_m_new(num * bytes);
}

void * pmm::pp_manager::pp_m_renew(void ** ptr, pmm::size_type oldsize, pmm::size_type newsize) const
{
	if (! ptr)
		return nullptr;
	if (newsize < oldsize)
		return *ptr;
	void * ptr2 = pmm::man.pp_m_new(newsize);
	if (! ptr2)
		return *ptr;
	if (*ptr)
	{
		std::copy_n(
			reinterpret_cast<pmm::ub8_t *>(* ptr),
			oldsize,
			reinterpret_cast<pmm::ub8_t *>(ptr2)
		);
		pmm::man.pp_m_delete(*ptr);
	}
	*ptr = ptr2;
	return *ptr;
}

void pmm::pp_manager::pp_m_delete(void * ptr) const
{
	if (! ptr)
		return;
	delete [] reinterpret_cast<pmm::ub8_t *>(ptr);
}

void pmm::pp_manager::pp_f_delete(void * ptr) const
{
	this->pp_m_delete(ptr);
}

void pmm::pp_manager::pp_set_file_loader(const pmm::pp_manager::file_loader_type & file_loader__)
{
	this->__file_loader = file_loader__;
}

int pmm::pp_manager::pp_load_file(const std::string & name__, pmm::ub8_t ** buffer__)
{
	if (name__.empty())
		return -1;
	if (! __file_loader)
		return -1;

	return __file_loader(name__, buffer__);
}

void pmm::pp_manager::pp_print(pmm::print_level level__, const std::string & str__) const
{
	std::string sub = str__;
	if (sub.back() == '\n')
		sub.back() == '\0';
	std::cout << sub << std::flush;
}

///////////////////////////////////////////////////////////////////////////

pmm::model_t *PicoModuleLoadModel( const pmm::module_t* pm, const char* fileName, pmm::ub8_t* buffer, int bufSize, int frameNum ){
	char                *modelFileName, *remapFileName;

	/* see whether this module can load the model file or not */
	if ( pm->canload( fileName, buffer, bufSize ) == pmm::pmv_ok ) {
		/* use loader provided by module to read the model data */
		pmm::model_t* model = pm->load( fileName, frameNum, buffer, bufSize );
		if ( model == nullptr ) {
			pmm::man.pp_f_delete(buffer);
			return nullptr;
		}

		/* assign pointer to file format module */
		model->module = pm;

		/* get model file name */
		modelFileName = pmm::pp_get_model_file_name( model );

		/* apply model remappings from <model>.remap */
		if ( strlen( modelFileName ) ) {
			/* alloc copy of model file name */
			remapFileName = reinterpret_cast<decltype(remapFileName)>(pmm::man.pp_m_new( strlen( modelFileName ) + 20 ));
			if ( remapFileName != nullptr ) {
				/* copy model file name and change extension */
				strcpy( remapFileName, modelFileName );
				_pico_setfext( remapFileName, "remap" );

				/* try to remap model; we don't handle the result */
				pmm::pp_remap_model( model, remapFileName );

				/* free the remap file name string */
				pmm::man.pp_m_delete( remapFileName );
			}
		}

		return model;
	}

	return nullptr;
}

/*
   pmm::pp_load_model()
   the meat and potatoes function
 */

pmm::model_t *pmm::pp_load_model( const char *fileName, int frameNum ){
	// make sure we've got a file name
	if ( fileName == nullptr )
	{
		pmm::man.pp_print(pmm::pl_error, "pmm::pp_load_model: No filename given (fileName == nullptr)");
		return nullptr;
	}

	const pmm::module_t  **modules, *pm;
	pmm::model_t         *model = nullptr;
	pmm::ub8_t          *buffer;

	// load file data (buffer is allocated by host app)
	int bufSize = pmm::man.pp_load_file(fileName, &buffer);

	if ( bufSize < 0 ) {
		pmm::man.pp_print(pmm::pl_error, (std::ostringstream{} << "pmm::pp_load_model: Failed loading model " << fileName).str());
		return nullptr;
	}

	/* get ptr to list of supported modules */
	modules = pmm::pp_module_list( nullptr );

	/* run it through the various loader functions and try */
	/* to find a loader that fits the given file data */
	for ( ; *modules != nullptr; modules++ )
	{
		/* get module */
		pm = *modules;

		/* sanity check */
		if ( pm == nullptr ) {
			break;
		}

		/* module must be able to load */
		if ( pm->canload == nullptr || pm->load == nullptr ) {
			continue;
		}

		model = PicoModuleLoadModel( pm, fileName, buffer, bufSize, frameNum );
		if ( model != nullptr ) {
			/* model was loaded, so break out of loop */
			break;
		}
	}

	/* free memory used by file buffer */
	if ( buffer ) {
		pmm::man.pp_f_delete(buffer);
	}

	/* return */
	return model;
}

pmm::model_t *pmm::pp_module_load_model_stream( const pmm::module_t* module, void* inputStream, pmm::pp_input_stream_read_func inputStreamRead, pmm::size_type streamLength, int frameNum, const char *fileName ){
	pmm::model_t         *model;
	pmm::ub8_t          *buffer;
	int bufSize;


	/* init */
	model = nullptr;

	if ( inputStream == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "pmm::pp_load_model: invalid input stream (inputStream == nullptr)");
		return nullptr;
	}

	if ( inputStreamRead == nullptr ) {
		pmm::man.pp_print(pmm::pl_error, "pmm::pp_load_model: invalid input stream (inputStreamRead == nullptr)");
		return nullptr;
	}

	buffer = reinterpret_cast<decltype(buffer)>(pmm::man.pp_m_new( streamLength + 1 ));

	bufSize = (int)inputStreamRead( inputStream, buffer, streamLength );
	buffer[bufSize] = '\0';

	model = PicoModuleLoadModel( module, fileName, buffer, bufSize, frameNum );

	if ( model != 0 ) {
		pmm::man.pp_m_delete( buffer );
	}

	/* return */
	return model;
}


/* ----------------------------------------------------------------------------
   models
   ---------------------------------------------------------------------------- */

/*
   pmm::pp_new_model()
   creates a new pico model
 */

pmm::model_t *pmm::pp_new_model( void ){
	pmm::model_t *model;

	/* allocate */
	model = reinterpret_cast<decltype(model)>(pmm::man.pp_m_new( sizeof( pmm::model_t ) ));
	if ( model == nullptr ) {
		return nullptr;
	}

	/* clear */
	memset( model,0,sizeof( pmm::model_t ) );

	/* model set up */
	_pico_zero_bounds( model->mins,model->maxs );

	/* set initial frame count to 1 -sea */
	model->numFrames = 1;

	/* return ptr to new model */
	return model;
}



/*
   pmm::pp_free_model()
   frees a model and all associated data
 */

void pmm::pp_free_model( pmm::model_t *model ){
	int i;


	/* sanity check */
	if ( model == nullptr ) {
		return;
	}

	/* free bits */
	if ( model->name ) {
		pmm::man.pp_m_delete( model->name );
	}

	if ( model->fileName ) {
		pmm::man.pp_m_delete( model->fileName );
	}

	/* free shaders */
	for ( i = 0; i < model->num_shaders; i++ )
		pmm::pp_free_shader( model->shader[ i ] );
	free( model->shader );

	/* free surfaces */
	for ( i = 0; i < model->num_surfaces; i++ )
		pmm::pp_free_surface( model->surface[ i ] );
	free( model->surface );

	/* free the model */
	pmm::man.pp_m_delete( model );
}



/*
   pmm::pp_adjust_model()
   adjusts a models's memory allocations to handle the requested sizes.
   will always grow, never shrink
 */

int pmm::pp_adjust_model( pmm::model_t *model, int num_shaders, int num_surfaces ){
	/* dummy check */
	if ( model == nullptr ) {
		return 0;
	}

	/* bare minimums */
	/* sea: null surface/shader fix (1s=>0s) */
	if ( num_shaders < 0 ) {
		num_shaders = 0;
	}
	if ( num_surfaces < 0 ) {
		num_surfaces = 0;
	}

	/* additional shaders? */
	while ( num_shaders > model->maxShaders )
	{
		model->maxShaders += pmm::ee_grow_shaders;
		if ( !pmm::man.pp_m_renew( (void **) &model->shader, model->num_shaders * sizeof( *model->shader ), model->maxShaders * sizeof( *model->shader ) ) ) {
			return 0;
		}
	}

	/* set shader count to higher */
	if ( num_shaders > model->num_shaders ) {
		model->num_shaders = num_shaders;
	}

	/* additional surfaces? */
	while ( num_surfaces > model->maxSurfaces )
	{
		model->maxSurfaces += pmm::ee_grow_surfaces;
		if ( !pmm::man.pp_m_renew( (void **) &model->surface, model->num_surfaces * sizeof( *model->surface ), model->maxSurfaces * sizeof( *model->surface ) ) ) {
			return 0;
		}
	}

	/* set shader count to higher */
	if ( num_surfaces > model->num_surfaces ) {
		model->num_surfaces = num_surfaces;
	}

	/* return ok */
	return 1;
}



/* ----------------------------------------------------------------------------
   shaders
   ---------------------------------------------------------------------------- */

/*
   pmm::pp_new_shader()
   creates a new pico shader and returns its index. -sea
 */

pmm::shader_t *pmm::pp_new_shader( pmm::model_t *model ){
	pmm::shader_t    *shader;


	/* allocate and clear */
	shader = reinterpret_cast<decltype(shader)>(pmm::man.pp_m_new( sizeof( pmm::shader_t ) ));
	if ( shader == nullptr ) {
		return nullptr;
	}
	memset( shader, 0, sizeof( pmm::shader_t ) );

	/* attach it to the model */
	if ( model != nullptr ) {
		/* adjust model */
		if ( !pmm::pp_adjust_model( model, model->num_shaders + 1, 0 ) ) {
			pmm::man.pp_m_delete( shader );
			return nullptr;
		}

		/* attach */
		model->shader[ model->num_shaders - 1 ] = shader;
		shader->model = model;
	}

	/* setup default shader colors */
	_pico_set_color( shader->ambientColor,0,0,0,0 );
	_pico_set_color( shader->diffuseColor,255,255,255,1 );
	_pico_set_color( shader->specularColor,0,0,0,0 );

	/* no need to do this, but i do it anyway */
	shader->transparency = 0;
	shader->shininess = 0;

	/* return the newly created shader */
	return shader;
}



/*
   pmm::pp_free_shader()
   frees a shader and all associated data -sea
 */

void pmm::pp_free_shader( pmm::shader_t *shader ){
	/* dummy check */
	if ( shader == nullptr ) {
		return;
	}

	/* free bits */
	if ( shader->name ) {
		pmm::man.pp_m_delete( shader->name );
	}
	if ( shader->mapName ) {
		pmm::man.pp_m_delete( shader->mapName );
	}

	/* free the shader */
	pmm::man.pp_m_delete( shader );
}



/*
   pmm::pp_find_shader()
   finds a named shader in a model
 */

pmm::shader_t *pmm::pp_find_shader( pmm::model_t *model, char *name, int caseSensitive ){
	int i;


	/* sanity checks */
	if ( model == nullptr || name == nullptr ) { /* sea: null name fix */
		return nullptr;
	}

	/* walk list */
	for ( i = 0; i < model->num_shaders; i++ )
	{
		/* skip null shaders or shaders with null names */
		if ( model->shader[ i ] == nullptr ||
			 model->shader[ i ]->name == nullptr ) {
			continue;
		}

		/* compare the shader name with name we're looking for */
		if ( caseSensitive ) {
			if ( !strcmp( name, model->shader[ i ]->name ) ) {
				return model->shader[ i ];
			}
		}
		else if ( !_pico_stricmp( name, model->shader[ i ]->name ) ) {
			return model->shader[ i ];
		}
	}

	/* named shader not found */
	return nullptr;
}



/* ----------------------------------------------------------------------------
   surfaces
   ---------------------------------------------------------------------------- */

/*
   pmm::pp_new_surface()
   creates a new pico surface
 */

pmm::surface_t *pmm::pp_new_surface( pmm::model_t *model ){
	pmm::surface_t   *surface;
	char surfaceName[64];

	/* allocate and clear */
	surface = reinterpret_cast<decltype(surface)>(pmm::man.pp_m_new( sizeof( *surface ) ));
	if ( surface == nullptr ) {
		return nullptr;
	}
	memset( surface, 0, sizeof( *surface ) );

	/* attach it to the model */
	if ( model != nullptr ) {
		/* adjust model */
		if ( !pmm::pp_adjust_model( model, 0, model->num_surfaces + 1 ) ) {
			pmm::man.pp_m_delete( surface );
			return nullptr;
		}

		/* attach */
		model->surface[ model->num_surfaces - 1 ] = surface;
		surface->model = model;

		/* set default name */
		sprintf( surfaceName, "Unnamed_%d", model->num_surfaces );
		pmm::pp_set_surface_name( surface, surfaceName );
	}

	/* return */
	return surface;
}



/*
   pmm::pp_free_surface()
   frees a surface and all associated data
 */
void pmm::pp_free_surface( pmm::surface_t *surface ){
	int i;


	/* dummy check */
	if ( surface == nullptr ) {
		return;
	}

	/* free bits */
	pmm::man.pp_m_delete( surface->xyz );
	pmm::man.pp_m_delete( surface->normal );
	pmm::man.pp_m_delete( surface->smoothingGroup );
	pmm::man.pp_m_delete( surface->index );
	pmm::man.pp_m_delete( surface->faceNormal );

	if ( surface->name ) {
		pmm::man.pp_m_delete( surface->name );
	}

	/* free arrays */
	for ( i = 0; i < surface->numSTArrays; i++ )
		pmm::man.pp_m_delete( surface->st[ i ] );
	free( surface->st );
	for ( i = 0; i < surface->numColorArrays; i++ )
		pmm::man.pp_m_delete( surface->color[ i ] );
	free( surface->color );

	/* free the surface */
	pmm::man.pp_m_delete( surface );
}



/*
   pmm::pp_adjust_surface()
   adjusts a surface's memory allocations to handle the requested sizes.
   will always grow, never shrink
 */

int pmm::pp_adjust_surface( pmm::surface_t *surface, int numVertexes, int numSTArrays, int numColorArrays, int numIndexes, int numFaceNormals ){
	int i;


	/* dummy check */
	if ( surface == nullptr ) {
		return 0;
	}

	/* bare minimums */
	if ( numVertexes < 1 ) {
		numVertexes = 1;
	}
	if ( numSTArrays < 1 ) {
		numSTArrays = 1;
	}
	if ( numColorArrays < 1 ) {
		numColorArrays = 1;
	}
	if ( numIndexes < 1 ) {
		numIndexes = 1;
	}

	/* additional vertices? */
	while ( numVertexes > surface->maxVertexes ) /* fix */
	{
		surface->maxVertexes += pmm::ee_grow_vertices;
		if ( !pmm::man.pp_m_renew( (void **) &surface->xyz, surface->numVertexes * sizeof( *surface->xyz ), surface->maxVertexes * sizeof( *surface->xyz ) ) ) {
			return 0;
		}
		if ( !pmm::man.pp_m_renew( (void **) &surface->normal, surface->numVertexes * sizeof( *surface->normal ), surface->maxVertexes * sizeof( *surface->normal ) ) ) {
			return 0;
		}
		if ( !pmm::man.pp_m_renew( (void **) &surface->smoothingGroup, surface->numVertexes * sizeof( *surface->smoothingGroup ), surface->maxVertexes * sizeof( *surface->smoothingGroup ) ) ) {
			return 0;
		}
		for ( i = 0; i < surface->numSTArrays; i++ )
			if ( !pmm::man.pp_m_renew( (void **) &surface->st[ i ], surface->numVertexes * sizeof( *surface->st[ i ] ), surface->maxVertexes * sizeof( *surface->st[ i ] ) ) ) {
				return 0;
			}
		for ( i = 0; i < surface->numColorArrays; i++ )
			if ( !pmm::man.pp_m_renew( (void **) &surface->color[ i ], surface->numVertexes * sizeof( *surface->color[ i ] ), surface->maxVertexes * sizeof( *surface->color[ i ] ) ) ) {
				return 0;
			}
	}

	/* set vertex count to higher */
	if ( numVertexes > surface->numVertexes ) {
		surface->numVertexes = numVertexes;
	}

	/* additional st arrays? */
	while ( numSTArrays > surface->maxSTArrays ) /* fix */
	{
		surface->maxSTArrays += pmm::ee_grow_arrays;
		if ( !pmm::man.pp_m_renew( (void **) &surface->st, surface->numSTArrays * sizeof( *surface->st ), surface->maxSTArrays * sizeof( *surface->st ) ) ) {
			return 0;
		}
		while ( surface->numSTArrays < numSTArrays )
		{
			surface->st[surface->numSTArrays] = reinterpret_cast<decltype(&*surface->st[0])>(pmm::man.pp_m_new(surface->maxVertexes * sizeof (*surface->st[0])));
			memset( surface->st[ surface->numSTArrays ], 0, surface->maxVertexes * sizeof( *surface->st[ 0 ] ) );
			surface->numSTArrays++;
		}
	}

	/* additional color arrays? */
	while ( numColorArrays > surface->maxColorArrays ) /* fix */
	{
		surface->maxColorArrays += pmm::ee_grow_arrays;
		if ( !pmm::man.pp_m_renew( (void **) &surface->color, surface->numColorArrays * sizeof( *surface->color ), surface->maxColorArrays * sizeof( *surface->color ) ) ) {
			return 0;
		}
		while ( surface->numColorArrays < numColorArrays )
		{
			surface->color[surface->numColorArrays] = reinterpret_cast<decltype(&*surface->color[0])>(pmm::man.pp_m_new(surface->maxVertexes * sizeof (*surface->color[0])));
			memset( surface->color[ surface->numColorArrays ], 0, surface->maxVertexes * sizeof( *surface->color[ 0 ] ) );
			surface->numColorArrays++;
		}
	}

	/* additional indices? */
	while ( numIndexes > surface->maxIndexes ) /* fix */
	{
		surface->maxIndexes += pmm::ee_grow_indices;
		if ( !pmm::man.pp_m_renew( (void **) &surface->index, surface->numIndexes * sizeof( *surface->index ), surface->maxIndexes * sizeof( *surface->index ) ) ) {
			return 0;
		}
	}

	/* set index count to higher */
	if ( numIndexes > surface->numIndexes ) {
		surface->numIndexes = numIndexes;
	}

	/* additional face normals? */
	while ( numFaceNormals > surface->maxFaceNormals ) /* fix */
	{
		surface->maxFaceNormals += pmm::ee_grow_faces;
		if ( !pmm::man.pp_m_renew( (void **) &surface->faceNormal, surface->numFaceNormals * sizeof( *surface->faceNormal ), surface->maxFaceNormals * sizeof( *surface->faceNormal ) ) ) {
			return 0;
		}
	}

	/* set face normal count to higher */
	if ( numFaceNormals > surface->numFaceNormals ) {
		surface->numFaceNormals = numFaceNormals;
	}

	/* return ok */
	return 1;
}


/* pmm::pp_find_surface:
 *   Finds first matching named surface in a model.
 */
pmm::surface_t *pmm::pp_find_surface(
	pmm::model_t *model, char *name, int caseSensitive ){
	int i;

	/* sanity check */
	if ( model == nullptr || name == nullptr ) {
		return nullptr;
	}

	/* walk list */
	for ( i = 0; i < model->num_surfaces; i++ )
	{
		/* skip null surfaces or surfaces with null names */
		if ( model->surface[ i ] == nullptr ||
			 model->surface[ i ]->name == nullptr ) {
			continue;
		}

		/* compare the surface name with name we're looking for */
		if ( caseSensitive ) {
			if ( !strcmp( name,model->surface[ i ]->name ) ) {
				return model->surface[ i ];
			}
		}
		else {
			if ( !_pico_stricmp( name,model->surface[ i ]->name ) ) {
				return model->surface[ i ];
			}
		}
	}
	/* named surface not found */
	return nullptr;
}



/*----------------------------------------------------------------------------
   PicoSet*() Setter Functions
   ----------------------------------------------------------------------------*/

void pmm::pp_set_model_name( pmm::model_t *model, const char *name ){
	if ( model == nullptr || name == nullptr ) {
		return;
	}
	if ( model->name != nullptr ) {
		pmm::man.pp_m_delete( model->name );
	}

	model->name = _pico_clone_alloc( name );
}



void pmm::pp_set_model_file_name( pmm::model_t *model, const char *fileName ){
	if ( model == nullptr || fileName == nullptr ) {
		return;
	}
	if ( model->fileName != nullptr ) {
		pmm::man.pp_m_delete( model->fileName );
	}

	model->fileName = _pico_clone_alloc( fileName );
}



void pmm::pp_set_model_frame_num( pmm::model_t *model, int frameNum ){
	if ( model == nullptr ) {
		return;
	}
	model->frameNum = frameNum;
}



void pmm::pp_set_model_num_frames( pmm::model_t *model, int numFrames ){
	if ( model == nullptr ) {
		return;
	}
	model->numFrames = numFrames;
}



void pmm::pp_set_model_data( pmm::model_t *model, void *data ){
	if ( model == nullptr ) {
		return;
	}
	model->data = data;
}



void pmm::pp_set_shader_name( pmm::shader_t *shader, char *name ){
	if ( shader == nullptr || name == nullptr ) {
		return;
	}
	if ( shader->name != nullptr ) {
		pmm::man.pp_m_delete( shader->name );
	}

	shader->name = _pico_clone_alloc( name );
}



void pmm::pp_set_shader_map_name( pmm::shader_t *shader, char *mapName ){
	if ( shader == nullptr || mapName == nullptr ) {
		return;
	}
	if ( shader->mapName != nullptr ) {
		pmm::man.pp_m_delete( shader->mapName );
	}

	shader->mapName = _pico_clone_alloc( mapName );
}



void pmm::pp_set_shader_ambient_color( pmm::shader_t *shader, pmm::color_t color ){
	if ( shader == nullptr || color == nullptr ) {
		return;
	}
	shader->ambientColor[ 0 ] = color[ 0 ];
	shader->ambientColor[ 1 ] = color[ 1 ];
	shader->ambientColor[ 2 ] = color[ 2 ];
	shader->ambientColor[ 3 ] = color[ 3 ];
}



void pmm::pp_set_shader_diffuse_color( pmm::shader_t *shader, pmm::color_t color ){
	if ( shader == nullptr || color == nullptr ) {
		return;
	}
	shader->diffuseColor[ 0 ] = color[ 0 ];
	shader->diffuseColor[ 1 ] = color[ 1 ];
	shader->diffuseColor[ 2 ] = color[ 2 ];
	shader->diffuseColor[ 3 ] = color[ 3 ];
}



void pmm::pp_set_shader_specular_color( pmm::shader_t *shader, pmm::color_t color ){
	if ( shader == nullptr || color == nullptr ) {
		return;
	}
	shader->specularColor[ 0 ] = color[ 0 ];
	shader->specularColor[ 1 ] = color[ 1 ];
	shader->specularColor[ 2 ] = color[ 2 ];
	shader->specularColor[ 3 ] = color[ 3 ];
}



void pmm::pp_set_shader_transparency( pmm::shader_t *shader, float value ){
	if ( shader == nullptr ) {
		return;
	}
	shader->transparency = value;

	/* cap to 0..1 range */
	if ( shader->transparency < 0.0 ) {
		shader->transparency = 0.0;
	}
	if ( shader->transparency > 1.0 ) {
		shader->transparency = 1.0;
	}
}



void pmm::pp_set_shader_shininess( pmm::shader_t *shader, float value ){
	if ( shader == nullptr ) {
		return;
	}
	shader->shininess = value;

	/* cap to 0..127 range */
	if ( shader->shininess < 0.0 ) {
		shader->shininess = 0.0;
	}
	if ( shader->shininess > 127.0 ) {
		shader->shininess = 127.0;
	}
}



void pmm::pp_set_surface_data( pmm::surface_t *surface, void *data ){
	if ( surface == nullptr ) {
		return;
	}
	surface->data = data;
}



void pmm::pp_set_surface_type( pmm::surface_t *surface, pmm::surface_type type ){
	if ( surface == nullptr ) {
		return;
	}
	surface->type = type;
}



void pmm::pp_set_surface_name( pmm::surface_t *surface, const char *name ){
	if ( surface == nullptr || name == nullptr ) {
		return;
	}
	if ( surface->name != nullptr ) {
		pmm::man.pp_m_delete( surface->name );
	}

	surface->name = _pico_clone_alloc( name );
}



void pmm::pp_set_surface_shader( pmm::surface_t *surface, pmm::shader_t *shader ){
	if ( surface == nullptr ) {
		return;
	}
	surface->shader = shader;
}



void pmm::pp_set_surface_xyz( pmm::surface_t *surface, int num, pmm::vec3_t xyz ){
	if ( surface == nullptr || num < 0 || xyz == nullptr ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	_pico_copy_vec( xyz, surface->xyz[ num ] );
	if ( surface->model != nullptr ) {
		_pico_expand_bounds( xyz, surface->model->mins, surface->model->maxs );
	}
}



void pmm::pp_set_surface_normal( pmm::surface_t *surface, int num, pmm::vec3_t normal ){
	if ( surface == nullptr || num < 0 || normal == nullptr ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	_pico_copy_vec( normal, surface->normal[ num ] );
}



void pmm::pp_set_surface_st( pmm::surface_t *surface, int array, int num, pmm::vec2_t st ){
	if ( surface == nullptr || num < 0 || st == nullptr ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, num + 1, array + 1, 0, 0, 0 ) ) {
		return;
	}
	surface->st[ array ][ num ][ 0 ] = st[ 0 ];
	surface->st[ array ][ num ][ 1 ] = st[ 1 ];
}



void pmm::pp_set_surface_color( pmm::surface_t *surface, int array, int num, pmm::color_t color ){
	if ( surface == nullptr || num < 0 || color == nullptr ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, num + 1, 0, array + 1, 0, 0 ) ) {
		return;
	}
	surface->color[ array ][ num ][ 0 ] = color[ 0 ];
	surface->color[ array ][ num ][ 1 ] = color[ 1 ];
	surface->color[ array ][ num ][ 2 ] = color[ 2 ];
	surface->color[ array ][ num ][ 3 ] = color[ 3 ];
}



void pmm::pp_set_surface_index( pmm::surface_t *surface, int num, pmm::index_t index ){
	if ( surface == nullptr || num < 0 ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, 0, 0, 0, num + 1, 0 ) ) {
		return;
	}
	surface->index[ num ] = index;
}



void pmm::pp_set_surface_indices( pmm::surface_t *surface, int num, pmm::index_t *index, int count ){
	if ( num < 0 || index == nullptr || count < 1 ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, 0, 0, 0, num + count, 0 ) ) {
		return;
	}
	memcpy( &surface->index[ num ], index, count * sizeof( surface->index[ num ] ) );
}



void pmm::pp_set_face_normal( pmm::surface_t *surface, int num, pmm::vec3_t normal ){
	if ( surface == nullptr || num < 0 || normal == nullptr ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, 0, 0, 0, 0, num + 1 ) ) {
		return;
	}
	_pico_copy_vec( normal, surface->faceNormal[ num ] );
}


void pmm::pp_set_surface_smoothing_group( pmm::surface_t *surface, int num, pmm::index_t smoothingGroup ){
	if ( num < 0 ) {
		return;
	}
	if ( !pmm::pp_adjust_surface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	surface->smoothingGroup[ num ] = smoothingGroup;
}


void pmm::pp_set_surface_special( pmm::surface_t *surface, int num, int special ){
	if ( surface == nullptr || num < 0 || num >= pmm::ee_max_special ) {
		return;
	}
	surface->special[ num ] = special;
}



/*----------------------------------------------------------------------------
   PicoGet*() Getter Functions
   ----------------------------------------------------------------------------*/

char *pmm::pp_get_model_name( pmm::model_t *model ){
	if ( model == nullptr ) {
		return nullptr;
	}
	if ( model->name == nullptr ) {
		return (char*) "";
	}
	return model->name;
}



char *pmm::pp_get_model_file_name( pmm::model_t *model ){
	if ( model == nullptr ) {
		return nullptr;
	}
	if ( model->fileName == nullptr ) {
		return (char*) "";
	}
	return model->fileName;
}



int pmm::pp_get_model_frame_num( pmm::model_t *model ){
	if ( model == nullptr ) {
		return 0;
	}
	return model->frameNum;
}



int pmm::pp_get_model_num_frames( pmm::model_t *model ){
	if ( model == nullptr ) {
		return 0;
	}
	return model->numFrames;
}



void *pmm::pp_get_model_data( pmm::model_t *model ){
	if ( model == nullptr ) {
		return nullptr;
	}
	return model->data;
}



int pmm::pp_get_model_num_shaders( pmm::model_t *model ){
	if ( model == nullptr ) {
		return 0;
	}
	return model->num_shaders;
}



pmm::shader_t *pmm::pp_get_model_shader( pmm::model_t *model, int num ){
	/* a few sanity checks */
	if ( model == nullptr ) {
		return nullptr;
	}
	if ( model->shader == nullptr ) {
		return nullptr;
	}
	if ( num < 0 || num >= model->num_shaders ) {
		return nullptr;
	}

	/* return the shader */
	return model->shader[ num ];
}



int pmm::pp_get_model_num_surfaces( pmm::model_t *model ){
	if ( model == nullptr ) {
		return 0;
	}
	return model->num_surfaces;
}



pmm::surface_t *pmm::pp_get_model_surface( pmm::model_t *model, int num ){
	/* a few sanity checks */
	if ( model == nullptr ) {
		return nullptr;
	}
	if ( model->surface == nullptr ) {
		return nullptr;
	}
	if ( num < 0 || num >= model->num_surfaces ) {
		return nullptr;
	}

	/* return the surface */
	return model->surface[ num ];
}



int pmm::pp_get_model_total_vertices( pmm::model_t *model ){
	int i, count;


	if ( model == nullptr ) {
		return 0;
	}
	if ( model->surface == nullptr ) {
		return 0;
	}

	count = 0;
	for ( i = 0; i < model->num_surfaces; i++ )
		count += pmm::pp_get_surface_num_vertices( model->surface[ i ] );

	return count;
}



int pmm::pp_get_model_total_indices( pmm::model_t *model ){
	int i, count;


	if ( model == nullptr ) {
		return 0;
	}
	if ( model->surface == nullptr ) {
		return 0;
	}

	count = 0;
	for ( i = 0; i < model->num_surfaces; i++ )
		count += pmm::pp_get_surface_num_indices( model->surface[ i ] );

	return count;
}



char *pmm::pp_get_shader_name( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return nullptr;
	}
	if ( shader->name == nullptr ) {
		return (char*) "";
	}
	return shader->name;
}



char *pmm::pp_get_shader_map_name( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return nullptr;
	}
	if ( shader->mapName == nullptr ) {
		return (char*) "";
	}
	return shader->mapName;
}



pmm::ub8_t *pmm::pp_get_shader_ambient_color( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return nullptr;
	}
	return shader->ambientColor;
}



pmm::ub8_t *pmm::pp_get_shader_diffuse_color( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return nullptr;
	}
	return shader->diffuseColor;
}



pmm::ub8_t *pmm::pp_get_shader_specular_color( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return nullptr;
	}
	return shader->specularColor;
}



float pmm::pp_get_shader_transparency( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return 0.0f;
	}
	return shader->transparency;
}



float pmm::pp_get_shader_shininess( pmm::shader_t *shader ){
	if ( shader == nullptr ) {
		return 0.0f;
	}
	return shader->shininess;
}



void *pmm::pp_get_surface_data( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return nullptr;
	}
	return surface->data;
}



pmm::surface_type pmm::pp_get_surface_type( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return pmm::st_bad;
	}
	return surface->type;
}



char *pmm::pp_get_surface_name( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return nullptr;
	}
	if ( surface->name == nullptr ) {
		return (char*) "";
	}
	return surface->name;
}



pmm::shader_t *pmm::pp_get_surface_shader( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return nullptr;
	}
	return surface->shader;
}



int pmm::pp_get_surface_num_vertices( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return 0;
	}
	return surface->numVertexes;
}



pmm::vec_t *pmm::pp_get_surface_xyz( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numVertexes ) {
		return nullptr;
	}
	return surface->xyz[ num ];
}



pmm::vec_t *pmm::pp_get_surface_normal( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numVertexes ) {
		return nullptr;
	}
	return surface->normal[ num ];
}



pmm::vec_t *pmm::pp_get_surface_st( pmm::surface_t *surface, int array, int num  ){
	if ( surface == nullptr || array < 0 || array > surface->numSTArrays || num < 0 || num > surface->numVertexes ) {
		return nullptr;
	}
	return surface->st[ array ][ num ];
}



pmm::ub8_t *pmm::pp_get_surface_color( pmm::surface_t *surface, int array, int num ){
	if ( surface == nullptr || array < 0 || array > surface->numColorArrays || num < 0 || num > surface->numVertexes ) {
		return nullptr;
	}
	return surface->color[ array ][ num ];
}



int pmm::pp_get_surface_num_indices( pmm::surface_t *surface ){
	if ( surface == nullptr ) {
		return 0;
	}
	return surface->numIndexes;
}



pmm::index_t pmm::pp_get_surface_index( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numIndexes ) {
		return 0;
	}
	return surface->index[ num ];
}



pmm::index_t *pmm::pp_get_surface_indices( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numIndexes ) {
		return nullptr;
	}
	return &surface->index[ num ];
}


pmm::vec_t *pmm::pp_get_face_normal( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numFaceNormals ) {
		return nullptr;
	}
	return surface->faceNormal[ num ];
}

pmm::index_t PicoGetSurfaceSmoothingGroup( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num > surface->numVertexes ) {
		return -1;
	}
	return surface->smoothingGroup[ num ];
}


int pmm::pp_get_surface_special( pmm::surface_t *surface, int num ){
	if ( surface == nullptr || num < 0 || num >= pmm::ee_max_special ) {
		return 0;
	}
	return surface->special[ num ];
}



/* ----------------------------------------------------------------------------
   hashtable related functions
   ---------------------------------------------------------------------------- */

/* hashtable code for faster vertex lookups */
//#define HASHTABLE_size 32768 // 2048			/* power of 2, use & */
const int HASHTABLE_size = 7919; // 32749 // 2039    /* prime, use % */

int pmm::pp_get_hash_table_size( void ){
	return HASHTABLE_size;
}

#define HASH_USE_EPSILON

#ifdef HASH_USE_EPSILON
#define HASH_XYZ_EPSILON                    0.01f
#define HASH_XYZ_EPSILONSPACE_MULTIPLIER    1.f / HASH_XYZ_EPSILON
#define HASH_ST_EPSILON                     0.0001f
#define HASH_NORMAL_EPSILON                 0.02f
#endif

unsigned int pmm::pp_vertex_coord_generate_hash( pmm::vec3_t xyz ){
	unsigned int hash = 0;

#ifndef HASH_USE_EPSILON
	hash += ~( *( (unsigned int*) &xyz[ 0 ] ) << 15 );
	hash ^= ( *( (unsigned int*) &xyz[ 0 ] ) >> 10 );
	hash += ( *( (unsigned int*) &xyz[ 1 ] ) << 3 );
	hash ^= ( *( (unsigned int*) &xyz[ 1 ] ) >> 6 );
	hash += ~( *( (unsigned int*) &xyz[ 2 ] ) << 11 );
	hash ^= ( *( (unsigned int*) &xyz[ 2 ] ) >> 16 );
#else
	pmm::vec3_t xyz_epsilonspace;

	_pico_scale_vec( xyz, HASH_XYZ_EPSILONSPACE_MULTIPLIER, xyz_epsilonspace );
	xyz_epsilonspace[ 0 ] = (float)floor( xyz_epsilonspace[ 0 ] );
	xyz_epsilonspace[ 1 ] = (float)floor( xyz_epsilonspace[ 1 ] );
	xyz_epsilonspace[ 2 ] = (float)floor( xyz_epsilonspace[ 2 ] );

	hash += ~( *( (unsigned int*) &xyz_epsilonspace[ 0 ] ) << 15 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 0 ] ) >> 10 );
	hash += ( *( (unsigned int*) &xyz_epsilonspace[ 1 ] ) << 3 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 1 ] ) >> 6 );
	hash += ~( *( (unsigned int*) &xyz_epsilonspace[ 2 ] ) << 11 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 2 ] ) >> 16 );
#endif

	//hash = hash & (HASHTABLE_size-1);
	hash = hash % ( HASHTABLE_size );
	return hash;
}

pmm::pp_vertex_comnination_hash_t **pmm::pp_new_vertex_combination_hash_table( void ){
	pmm::pp_vertex_comnination_hash_t ** hashTable = reinterpret_cast<decltype(hashTable)>(pmm::man.pp_m_new(HASHTABLE_size * sizeof (pmm::pp_vertex_comnination_hash_t*)));

	memset( hashTable, 0, HASHTABLE_size * sizeof( pmm::pp_vertex_comnination_hash_t* ) );

	return hashTable;
}

void pmm::pp_free_vertex_combination_hash_table( pmm::pp_vertex_comnination_hash_t **hashTable ){
	int i;
	pmm::pp_vertex_comnination_hash_t *vertexCombinationHash;
	pmm::pp_vertex_comnination_hash_t *nextVertexCombinationHash;

	/* dummy check */
	if ( hashTable == nullptr ) {
		return;
	}

	for ( i = 0; i < HASHTABLE_size; i++ )
	{
		if ( hashTable[ i ] ) {
			nextVertexCombinationHash = nullptr;

			for ( vertexCombinationHash = hashTable[ i ]; vertexCombinationHash; vertexCombinationHash = nextVertexCombinationHash )
			{
				nextVertexCombinationHash = vertexCombinationHash->next;
				if ( vertexCombinationHash->data != nullptr ) {
					pmm::man.pp_m_delete( vertexCombinationHash->data );
				}
				pmm::man.pp_m_delete( vertexCombinationHash );
			}
		}
	}

	pmm::man.pp_m_delete( hashTable );
}

pmm::pp_vertex_comnination_hash_t *pmm::pp_find_vertex_combination_in_hash_table( pmm::pp_vertex_comnination_hash_t **hashTable, pmm::vec3_t xyz, pmm::vec3_t normal, pmm::vec3_t st, pmm::color_t color ){
	unsigned int hash;
	pmm::pp_vertex_comnination_hash_t *vertexCombinationHash;

	/* dumy check */
	if ( hashTable == nullptr || xyz == nullptr || normal == nullptr || st == nullptr || color == nullptr ) {
		return nullptr;
	}

	hash = pmm::pp_vertex_coord_generate_hash( xyz );

	for ( vertexCombinationHash = hashTable[ hash ]; vertexCombinationHash; vertexCombinationHash = vertexCombinationHash->next )
	{
#ifndef HASH_USE_EPSILON
		/* check xyz */
		if ( ( vertexCombinationHash->vcd.xyz[ 0 ] != xyz[ 0 ] || vertexCombinationHash->vcd.xyz[ 1 ] != xyz[ 1 ] || vertexCombinationHash->vcd.xyz[ 2 ] != xyz[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( ( vertexCombinationHash->vcd.normal[ 0 ] != normal[ 0 ] || vertexCombinationHash->vcd.normal[ 1 ] != normal[ 1 ] || vertexCombinationHash->vcd.normal[ 2 ] != normal[ 2 ] ) ) {
			continue;
		}

		/* check st */
		if ( vertexCombinationHash->vcd.st[ 0 ] != st[ 0 ] || vertexCombinationHash->vcd.st[ 1 ] != st[ 1 ] ) {
			continue;
		}
#else
		/* check xyz */
		if ( ( fabs( xyz[ 0 ] - vertexCombinationHash->vcd.xyz[ 0 ] ) ) > HASH_XYZ_EPSILON ||
			 ( fabs( xyz[ 1 ] - vertexCombinationHash->vcd.xyz[ 1 ] ) ) > HASH_XYZ_EPSILON ||
			 ( fabs( xyz[ 2 ] - vertexCombinationHash->vcd.xyz[ 2 ] ) ) > HASH_XYZ_EPSILON ) {
			continue;
		}

		/* check normal */
		if ( ( fabs( normal[ 0 ] - vertexCombinationHash->vcd.normal[ 0 ] ) ) > HASH_NORMAL_EPSILON ||
			 ( fabs( normal[ 1 ] - vertexCombinationHash->vcd.normal[ 1 ] ) ) > HASH_NORMAL_EPSILON ||
			 ( fabs( normal[ 2 ] - vertexCombinationHash->vcd.normal[ 2 ] ) ) > HASH_NORMAL_EPSILON ) {
			continue;
		}

		/* check st */
		if ( ( fabs( st[ 0 ] - vertexCombinationHash->vcd.st[ 0 ] ) ) > HASH_ST_EPSILON ||
			 ( fabs( st[ 1 ] - vertexCombinationHash->vcd.st[ 1 ] ) ) > HASH_ST_EPSILON ) {
			continue;
		}
#endif

		/* check color */
		if ( *( (int*) vertexCombinationHash->vcd.color ) != *( (int*) color ) ) {
			continue;
		}

		/* gotcha */
		return vertexCombinationHash;
	}

	return nullptr;
}

pmm::pp_vertex_comnination_hash_t *pmm::pp_add_vertex_combination_to_hash_table( pmm::pp_vertex_comnination_hash_t **hashTable, pmm::vec3_t xyz, pmm::vec3_t normal, pmm::vec3_t st, pmm::color_t color, pmm::index_t index ){
	unsigned int hash;
	pmm::pp_vertex_comnination_hash_t *vertexCombinationHash;

	/* dumy check */
	if ( hashTable == nullptr || xyz == nullptr || normal == nullptr || st == nullptr || color == nullptr ) {
		return nullptr;
	}

	vertexCombinationHash = reinterpret_cast<decltype(vertexCombinationHash)>(pmm::man.pp_m_new( sizeof( pmm::pp_vertex_comnination_hash_t ) ));

	if ( !vertexCombinationHash ) {
		return nullptr;
	}

	hash = pmm::pp_vertex_coord_generate_hash( xyz );

	_pico_copy_vec( xyz, vertexCombinationHash->vcd.xyz );
	_pico_copy_vec( normal, vertexCombinationHash->vcd.normal );
	_pico_copy_vec2( st, vertexCombinationHash->vcd.st );
	_pico_copy_color( color, vertexCombinationHash->vcd.color );
	vertexCombinationHash->index = index;
	vertexCombinationHash->data = nullptr;
	vertexCombinationHash->next = hashTable[ hash ];
	hashTable[ hash ] = vertexCombinationHash;

	return vertexCombinationHash;
}

/* ----------------------------------------------------------------------------
   specialized routines
   ---------------------------------------------------------------------------- */

/*
   pmm::pp_find_surfaceVertex()
   finds a vertex matching the set parameters
   fixme: needs non-naive algorithm
 */

int pmm::pp_find_surface_vertex_num( pmm::surface_t *surface, pmm::vec3_t xyz, pmm::vec3_t normal, int numSTs, pmm::vec2_t *st, int numColors, pmm::color_t *color, pmm::index_t smoothingGroup ){
	int i, j;


	/* dummy check */
	if ( surface == nullptr || surface->numVertexes <= 0 ) {
		return -1;
	}

	/* walk vertex list */
	for ( i = 0; i < surface->numVertexes; i++ )
	{
		/* check xyz */
		if ( xyz != nullptr && ( surface->xyz[ i ][ 0 ] != xyz[ 0 ] || surface->xyz[ i ][ 1 ] != xyz[ 1 ] || surface->xyz[ i ][ 2 ] != xyz[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( normal != nullptr && ( surface->normal[ i ][ 0 ] != normal[ 0 ] || surface->normal[ i ][ 1 ] != normal[ 1 ] || surface->normal[ i ][ 2 ] != normal[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( surface->smoothingGroup[ i ] != smoothingGroup ) {
			continue;
		}

		/* check st */
		if ( numSTs > 0 && st != nullptr ) {
			for ( j = 0; j < numSTs; j++ )
			{
				if ( surface->st[ j ][ i ][ 0 ] != st[ j ][ 0 ] || surface->st[ j ][ i ][ 1 ] != st[ j ][ 1 ] ) {
					break;
				}
			}
			if ( j != numSTs ) {
				continue;
			}
		}

		/* check color */
		if ( numColors > 0 && color != nullptr ) {
			for ( j = 0; j < numSTs; j++ )
			{
				if ( *( (int*) surface->color[ j ] ) != *( (int*) color[ j ] ) ) {
					break;
				}
			}
			if ( j != numColors ) {
				continue;
			}
		}

		/* vertex matches */
		return i;
	}

	/* nada */
	return -1;
}




class IndexArray
{
public:
	pmm::index_t* data;
	pmm::index_t* last;
};

void indexarray_push_back( IndexArray* self, pmm::index_t value ){
	*self->last++ = value;
}

pmm::size_type indexarray_size( IndexArray* self ){
	return self->last - self->data;
}

void indexarray_reserve( IndexArray* self, pmm::size_type size ){
	self->last = reinterpret_cast<decltype(self->last)>(pmm::man.pp_k_new(size, sizeof (pmm::index_t)));
	self->data = reinterpret_cast<decltype(self->data)>(self->last);
}

void indexarray_clear( IndexArray* self ){
	pmm::man.pp_m_delete( self->data );
}

class BinaryTreeNode
{
public:
	pmm::index_t left;
	pmm::index_t right;
};

class BinaryTree
{
public:
	BinaryTreeNode* data;
	BinaryTreeNode* last;
};

void binarytree_extend( BinaryTree* self ){
	self->last->left = 0;
	self->last->right = 0;
	++self->last;
}

pmm::size_type binarytree_size( BinaryTree* self ){
	return self->last - self->data;
}

void binarytree_reserve( BinaryTree* self, pmm::size_type size ){
	self->last = reinterpret_cast<decltype(self->last)>(pmm::man.pp_k_new(size, sizeof (BinaryTreeNode)));
	self->data = reinterpret_cast<decltype(self->data)>(self->last);
}

void binarytree_clear( BinaryTree* self ){
	pmm::man.pp_m_delete( self->data );
}

using LessFunc = int(*)(void *, pmm::index_t, pmm::index_t);

class UniqueIndices
{
public:
	BinaryTree tree;
	IndexArray indices;
	LessFunc lessFunc;
	void* lessData;
};

pmm::size_type UniqueIndices_size( UniqueIndices* self ){
	return binarytree_size( &self->tree );
}

void UniqueIndices_reserve( UniqueIndices* self, pmm::size_type size ){
	binarytree_reserve( &self->tree, size );
	indexarray_reserve( &self->indices, size );
}

void UniqueIndices_init( UniqueIndices* self, LessFunc lessFunc, void* lessData ){
	self->lessFunc = lessFunc;
	self->lessData = lessData;
}

void UniqueIndices_destroy( UniqueIndices* self ){
	binarytree_clear( &self->tree );
	indexarray_clear( &self->indices );
}


pmm::index_t UniqueIndices_find_or_insert( UniqueIndices* self, pmm::index_t value ){
	pmm::index_t index = 0;

	for (;; )
	{
		if ( self->lessFunc( self->lessData, value, self->indices.data[index] ) ) {
			BinaryTreeNode* node = self->tree.data + index;
			if ( node->left != 0 ) {
				index = node->left;
				continue;
			}
			else
			{
				node->left = (pmm::index_t)binarytree_size( &self->tree );
				binarytree_extend( &self->tree );
				indexarray_push_back( &self->indices, value );
				return node->left;
			}
		}
		if ( self->lessFunc( self->lessData, self->indices.data[index], value ) ) {
			BinaryTreeNode* node = self->tree.data + index;
			if ( node->right != 0 ) {
				index = node->right;
				continue;
			}
			else
			{
				node->right = (pmm::index_t)binarytree_size( &self->tree );
				binarytree_extend( &self->tree );
				indexarray_push_back( &self->indices, value );
				return node->right;
			}
		}

		return index;
	}
}

pmm::index_t UniqueIndices_insert( UniqueIndices* self, pmm::index_t value ){
	if ( self->tree.data == self->tree.last ) {
		binarytree_extend( &self->tree );
		indexarray_push_back( &self->indices, value );
		return 0;
	}
	else
	{
		return UniqueIndices_find_or_insert( self, value );
	}
}

class picoSmoothVertices_t
{
public:
	pmm::vec3_t* xyz;
	pmm::index_t* smoothingGroups;
};

int lessSmoothVertex( void* data, pmm::index_t first, pmm::index_t second ){
	picoSmoothVertices_t* smoothVertices = reinterpret_cast<decltype(smoothVertices)>(data);

	if ( smoothVertices->xyz[first][0] != smoothVertices->xyz[second][0] ) {
		return smoothVertices->xyz[first][0] < smoothVertices->xyz[second][0];
	}
	if ( smoothVertices->xyz[first][1] != smoothVertices->xyz[second][1] ) {
		return smoothVertices->xyz[first][1] < smoothVertices->xyz[second][1];
	}
	if ( smoothVertices->xyz[first][2] != smoothVertices->xyz[second][2] ) {
		return smoothVertices->xyz[first][2] < smoothVertices->xyz[second][2];
	}
	if ( smoothVertices->smoothingGroups[first] != smoothVertices->smoothingGroups[second] ) {
		return smoothVertices->smoothingGroups[first] < smoothVertices->smoothingGroups[second];
	}
	return 0;
}

void _pico_vertices_combine_shared_normals( pmm::vec3_t* xyz, pmm::index_t* smoothingGroups, pmm::vec3_t* normals, pmm::index_t numVertices ){
	UniqueIndices vertices;
	IndexArray indices;
	picoSmoothVertices_t smoothVertices = { xyz, smoothingGroups };
	UniqueIndices_init( &vertices, lessSmoothVertex, &smoothVertices );
	UniqueIndices_reserve( &vertices, numVertices );
	indexarray_reserve( &indices, numVertices );


	{
		pmm::index_t i = 0;
		for (; i < numVertices; ++i )
		{
			pmm::size_type size = UniqueIndices_size( &vertices );
			pmm::index_t index = UniqueIndices_insert( &vertices, i );
			if ( (pmm::size_type)index != size ) {
				float* normal = normals[vertices.indices.data[index]];
				_pico_add_vec( normal, normals[i], normal );
			}
			indexarray_push_back( &indices, index );
		}
	}

	{
		pmm::index_t maxIndex = 0;
		pmm::index_t* i = indices.data;
		for (; i != indices.last; ++i )
		{
			if ( *i <= maxIndex ) {
				_pico_copy_vec( normals[vertices.indices.data[*i]], normals[i - indices.data] );
			}
			else
			{
				maxIndex = *i;
			}
		}
	}

	UniqueIndices_destroy( &vertices );
	indexarray_clear( &indices );
}

using picoNormalIter_t = pmm::vec3_t *;
using picoIndexIter_t = pmm::index_t *;

#define THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL 1

void _pico_triangles_generate_weighted_normals( picoIndexIter_t first, picoIndexIter_t end, pmm::vec3_t* xyz, pmm::vec3_t* normals ){
	for (; first != end; first += 3 )
	{
#if ( THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL )
		pmm::vec3_t weightedNormal;
		{
			float* a = xyz[*( first + 0 )];
			float* b = xyz[*( first + 1 )];
			float* c = xyz[*( first + 2 )];
			pmm::vec3_t ba, ca;
			_pico_subtract_vec( b, a, ba );
			_pico_subtract_vec( c, a, ca );
			_pico_cross_vec( ca, ba, weightedNormal );
		}
#endif
		{
			int j = 0;
			for (; j < 3; ++j )
			{
				float* normal = normals[*( first + j )];
#if ( !THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL )
				pmm::vec3_t weightedNormal;
				{
					float* a = xyz[*( first + ( ( j + 0 ) % 3 ) )];
					float* b = xyz[*( first + ( ( j + 1 ) % 3 ) )];
					float* c = xyz[*( first + ( ( j + 2 ) % 3 ) )];
					pmm::vec3_t ba, ca;
					_pico_subtract_vec( b, a, ba );
					_pico_subtract_vec( c, a, ca );
					_pico_cross_vec( ca, ba, weightedNormal );
				}
#endif
				_pico_add_vec( weightedNormal, normal, normal );
			}
		}
	}
}

void _pico_normals_zero( picoNormalIter_t first, picoNormalIter_t last ){
	for (; first != last; ++first )
	{
		_pico_zero_vec( *first );
	}
}

void _pico_normals_normalize( picoNormalIter_t first, picoNormalIter_t last ){
	for (; first != last; ++first )
	{
		_pico_normalize_vec( *first );
	}
}

double _pico_length_vec( pmm::vec3_t vec ){
	return sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
}

#define NORMAL_UNIT_LENGTH_EPSILON 0.01
#define FLOAT_EQUAL_EPSILON( f, other, epsilon ) ( fabs( f - other ) < epsilon )

int _pico_normal_is_unit_length( pmm::vec3_t normal ){
	return FLOAT_EQUAL_EPSILON( _pico_length_vec( normal ), 1.0, NORMAL_UNIT_LENGTH_EPSILON );
}

int _pico_normal_within_tolerance( pmm::vec3_t normal, pmm::vec3_t other ){
	return _pico_dot_vec( normal, other ) > 0.0f;
}


void _pico_normals_assign_generated_normals( picoNormalIter_t first, picoNormalIter_t last, picoNormalIter_t generated ){
	for (; first != last; ++first, ++generated )
	{
		if ( !_pico_normal_is_unit_length( *first ) || !_pico_normal_within_tolerance( *first, *generated ) ) {
			_pico_copy_vec( *generated, *first );
		}
	}
}

void pmm::pp_fix_surface_normals( pmm::surface_t* surface ){
	pmm::vec3_t* normals = (pmm::vec3_t*)pmm::man.pp_k_new( surface->numVertexes, sizeof( pmm::vec3_t ) );

	_pico_normals_zero( normals, normals + surface->numVertexes );

	_pico_triangles_generate_weighted_normals( surface->index, surface->index + surface->numIndexes, surface->xyz, normals );
	_pico_vertices_combine_shared_normals( surface->xyz, surface->smoothingGroup, normals, surface->numVertexes );

	_pico_normals_normalize( normals, normals + surface->numVertexes );

	_pico_normals_assign_generated_normals( surface->normal, surface->normal + surface->numVertexes, normals );

	pmm::man.pp_m_delete( normals );
}


/*
   pmm::pp_remap_model() - sea
   remaps model material/etc. information using the remappings
   contained in the given 'remapFile' (full path to the ascii file to open)
   returns 1 on success or 0 on error
 */

#define _prm_error_return \
	{ \
		_pico_free_parser( p );	\
		pmm::man.pp_f_delete( remapBuffer );	\
		return 0; \
	}

int pmm::pp_remap_model( pmm::model_t *model, char *remapFile ){
	// sanity checks
	if ( model == nullptr || remapFile == nullptr )
	{
		return 0;
	}

	picoParser_t    *p;
	pmm::ub8_t      *remapBuffer;

	// load remap file contents
	int remapBufSize = pmm::man.pp_load_file(remapFile, &remapBuffer);

	/* check result */
	if ( remapBufSize == 0 ) {
		return 1;   /* file is empty: no error */
	}
	if ( remapBufSize < 0 ) {
		return 0;   /* load failed: error */

	}
	/* create a new pico parser */
	p = _pico_new_parser( remapBuffer, remapBufSize );
	if ( p == nullptr ) {
		/* ram is really cheap nowadays... */
		_prm_error_return;
	}

	/* doo teh parse */
	while ( 1 )
	{
		/* get next token in remap file */
		if ( !_pico_parse( p,1 ) ) {
			break;
		}

		/* skip over c++ style comment lines */
		if ( !_pico_stricmp( p->token,"//" ) ) {
			_pico_parse_skip_rest( p );
			continue;
		}

		/* block for quick material shader name remapping */
		/* materials { "m" (=>|->|=) "s" } */
		if ( !_pico_stricmp( p->token, "materials" ) ) {
			int level = 1;

			/* check bracket */
			if ( !_pico_parse_check( p,1,"{" ) ) {
				_prm_error_return;
			}

			/* process assignments */
			while ( 1 )
			{
				pmm::shader_t    *shader;
				char            *materialName;


				/* get material name */
				if ( _pico_parse( p,1 ) == nullptr ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}
				materialName = _pico_clone_alloc( p->token );
				if ( materialName == nullptr ) {
					_prm_error_return;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				/* get next token (assignment token or shader name) */
				if ( !_pico_parse( p,0 ) ) {
					pmm::man.pp_m_delete( materialName );
					_prm_error_return;
				}
				/* skip assignment token (if present) */
				if ( !strcmp( p->token,"=>" ) ||
					 !strcmp( p->token,"->" ) ||
					 !strcmp( p->token,"=" ) ) {
					/* simply grab the next token */
					if ( !_pico_parse( p,0 ) ) {
						pmm::man.pp_m_delete( materialName );
						_prm_error_return;
					}
				}
				/* try to find material by name */
				shader = pmm::pp_find_shader( model,materialName,0 );

				/* we've found a material matching the name */
				if ( shader != nullptr ) {
					pmm::pp_set_shader_name( shader,p->token );
				}
				/* free memory used by material name */
				pmm::man.pp_m_delete( materialName );

				/* skip rest */
				_pico_parse_skip_rest( p );
			}
		}
		/* block for detailed single material remappings */
		/* materials[ "m" ] { key data... } */
		else if ( !_pico_stricmp( p->token,"materials[" ) ) {
			pmm::shader_t *shader;
			char *tempMaterialName;
			int level = 1;

			/* get material name */
			if ( !_pico_parse( p,0 ) ) {
				_prm_error_return;
			}

			/* temporary copy of material name */
			tempMaterialName = _pico_clone_alloc( p->token );
			if ( tempMaterialName == nullptr ) {
				_prm_error_return;
			}

			/* check square closing bracket */
			if ( !_pico_parse_check( p,0,"]" ) ) {
				_prm_error_return;
			}

			/* try to find material by name */
			shader = pmm::pp_find_shader( model,tempMaterialName,0 );

			/* free memory used by temporary material name */
			pmm::man.pp_m_delete( tempMaterialName );

			/* we haven't found a material matching the name */
			/* so we simply skip the braced section now and */
			/* continue parsing with the next main token */
			if ( shader == nullptr ) {
				_pico_parse_skip_braced( p );
				continue;
			}
			/* check opening bracket */
			if ( !_pico_parse_check( p,1,"{" ) ) {
				_prm_error_return;
			}

			/* process material info keys */
			while ( 1 )
			{
				/* get key name */
				if ( _pico_parse( p,1 ) == nullptr ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				/* remap shader name */
				if ( !_pico_stricmp( p->token,"shader" ) ) {
					if ( !_pico_parse( p,0 ) ) {
						_prm_error_return;
					}
					pmm::pp_set_shader_name( shader,p->token );
				}
				/* remap shader map name */
				else if ( !_pico_stricmp( p->token,"mapname" ) ) {
					if ( !_pico_parse( p,0 ) ) {
						_prm_error_return;
					}
					pmm::pp_set_shader_map_name( shader,p->token );
				}
				/* remap shader's ambient color */
				else if ( !_pico_stricmp( p->token,"ambient" ) ) {
					pmm::color_t color;
					pmm::vec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p,v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (pmm::ub8_t)v[ 0 ];
					color[ 1 ] = (pmm::ub8_t)v[ 1 ];
					color[ 2 ] = (pmm::ub8_t)v[ 2 ];

					/* set new ambient color */
					pmm::pp_set_shader_ambient_color( shader,color );
				}
				/* remap shader's diffuse color */
				else if ( !_pico_stricmp( p->token,"diffuse" ) ) {
					pmm::color_t color;
					pmm::vec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p,v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (pmm::ub8_t)v[ 0 ];
					color[ 1 ] = (pmm::ub8_t)v[ 1 ];
					color[ 2 ] = (pmm::ub8_t)v[ 2 ];

					/* set new ambient color */
					pmm::pp_set_shader_diffuse_color( shader,color );
				}
				/* remap shader's specular color */
				else if ( !_pico_stricmp( p->token,"specular" ) ) {
					pmm::color_t color;
					pmm::vec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p,v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (pmm::ub8_t)v[ 0 ];
					color[ 1 ] = (pmm::ub8_t)v[ 1 ];
					color[ 2 ] = (pmm::ub8_t)v[ 2 ];

					/* set new ambient color */
					pmm::pp_set_shader_specular_color( shader,color );
				}
				/* skip rest */
				_pico_parse_skip_rest( p );
			}
		}
		/* end 'materials[' */
	}

	/* free both parser and file buffer */
	_pico_free_parser( p );
	pmm::man.pp_f_delete(remapBuffer);

	/* return with success */
	return 1;
}


/*
   pmm::pp_add_triangle_to_model() - jhefty
   A nice way to add individual triangles to the model.
   Chooses an appropriate surface based on the shader, or adds a new surface if necessary
 */

void pmm::pp_add_triangle_to_model( pmm::model_t *model, pmm::vec3_t** xyz, pmm::vec3_t** normals,
							 int numSTs, pmm::vec2_t **st, int numColors, pmm::color_t **colors,
							 pmm::shader_t* shader, const char *name, pmm::index_t* smoothingGroup ){
	int i,j;
	int vertDataIndex;
	pmm::surface_t* workSurface = nullptr;

	/* see if a surface already has the shader */
	for ( i = 0 ; i < model->num_surfaces ; i++ )
	{
		workSurface = model->surface[i];
		if ( !name || !strcmp( workSurface->name, name ) ) {
			if ( workSurface->shader == shader ) {
				break;
			}
		}
	}

	/* no surface uses this shader yet, so create a new surface */
	if ( !workSurface || i >= model->num_surfaces ) {
		/* create a new surface in the model for the unique shader */
		workSurface = pmm::pp_new_surface( model );
		if ( !workSurface ) {
			pmm::man.pp_print(pmm::pl_error, "Could not allocate a new surface!\n");
			return;
		}

		/* do surface setup */
		pmm::pp_set_surface_type( workSurface, pmm::st_triangles );
		pmm::pp_set_surface_name( workSurface, name ? name : shader->name );
		pmm::pp_set_surface_shader( workSurface, shader );
	}

	/* add the triangle data to the surface */
	for ( i = 0 ; i < 3 ; i++ )
	{
		/* get the next free spot in the index array */
		int newVertIndex = pmm::pp_get_surface_num_indices( workSurface );

		/* get the index of the vertex that we're going to store at newVertIndex */
		vertDataIndex = pmm::pp_find_surface_vertex_num( workSurface, *xyz[i], *normals[i], numSTs, st[i], numColors, colors[i], smoothingGroup[i] );

		/* the vertex wasn't found, so create a new vertex in the pool from the data we have */
		if ( vertDataIndex == -1 ) {
			/* find the next spot for a new vertex */
			vertDataIndex = pmm::pp_get_surface_num_vertices( workSurface );

			/* assign the data to it */
			pmm::pp_set_surface_xyz( workSurface,vertDataIndex, *xyz[i] );
			pmm::pp_set_surface_normal( workSurface, vertDataIndex, *normals[i] );

			/* make sure to copy over all available ST's and colors for the vertex */
			for ( j = 0 ; j < numColors ; j++ )
			{
				pmm::pp_set_surface_color( workSurface, j, vertDataIndex, colors[i][j] );
			}
			for ( j = 0 ; j < numSTs ; j++ )
			{
				pmm::pp_set_surface_st( workSurface, j, vertDataIndex, st[i][j] );
			}

			pmm::pp_set_surface_smoothing_group( workSurface, vertDataIndex, smoothingGroup[i] );
		}

		/* add this vertex to the triangle */
		pmm::pp_set_surface_index( workSurface, newVertIndex, vertDataIndex );
	}
}
