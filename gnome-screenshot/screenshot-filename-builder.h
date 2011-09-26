/* screenshot-filename-builder.c - Builds a filename suitable for a screenshot
 *
 * Copyright (C) 2008, 2011 Cosimo Cecchi <cosimoc@gnome.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef __SCREENSHOT_FILENAME_BUILDER_H__
#define __SCREENSHOT_FILENAME_BUILDER_H__

#include <gio/gio.h>

void screenshot_build_filename_async (const gchar *save_dir,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data);
gchar *screenshot_build_filename_finish (GAsyncResult *result,
                                         GError **error);

#endif /* __SCREENSHOT_FILENAME_BUILDER_H__ */
