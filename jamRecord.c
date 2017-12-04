/*
  Copyright 2017 Tristan Halna du Fretay <tdufret@yahoo.fr>
  Copyright 2006-2016 David Robillard <d@drobilla.net>
  Copyright 2006 Steve Harris <steve@plugin.org.uk>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/** Include standard C headers */
#include <math.h>
#include <stdlib.h>

/**
   LV2 headers are based on the URI of the specification they come from, so a
   consistent convention can be used even for unofficial extensions.  The URI
   of the core LV2 specification is <http://lv2plug.in/ns/lv2core>, by
   replacing `http:/` with `lv2` any header in the specification bundle can be
   included, in this case `lv2.h`.
*/
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

/**
   The URI is the identifier for a plugin, and how the host associates this
   implementation in code with its description in data.  In this plugin it is
   only used once in the code, but defining the plugin URI at the top of the
   file is a good convention to follow.  If this URI does not match that used
   in the data files, the host will fail to load the plugin.
*/
#define JAMRECORD_ST_URI "https://tdufret.fr/plugins/jamrecord#stereo_record"

/**
   In code, ports are referred to by index.  An enumeration of port indices
   should be defined for readability.
*/
typedef enum {
  FORMAT_PI   = 0,
  RECORD_PI   = 1,
  SAVE_PI     = 2,
  CLIP_PI     = 3,
  INPUT_L_PI  = 4,
  OUTPUT_L_PI = 5,
  INPUT_R_PI  = 6,
  OUTPUT_R_PI = 7
} PortIndex;

/* maximum recoding duration supported by the plugin (in seconds) */
/* used to determine the data ring buffer size                    */
#define MAX_RECORDING_DURATION 60      /* test value */

/**
   Every plugin defines a private structure for the plugin instance.  All data
   associated with a plugin instance is stored here, and is available to
   every instance method.  In this simple plugin, only port buffers need to be
   stored, since there is no additional instance data.
*/
typedef struct {
  // Port buffers
  const int*   format;
  const int*   record;
  const int*   save;
  int*         clip;
  const float* input_l;
  const float* input_r;
  float*       output_l;
  float*       output_r;
  double       sample_rate;     /* to store sample rate value provided by host */
  int          record_duration; /* recording duration - to be set by user */
  float*       data_buffer_l;   /* pointer to the left channel data ring buffer */
  float*       data_buffer_r;   /* pointer to the right channel data ring buffer */
  unsigned long write_ptr;       /* buffer position of the next data to be wrote */
  unsigned long read_ptr;        /* buffer position of the next data to be red   */
} JamRecord_t;

/**
   The `instantiate()` function is called by the host to create a new plugin
   instance.  The host passes the plugin descriptor, sample rate, and bundle
   path for plugins that need to load additional resources (e.g. waveforms).
   The features parameter contains host-provided features defined in LV2
   extensions, but this simple plugin does not use any.

   This function is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static LV2_Handle
instantiate(const LV2_Descriptor*    descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
  JamRecord_t* jamRecord = (JamRecord_t*)calloc(1, sizeof(JamRecord_t));

  /* store rate value */
  jamRecord->sample_rate = rate;
  
  return (LV2_Handle)jamRecord;
}

/**
   The `connect_port()` method is called by the host to connect a particular
   port to a buffer.  The plugin must store the data location, but data may not
   be accessed except in run().

   This method is in the ``audio'' threading class, and is called in the same
   context as run().
*/
static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
  JamRecord_t* jamRecord = (JamRecord_t*)instance;
  
  switch ((PortIndex)port) {
  case FORMAT_PI:
    jamRecord->format = (const int*)data;
    break;
  case RECORD_PI:
    jamRecord->record = (const int*)data;
    break;
  case SAVE_PI:
    jamRecord->save = (const int*)data;
    break;
  case CLIP_PI:
    jamRecord->clip = (int*)data;
    break;
  case INPUT_L_PI:
    jamRecord->input_l = (const float*)data;
    break;
  case INPUT_R_PI:
    jamRecord->input_r = (const float*)data;
    break;
  case OUTPUT_L_PI:
    jamRecord->output_l = (float*)data;
    break;
  case OUTPUT_R_PI:
    jamRecord->output_r = (float*)data;
    break;
  }
}

/**
   The `activate()` method is called by the host to initialise and prepare the
   plugin instance for running.  The plugin must reset all internal state
   except for buffer locations set by `connect_port()`.  Since this plugin has
   no other internal state, this method does nothing.

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
activate(LV2_Handle instance)
{
  JamRecord_t* jamRecord = (JamRecord_t*)instance;
  
  jamRecord->record_duration = 50;      /* for tests - to be set from interface in futur */
  jamRecord->write_ptr = 0; 
  jamRecord->read_ptr = 0;


  /* allocate a ring buffer to store audio data */
  /* to be moved into instantiate() ? */
  jamRecord->data_buffer_l = (float*) malloc(jamRecord->sample_rate * MAX_RECORDING_DURATION * sizeof(float));
  jamRecord->data_buffer_r = (float*) malloc(jamRecord->sample_rate * MAX_RECORDING_DURATION * sizeof(float));
 
}

/**
   The `run()` method is the main process function of the plugin.  It processes
   a block of audio in the audio context.  Since this plugin is
   `lv2:hardRTCapable`, `run()` must be real-time safe, so blocking (e.g. with
   a mutex) or memory allocation are not allowed.
*/
static void
run(LV2_Handle instance, uint32_t n_samples)
{
  /*
  const JamRecord_t* jamRecord = (const JamRecord_t*)instance;
  */
  JamRecord_t* jamRecord = (JamRecord_t*)instance;
  
  const float        record   = *(jamRecord->record);
  const float* const input_l  = jamRecord->input_l;
  const float* const input_r  = jamRecord->input_r;
  float* const       output_l = jamRecord->output_l;
  float* const       output_r = jamRecord->output_r;
  
  for (uint32_t pos = 0; pos < n_samples; pos++)
    {
      /* forward left and right samples from inputs to outputs */
      output_l[pos] = input_l[pos];
      output_r[pos] = input_r[pos];

      if (record == 1)
	{
	  /* store left and right samples into data_buffer */
	  jamRecord->data_buffer_l[jamRecord->write_ptr] = input_l[pos];
	  jamRecord->data_buffer_r[jamRecord->write_ptr] = input_r[pos];
	  /* update write pointer */
	  jamRecord->write_ptr++;
	  if (jamRecord->write_ptr >= (jamRecord->sample_rate * MAX_RECORDING_DURATION))
	    {
	      /* write pointer reached the end of the ring buffer */
	      jamRecord->write_ptr = 0;
	    }
	  /* if right pointer reached read pointer, update read pointer */
	  if (jamRecord->write_ptr == jamRecord->read_ptr)
	    {
	      jamRecord->read_ptr++;
	      if (jamRecord->read_ptr >= (jamRecord->sample_rate * MAX_RECORDING_DURATION))
		{
		  /* read pointer reached the end of the ring buffer */
		  jamRecord->read_ptr = 0;
		}
	      
	    }
	}
      
    }
}

/**
   The `deactivate()` method is the counterpart to `activate()`, and is called by
   the host after running the plugin.  It indicates that the host will not call
   `run()` again until another call to `activate()` and is mainly useful for more
   advanced plugins with ``live'' characteristics such as those with auxiliary
   processing threads.  As with `activate()`, this plugin has no use for this
   information so this method does nothing.

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
deactivate(LV2_Handle instance)
{
}

/**
   Destroy a plugin instance (counterpart to `instantiate()`).

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
cleanup(LV2_Handle instance)
{
  JamRecord_t* jamRecord = (JamRecord_t*)instance;

  free(jamRecord->data_buffer_l);
  free(jamRecord->data_buffer_r);
  free(instance);
}

/**
   The `extension_data()` function returns any extension data supported by the
   plugin.  Note that this is not an instance method, but a function on the
   plugin descriptor.  It is usually used by plugins to implement additional
   interfaces.  This plugin does not have any extension data, so this function
   returns NULL.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
static const void*
extension_data(const char* uri)
{
	return NULL;
}

/**
   Every plugin must define an `LV2_Descriptor`.  It is best to define
   descriptors statically to avoid leaking memory and non-portable shared
   library constructors and destructors to clean up properly.
*/
static const LV2_Descriptor descriptor = {
	JAMRECORD_ST_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

/**
   The `lv2_descriptor()` function is the entry point to the plugin library.  The
   host will load the library and call this function repeatedly with increasing
   indices to find all the plugins defined in the library.  The index is not an
   indentifier, the URI of the returned descriptor is used to determine the
   identify of the plugin.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:  return &descriptor;
	default: return NULL;
	}
}
