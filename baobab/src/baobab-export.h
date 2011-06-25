/*
 * baobab-export.h
 * This file is part of baobab
 *
 * Copyright (C) 2011 Oliver Gerlich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __BAOBAB_EXPORT_H__
#define __BAOBAB_EXPORT_H__

#include <gio/gio.h>


#define BAOBAB_IMPORT_ERROR baobab_import_error_quark ()
GQuark baobab_import_error_quark (void);

void baobab_import (GFile* infileName, GError **error);
void baobab_export (GFile* outfileName);

#endif /* __BAOBAB_EXPORT_H_ */
