/*
 * Event values functions
 *
 * Copyright (c) 2011-2012, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include <liberror.h>
#include <libnotify.h>

#include "libevtx_binary_xml_token.h"
#include "libevtx_event_values.h"
#include "libevtx_io_handle.h"

#include "evtx_event_record.h"

const uint8_t *evtx_event_record_signature = (uint8_t *) "\x2a\x2a\x00\x00";

/* Initialize event values
 * Make sure the value event values is pointing to is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libevtx_event_values_initialize(
     libevtx_event_values_t **event_values,
     liberror_error_t **error )
{
	static char *function = "libevtx_event_values_initialize";

	if( event_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid event values.",
		 function );

		return( -1 );
	}
	if( *event_values != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid event values value already set.",
		 function );

		return( -1 );
	}
	*event_values = memory_allocate_structure(
	                 libevtx_event_values_t );

	if( *event_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create event values.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     ( *event_values ),
	     0,
	     sizeof( libevtx_event_values_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear event values.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *event_values != NULL )
	{
		memory_free(
		 *event_values );

		*event_values = NULL;
	}
	return( -1 );
}

/* Frees event values
 * Returns 1 if successful or -1 on error
 */
int libevtx_event_values_free(
     libevtx_event_values_t **event_values,
     liberror_error_t **error )
{
	static char *function = "libevtx_event_values_free";
	int result            = 1;

	if( event_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid event values.",
		 function );

		return( -1 );
	}
	if( *event_values != NULL )
	{
		memory_free(
		 *event_values );

		*event_values = NULL;
	}
	return( result );
}

/* Reads the event values
 * Returns 1 if successful or -1 on error
 */
int libevtx_event_values_read(
     libevtx_event_values_t *event_values,
     libevtx_io_handle_t *io_handle,
     const uint8_t *chunk_data,
     size_t chunk_data_size,
     size_t chunk_data_offset,
     liberror_error_t **error )
{
	const uint8_t *event_record_data = NULL;
	static char *function            = "libevtx_event_values_read";
	size_t event_record_data_size    = 0;
	uint32_t copy_of_size            = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint64_t value_64bit             = 0;
	uint32_t value_32bit             = 0;
	uint16_t value_16bit             = 0;
#endif

	if( event_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid event values.",
		 function );

		return( -1 );
	}
	if( io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( chunk_data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk data.",
		 function );

		return( -1 );
	}
	if( chunk_data_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid event record data size value exceeds maximum.",
		 function );

		goto on_error;
	}
	if( chunk_data_offset >= chunk_data_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid chunk data offset value out of bounds.",
		 function );

		goto on_error;
	}
	event_record_data      = &( chunk_data[ chunk_data_offset ] );
	event_record_data_size = chunk_data_size - chunk_data_offset;

	if( event_record_data_size < ( sizeof( evtx_event_record_header_t ) + 4 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid event record data size value too small.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: event record header data:\n",
		 function );
		libnotify_print_data(
		 event_record_data,
		 sizeof( evtx_event_record_header_t ),
		 0 );
	}
#endif
	if( memory_compare(
	     ( (evtx_event_record_header_t *) event_record_data )->signature,
	     evtx_event_record_signature,
	     4 ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported event record signature.",
		 function );

		goto on_error;
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( (evtx_event_record_header_t *) event_record_data )->size,
	 event_values->size );

	byte_stream_copy_to_uint64_little_endian(
	 ( (evtx_event_record_header_t *) event_record_data )->identifier,
	 event_values->identifier );

	byte_stream_copy_to_uint64_little_endian(
	 ( (evtx_event_record_header_t *) event_record_data )->creation_time,
	 event_values->creation_time );

	byte_stream_copy_to_uint32_little_endian(
	 &( event_record_data[ event_values->size - 4 ] ),
	 copy_of_size );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: signature\t\t\t\t\t: \\x%02x\\x%02x\\x%02x\\x%02x\n",
		 function,
		 ( (evtx_event_record_header_t *) event_record_data )->signature[ 0 ],
		 ( (evtx_event_record_header_t *) event_record_data )->signature[ 1 ],
		 ( (evtx_event_record_header_t *) event_record_data )->signature[ 2 ] ,
		 ( (evtx_event_record_header_t *) event_record_data )->signature[ 3 ] );

		libnotify_printf(
		 "%s: size\t\t\t\t\t\t: %" PRIu32 "\n",
		 function,
		 event_values->size );

		libnotify_printf(
		 "%s: identifier\t\t\t\t\t: %" PRIu64 "\n",
		 function,
		 event_values->identifier );

		libnotify_printf(
		 "%s: creation time\t\t\t\t: 0x%08" PRIx64 "\n",
		 function,
		 event_values->creation_time );

		libnotify_printf(
		 "%s: copy of size\t\t\t\t\t: %" PRIu32 "\n",
		 function,
		 copy_of_size );

		libnotify_printf(
		 "\n" );
	}
#endif
	if( ( event_values->size < sizeof( evtx_event_record_header_t ) )
	 || ( event_values->size > ( event_record_data_size - 4 ) ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid event record data size value out of bounds.",
		 function );

		goto on_error;
	}
/* TODO validate size with copy */
	chunk_data_offset     += sizeof( evtx_event_record_header_t );
	event_record_data     += sizeof( evtx_event_record_header_t );
	event_record_data_size = event_values->size
	                       - ( sizeof( evtx_event_record_header_t ) + 4 );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: event record data:\n",
		 function );
		libnotify_print_data(
		 event_record_data,
		 event_record_data_size,
		 0 );
	}
#endif
/* TODO */
	libevtx_binary_xml_token_t *binary_xml_token = NULL;

	while( chunk_data_offset < chunk_data_size )
	{
		if( libevtx_binary_xml_token_initialize(
		     &binary_xml_token,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create binary XML token.",
			 function );

			goto on_error;
		}
		if( libevtx_binary_xml_token_read(
		     binary_xml_token,
		     io_handle,
		     chunk_data,
		     chunk_data_size,
		     chunk_data_offset,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read binary XML token.",
			 function );

			goto on_error;
		}
		chunk_data_offset += binary_xml_token->size;

		if( libevtx_binary_xml_token_free(
		     &binary_xml_token,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free binary XML token.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	return( -1 );
}

