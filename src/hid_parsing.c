#define HID_INTERNAL

#include <hid.h>
#include <hid_helpers.h>

#include <string.h>

#include <debug.h>
#include <assert.h>

#if WIN32
  #define snprintf _snprintf
#endif

static void hid_prepare_parse_path(HIDInterface* const hidif,
    int const path[], unsigned int const depth)
{
  unsigned int i = 0;

  ASSERT(hid_is_opened(hidif));
  ASSERT(hidif->hid_data);

  TRACE_PRINT("preparing search path of depth %d for parse tree of USB device %s...",
      depth, hidif->id);
  for (; i < depth; ++i) {
    hidif->hid_data->Path.Node[i].UPage = path[i] >> 16;
    hidif->hid_data->Path.Node[i].Usage = path[i] & 0x0000ffff;
  }

  hidif->hid_data->Path.Size = depth;

  TRACE_PRINT("search path prepared for parse tree of USB device %s.", hidif->id);
}

hid_return hid_init_parser(HIDInterface* const hidif)
{
  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot initialise parser of unopened HIDinterface.");
    return HID_RET_DEVICE_NOT_OPENED;
  }
  
  ASSERT(!hidif->hid_parser);
  ASSERT(!hidif->hid_data);

  TRACE_PRINT("initialising the HID parser for USB Device %s...", hidif->id);

  TRACE_PRINT("allocating space for HIDData structure...");
  hidif->hid_data = (HIDData*)malloc(sizeof(HIDData));
  if (!hidif->hid_data) {
    ERROR_PRINT("failed to allocate memory for HIDData structure of USB device %s.",
        hidif->id);
    return HID_RET_FAIL_ALLOC;
  }
  TRACE_PRINT("successfully allocated memory for HIDData strcture.");

  TRACE_PRINT("allocating space for HIDParser structure...");
  hidif->hid_parser = (HIDParser*)malloc(sizeof(HIDParser));
  if (!hidif->hid_parser) {
    ERROR_PRINT("failed to allocate memory for HIDParser structure of USB device %s.",
        hidif->id);
    return HID_RET_FAIL_ALLOC;
  }
  TRACE_PRINT("successfully allocated memory for HIDParser strcture.");

  NOTICE_PRINT("successfully initialised the HID parser for USB Device %s.",
      hidif->id);
  
  return HID_RET_SUCCESS;
}

hid_return hid_prepare_parser(HIDInterface* const hidif)
{
  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot prepare parser of unopened HIDinterface.");
    return HID_RET_DEVICE_NOT_OPENED;
  }
  ASSERT(hidif->hid_parser);
  
  TRACE_PRINT("setting up the HID parser for USB device %s...", hidif->id);

  hid_reset_parser(hidif);

  TRACE_PRINT("dumping the raw report descriptor");
  {
	  char buffer[160], tmp[10];
	  int i;

	  sprintf(buffer, "0x%03x: ", 0);
	  for(i=0; i<hidif->hid_parser->ReportDescSize; i++) {
		  if(!(i % 8)) {
			  if(i != 0) TRACE_PRINT("%s", buffer);
			  sprintf(buffer, "0x%03x: ", i);
		  }
		  sprintf(tmp, "0x%02x ", (int)(hidif->hid_parser->ReportDesc[i]));
		  strcat(buffer, tmp);
	  }
	  TRACE_PRINT("%s", buffer);
  }
  
  /* TODO: the return value here should be used, no? */
  TRACE_PRINT("parsing the HID tree of USB device %s...", hidif->id);
  HIDParse(hidif->hid_parser, hidif->hid_data);

  NOTICE_PRINT("successfully set up the HID parser for USB device %s.", hidif->id);

  return HID_RET_SUCCESS;
}

void hid_reset_parser(HIDInterface* const hidif)
{
  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot prepare parser of unopened HIDinterface.");
    return;
  }
  ASSERT(hidif->hid_parser);
  
  TRACE_PRINT("resetting the HID parser for USB device %s...", hidif->id);
  ResetParser(hidif->hid_parser);
}

hid_return hid_find_object(HIDInterface* const hidif,
    int const path[], unsigned int const depth, unsigned int const size)
{
  char* buffer = (char*)malloc(depth * (strlen("0xdeadbeef") + 1));

  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot prepare parser of unopened HIDinterface.");
    return HID_RET_DEVICE_NOT_OPENED;
  }
  ASSERT(hidif->hid_parser);
  ASSERT(hidif->hid_data);
  
  hid_prepare_parse_path(hidif, path, depth);

  if (FindObject(hidif->hid_parser, hidif->hid_data, size) == 1) {
    NOTICE_PRINT("found requested item.");
	free(buffer);
    return HID_RET_SUCCESS;
  }

  hid_format_path(buffer, depth * (strlen("0xdeadbeef") + 1), path, depth); 
  WARNING_PRINT("can't find requested item %s of USB device %s.\n", buffer, hidif->id);
  free(buffer);
  
  return HID_RET_NOT_FOUND;
}

hid_return hid_extract_value(HIDInterface* const hidif,
    unsigned char *const buffer, double *const value)
{
  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot extract value from unopened HIDinterface.");
    return HID_RET_DEVICE_NOT_OPENED;
  }
  ASSERT(hidif->hid_parser);
  ASSERT(hidif->hid_data);

  if (!buffer) {
    ERROR_PRINT("cannot extract value from USB device %s into NULL raw buffer.",
        hidif->id);
    return HID_RET_INVALID_PARAMETER;
  }

  if (!value) {
    ERROR_PRINT("cannot extract value from USB device %s into NULL value buffer.",
        hidif->id);
    return HID_RET_INVALID_PARAMETER;
  }
  
  TRACE_PRINT("extracting data value...");

  /* Extract the data value */
  GetValue(buffer, hidif->hid_data);

  /* FIXME: unit conversion and exponent?! */
  *value = hidif->hid_data->Value;
	
  return HID_RET_SUCCESS;
}

hid_return hid_get_report_size(HIDInterface* const hidif,
    unsigned int const reportID, unsigned int const reportType,
    unsigned int *size)
{
  if (!hid_is_opened(hidif)) {
    ERROR_PRINT("cannot get report size of unopened HIDinterface.");
    return HID_RET_DEVICE_NOT_OPENED;
  }
  ASSERT(hidif->hid_parser);
  ASSERT(hidif->hid_data);
  
  if (!size) {
    ERROR_PRINT("cannot read report size from USB device %s into NULL size buffer.",
        hidif->id);
    return HID_RET_INVALID_PARAMETER;
  }
  
  /* FIXME: GetReportOffset has to be rewritten! */
  *size = *GetReportOffset(hidif->hid_parser, reportID, reportType);
	
  return HID_RET_SUCCESS;
}

hid_return hid_format_path(char* const buffer, unsigned int length,
    int const path[], unsigned int const depth)
{
  byte const ITEMSIZE = strlen("0xdeadbeef") + 1;
  unsigned int i = 0;

  if (!buffer) {
    ERROR_PRINT("cannot format path into NULL buffer.");
    return HID_RET_INVALID_PARAMETER;
  }

  TRACE_PRINT("formatting device path...");
  for (; i < depth; ++i) {
    if (length < ITEMSIZE) {
      WARNING_PRINT("not enough space in buffer to finish formatting of path.")
      return HID_RET_OUT_OF_SPACE;
    }
    snprintf(buffer + i * ITEMSIZE, ITEMSIZE, "0x%08x.", path[i]);
    length -= ITEMSIZE;
  }
  buffer[i * ITEMSIZE - 1] = '\0';

  return HID_RET_SUCCESS;
}

/* COPYRIGHT --
 *
 * This file is part of libhid, a user-space HID access library.
 * libhid is (c) 2003-2005
 *   Martin F. Krafft <libhid@pobox.madduck.net>
 *   Charles Lepple <clepple@ghz.cc>
 *   Arnaud Quette <arnaud.quette@free.fr> && <arnaud.quette@mgeups.com>
 * and distributed under the terms of the GNU General Public License.
 * See the file ./COPYING in the source distribution for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
