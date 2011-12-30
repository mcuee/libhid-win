#ifndef __INCLUDED_DEBUG_H__
#define __INCLUDED_DEBUG_H__

#ifndef HID_INTERNAL
#  error "this file is only supposed to be used from within libhid."
#endif /* HID_INTERNAL */

#include <hid.h>
#include <usb.h>

extern HIDDebugLevel hid_debug_level;
extern FILE* hid_debug_stream;

#define DEBUGPRINTF(t, s, ...) if (hid_debug_stream) { \
                                  fprintf(hid_debug_stream, "%s: %s(): ", t, __FUNCTION__); \
                                  fprintf(hid_debug_stream, s "\n", __VA_ARGS__); \
                                }
#define TRACE_PRINT(s, ...) if (hid_debug_level & HID_DEBUG_TRACES) { DEBUGPRINTF("  TRACE_PRINT", s, __VA_ARGS__) }
#define NOTICE_PRINT(s, ...) if (hid_debug_level & HID_DEBUG_NOTICES) { DEBUGPRINTF(" NOTICE_PRINT", s, __VA_ARGS__) }
#define WARNING_PRINT(s, ...) if (hid_debug_level & HID_DEBUG_WARNINGS) { DEBUGPRINTF("WARNING_PRINT", s, __VA_ARGS__) }
#define ERROR_PRINT(s, ...) if (hid_debug_level & HID_DEBUG_ERRORS) { DEBUGPRINTF("  ERROR", s, __VA_ARGS__) }

void trace_usb_bus(FILE* const out, struct usb_bus const* const usbbus);
void trace_usb_device(FILE* const out, struct usb_device const* const usbdev);
void trace_usb_device_descriptor(FILE* const out, struct usb_device_descriptor const* const descriptor);
void trace_usb_config_descriptor(FILE* const out, struct usb_config_descriptor const* const config);
void trace_usb_dev_handle(FILE* const out, usb_dev_handle const* const usbdev_h);

#endif /* __INCLUDED_DEBUG_H__ */

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
