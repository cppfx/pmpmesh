/*
   ======================================================================
   lwo2.c

   The entry point for loading LightWave object files.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include <pmpmesh/pm_internal.hpp>
#include <pmpmesh/lwo/lwo2.hpp>

/* disable warnings */
#if GDEF_COMPILER_MSVC
#pragma warning( disable:4018 )		/* signed/unsigned mismatch */
#endif


/*
   ======================================================================
   lwFreeLayer()

   Free memory used by an lwLayer.
   ====================================================================== */

void lwFreeLayer( lwLayer *layer ){
	if ( layer ) {
		if ( layer->name ) {
			pmm::man.pp_m_delete( layer->name );
		}
		lwFreePoints( &layer->point );
		lwFreePolygons( &layer->polygon );
		lwListFree(layer->vmap, (void (*)(void *)) lwFreeVMap);
		pmm::man.pp_m_delete( layer );
	}
}


/*
   ======================================================================
   lwFreeObject()

   Free memory used by an lwObject.
   ====================================================================== */

void lwFreeObject( lwObject *object ){
	if ( object ) {
		lwListFree(object->layer, (void (*)(void *)) lwFreeLayer);
		lwListFree(object->env, (void (*)(void *)) lwFreeEnvelope);
		lwListFree(object->clip, (void (*)(void *)) lwFreeClip);
		lwListFree(object->surf, (void (*)(void *)) lwFreeSurface);
		lwFreeTags( &object->taglist );
		pmm::man.pp_m_delete( object );
	}
}


/*
   ======================================================================
   lwGetObject()

   Returns the contents of a LightWave object, given its filename, or
   nullptr if the file couldn't be loaded.  On failure, failID and failpos
   can be used to diagnose the cause.

   1.  If the file isn't an LWO2 or an LWOB, failpos will contain 12 and
    failID will be unchanged.

   2.  If an error occurs while reading, failID will contain the most
    recently read IFF chunk ID, and failpos will contain the value
    returned by _pico_memstream_tell() at the time of the failure.

   3.  If the file couldn't be opened, or an error occurs while reading
    the first 12 bytes, both failID and failpos will be unchanged.

   If you don't need this information, failID and failpos can be nullptr.
   ====================================================================== */

lwObject *lwGetObject( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos ){
	lwObject *object;
	lwLayer *layer;
	lwNode *node;
	unsigned int id, formsize, type;
	int i, rlen, cksize;

	/* open the file */

	if ( !fp ) {
		return nullptr;
	}

	/* read the first 12 bytes */

	set_flen( 0 );
	id       = getU4( fp );
	formsize = getU4( fp );
	type     = getU4( fp );
	if ( 12 != get_flen() ) {
		return nullptr;
	}

	/* is this a LW object? */

	if ( id != ID_FORM ) {
		if ( failpos ) {
			*failpos = 12;
		}
		return nullptr;
	}

	if ( type != ID_LWO2 ) {
		if ( type == ID_LWOB ) {
			return lwGetObject5( filename, fp, failID, failpos );
		}
		else {
			if ( failpos ) {
				*failpos = 12;
			}
			return nullptr;
		}
	}

	/* allocate an object and a default layer */

	object = reinterpret_cast<decltype(object)>(pmm::man.pp_k_new(1, sizeof(lwObject)));
	if ( !object ) {
		goto Fail;
	}

	layer = reinterpret_cast<decltype(layer)>(pmm::man.pp_k_new( 1, sizeof(lwLayer)));
	if ( !layer ) {
		goto Fail;
	}
	object->layer = layer;

	/* get the first chunk header */

	id = getU4( fp );
	cksize = getU4( fp );
	if ( 0 > get_flen() ) {
		goto Fail;
	}

	/* process chunks as they're encountered */

	while ( 1 ) {
		cksize += cksize & 1;

		switch ( id )
		{
		case ID_LAYR:
			if ( object->nlayers > 0 ) {
				layer = reinterpret_cast<decltype(layer)>(pmm::man.pp_k_new(1, sizeof(lwLayer)));
				if ( !layer ) {
					goto Fail;
				}
				lwListAdd( (void **) &object->layer, layer );
			}
			object->nlayers++;

			set_flen( 0 );
			layer->index = getU2( fp );
			layer->flags = getU2( fp );
			layer->pivot[ 0 ] = getF4( fp );
			layer->pivot[ 1 ] = getF4( fp );
			layer->pivot[ 2 ] = getF4( fp );
			layer->name = getS0( fp );

			rlen = get_flen();
			if ( rlen < 0 || rlen > cksize ) {
				goto Fail;
			}
			if ( rlen <= cksize - 2 ) {
				layer->parent = getU2( fp );
			}
			rlen = get_flen();
			if ( rlen < cksize ) {
				_pico_memstream_seek( fp, cksize - rlen, PICO_SEEK_CUR );
			}
			break;

		case ID_PNTS:
			if ( !lwGetPoints( fp, cksize, &layer->point ) ) {
				goto Fail;
			}
			break;

		case ID_POLS:
			if ( !lwGetPolygons( fp, cksize, &layer->polygon,
								 layer->point.offset ) ) {
				goto Fail;
			}
			break;

		case ID_VMAP:
		case ID_VMAD:
			node = ( lwNode * ) lwGetVMap( fp, cksize, layer->point.offset,
										   layer->polygon.offset, id == ID_VMAD );
			if ( !node ) {
				goto Fail;
			}
			lwListAdd( (void **) &layer->vmap, node );
			layer->nvmaps++;
			break;

		case ID_PTAG:
			if ( !lwGetPolygonTags( fp, cksize, &object->taglist,
									&layer->polygon ) ) {
				goto Fail;
			}
			break;

		case ID_BBOX:
			set_flen( 0 );
			for ( i = 0; i < 6; i++ )
				layer->bbox[ i ] = getF4( fp );
			rlen = get_flen();
			if ( rlen < 0 || rlen > cksize ) {
				goto Fail;
			}
			if ( rlen < cksize ) {
				_pico_memstream_seek( fp, cksize - rlen, PICO_SEEK_CUR );
			}
			break;

		case ID_TAGS:
			if ( !lwGetTags( fp, cksize, &object->taglist ) ) {
				goto Fail;
			}
			break;

		case ID_ENVL:
			node = ( lwNode * ) lwGetEnvelope( fp, cksize );
			if ( !node ) {
				goto Fail;
			}
			lwListAdd( (void **) &object->env, node );
			object->nenvs++;
			break;

		case ID_CLIP:
			node = ( lwNode * ) lwGetClip( fp, cksize );
			if ( !node ) {
				goto Fail;
			}
			lwListAdd( (void **) &object->clip, node );
			object->nclips++;
			break;

		case ID_SURF:
			node = ( lwNode * ) lwGetSurface( fp, cksize );
			if ( !node ) {
				goto Fail;
			}
			lwListAdd( (void **) &object->surf, node );
			object->nsurfs++;
			break;

		case ID_DESC:
		case ID_TEXT:
		case ID_ICON:
		default:
			_pico_memstream_seek( fp, cksize, PICO_SEEK_CUR );
			break;
		}

		/* end of the file? */

		if ( formsize <= (unsigned int) ( _pico_memstream_tell( fp ) - 8 ) ) {
			break;
		}

		/* get the next chunk header */

		set_flen( 0 );
		id = getU4( fp );
		cksize = getU4( fp );
		if ( 8 != get_flen() ) {
			goto Fail;
		}
	}

	if ( object->nlayers == 0 ) {
		object->nlayers = 1;
	}

	layer = object->layer;
	while ( layer ) {
		lwGetBoundingBox( &layer->point, layer->bbox );
		lwGetPolyNormals( &layer->point, &layer->polygon );
		if ( !lwGetPointPolygons( &layer->point, &layer->polygon ) ) {
			goto Fail;
		}
		if ( !lwResolvePolySurfaces( &layer->polygon, &object->taglist,
									 &object->surf, &object->nsurfs ) ) {
			goto Fail;
		}
		lwGetVertNormals( &layer->point, &layer->polygon );
		if ( !lwGetPointVMaps( &layer->point, layer->vmap ) ) {
			goto Fail;
		}
		if ( !lwGetPolyVMaps( &layer->polygon, layer->vmap ) ) {
			goto Fail;
		}
		layer = layer->next;
	}

	return object;

Fail:
	if ( failID ) {
		*failID = id;
	}
	if ( fp ) {
		if ( failpos ) {
			*failpos = _pico_memstream_tell( fp );
		}
	}
	lwFreeObject( object );
	return nullptr;
}

int lwValidateObject( const char *filename, picoMemStream_t *fp, unsigned int *failID, int *failpos ){
	unsigned int id, type;

	/* open the file */

	if ( !fp ) {
		return pmm::pmv_error_memory;
	}

	/* read the first 12 bytes */

	set_flen( 0 );
	id       = getU4( fp );
	/* formsize = */ getU4( fp );
	type     = getU4( fp );
	if ( 12 != get_flen() ) {
		return pmm::pmv_error_size;
	}

	/* is this a LW object? */

	if ( id != ID_FORM ) {
		if ( failpos ) {
			*failpos = 12;
		}
		return pmm::pmv_error_size;
	}

	if ( type != ID_LWO2 ) {
		if ( type == ID_LWOB ) {
			return lwValidateObject5( filename, fp, failID, failpos );
		}
		else {
			if ( failpos ) {
				*failpos = 12;
			}
			return pmm::pmv_error_ident;
		}
	}

	return pmm::pmv_ok;
}
