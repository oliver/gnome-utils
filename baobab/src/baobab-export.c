/*
 * baobab-export.c
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


#include <config.h>

#include <errno.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include "baobab-export.h"
#include "baobab.h"
#include "baobab-treeview.h"


GQuark
baobab_import_error_quark (void)
{
	return g_quark_from_static_string ("baobab-import-error-quark");
}


static gboolean
load_node (xmlNodePtr cur, int depth, int* max_depth, GError **error)
{
	gboolean errorFound = FALSE;

	if (depth > *max_depth)
		*max_depth = depth;
	depth++;

	if (xmlStrcmp(cur->name, "dir") == 0)
	{
		struct chan_data entry;
		entry.size = 0;
		entry.alloc_size = 0;
		entry.depth = depth -1;
		entry.elements = -1;
		entry.display_name = NULL;
		entry.parse_name = NULL;
		entry.tempHLsize = 0;

		/* parse attributes from XML */
		xmlChar* value;
		value = xmlGetProp(cur, "name");
		char* name = g_uri_unescape_string(value, NULL);
		printf("dir name: %s\n", name);
		entry.parse_name = name;
		entry.display_name = g_filename_display_name(entry.parse_name);
		xmlFree(value);

		gchar* endPtr;
		gboolean allParsed;

		value = xmlGetProp(cur, "size");
		errno = 0;
		entry.size = g_ascii_strtoull(value, &endPtr, 10);
		allParsed = (*endPtr == '\0');
		xmlFree(value);
		if (!allParsed || errno != 0)
		{
			g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "invalid size value");
			errorFound = TRUE;
			goto end;
		}

		value = xmlGetProp(cur, "allocated");
		errno = 0;
		entry.alloc_size = g_ascii_strtoull(value, &endPtr, 10);
		allParsed = (*endPtr == '\0');
		xmlFree(value);
		if (!allParsed || errno != 0)
		{
			g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "invalid alloc_size value");
			errorFound = TRUE;
			goto end;
		}

		value = xmlGetProp(cur, "hardlinksize");
		errno = 0;
		entry.tempHLsize = g_ascii_strtoull(value, &endPtr, 10);
		allParsed = (*endPtr == '\0');
		xmlFree(value);
		if (!allParsed || errno != 0)
		{
			g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "invalid tempHLsize value");
			errorFound = TRUE;
			goto end;
		}

		/* prefill the model */
		baobab_fill_model(&entry);

		/* load children */
		xmlNodePtr child = cur->xmlChildrenNode;
		while (child)
		{
			const gboolean success = load_node(child, depth, max_depth, error);
			if (!success)
			{
				errorFound = TRUE;
				goto end;
			}

			child = child->next;
		}

		/* set final values for this directory */
		value = xmlGetProp(cur, "elements");
		entry.elements = atoi(value);
		xmlFree(value);

		if (entry.elements >= 0)
		{
			baobab_fill_model(&entry);
		}

	end:
		g_free(entry.display_name);
		g_free(entry.parse_name);
	}

	return (!errorFound);
}


void
baobab_import (GFile *infileName, GError **error)
{
	printf("loading snapshot...\n");
	xmlDocPtr doc = xmlParseFile(g_file_get_path(infileName));
	if (!doc)
	{
		g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "file could not be read");
		return;
	}

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (!cur)
	{
		g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "file is empty");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, "dump") != 0)
	{
		g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "file is invalid (missing root node)");
		xmlFreeDoc(doc);
		return;
	}

	/* retrieve base path */
	GFile* targetPath = NULL;
	xmlChar* baseUri = xmlGetProp(cur, "baseuri");
	if (baseUri)
	{
		targetPath = g_file_new_for_uri(baseUri);
		xmlFree(baseUri);
	}
	else
	{
		xmlChar* basePath = xmlGetProp(cur, "basepath");
		if (basePath)
		{
			targetPath = g_file_new_for_path(basePath);
			xmlFree(basePath);
		}
		else
		{
			g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "file is invalid (missing base path/URI)");
			xmlFreeDoc(doc);
			return;
		}
	}

	/* find root <dir> node */
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next)
	{
		if (xmlStrcmp(cur->name, "dir") == 0)
		{
			break;
		}
	}

	if (!cur)
	{
		g_set_error(error, BAOBAB_IMPORT_ERROR, 0, "file is invalid (missing root directory node)");
		xmlFreeDoc(doc);
		return;
	}

	baobab_scan_prepare(targetPath);

	int max_depth = 0;
	load_node(cur, 0, &max_depth, error);

	baobab.model_max_depth = max_depth;
	baobab_scan_finish();

	xmlFreeDoc(doc);
	printf("done.\n");
}


static void
export_iter (GtkTreeModel* model, GtkTreeIter* it, xmlTextWriterPtr writer)
{
	gboolean valid = TRUE;
	while (valid)
	{
		xmlTextWriterStartElement(writer, "dir");
		GValue value = {0};

		gtk_tree_model_get_value(model, it, COL_H_PARSENAME, &value);
		g_assert( G_VALUE_HOLDS_STRING(&value) );
		char* escapedName;
		const char* parsedName = g_value_get_string(&value);
		if (parsedName[0] == '\0')
		{
			// if scan is stopped, it can happen that some top elements have no parse_name set;
			// fall back to display_name in that case:
			g_value_unset(&value);
			gtk_tree_model_get_value(model, it, COL_DIR_NAME, &value);
			g_assert( G_VALUE_HOLDS_STRING(&value) );
			const char* displayName = g_value_get_string(&value);
			escapedName = g_uri_escape_string(displayName, NULL, TRUE);
		}
		else
		{
			GFile* path = g_file_parse_name(parsedName);
			escapedName = g_uri_escape_string(g_file_get_basename(path), NULL, TRUE);
		}
		xmlTextWriterWriteAttribute(writer, "name", escapedName);
		g_free(escapedName);
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_SIZE, &value);
		g_assert( G_VALUE_HOLDS_UINT64(&value) );
		xmlTextWriterWriteFormatAttribute(writer, "size", "%" G_GUINT64_FORMAT, g_value_get_uint64(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_ALLOCSIZE, &value);
		g_assert( G_VALUE_HOLDS_UINT64(&value) );
		xmlTextWriterWriteFormatAttribute(writer, "allocated", "%" G_GUINT64_FORMAT, g_value_get_uint64(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_ELEMENTS, &value);
		g_assert( G_VALUE_HOLDS_INT(&value) );
		xmlTextWriterWriteFormatAttribute(writer, "elements", "%d", g_value_get_int(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_HARDLINK, &value);
		g_assert( G_VALUE_HOLDS_UINT64(&value) );
		xmlTextWriterWriteFormatAttribute(writer, "hardlinksize", "%" G_GUINT64_FORMAT, g_value_get_uint64(&value));
		g_value_unset(&value);

		if (gtk_tree_model_iter_has_child(model, it))
		{
			GtkTreeIter childIt;
			gtk_tree_model_iter_children(model, &childIt, it);
			export_iter(model, &childIt, writer);
		}
		xmlTextWriterEndElement(writer);

		valid = gtk_tree_model_iter_next(model, it);
	}
}

void
baobab_export (GFile *outfileName)
{
	printf("saving snapshot...\n");

	// preemptively delete file first:
	g_file_delete(outfileName, NULL, NULL);

	xmlTextWriterPtr writer = xmlNewTextWriterFilename(g_file_get_path(outfileName), 0);
	if (!writer)
	{
		printf("error opening output file\n");
		return;
	}

	xmlTextWriterSetIndent(writer, 2);
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
	xmlTextWriterStartElement(writer, "dump");

	if (g_file_is_native(baobab.current_location))
	{
		xmlTextWriterWriteAttribute(writer, "basepath", g_file_get_path(baobab.current_location));
	}
	else
	{
		xmlTextWriterWriteAttribute(writer, "baseuri", g_file_get_uri(baobab.current_location));
	}

	xmlTextWriterWriteComment(writer, " Baobab directory structure dump ");

	GtkTreeModel* model = GTK_TREE_MODEL(baobab.model);
	GtkTreeIter it;
	const gboolean valid = gtk_tree_model_get_iter_first(model, &it);

	if (valid)
	{
		export_iter(model, &it, writer);
	}

	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	printf("done\n");
}
