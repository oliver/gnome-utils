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

#include "baobab-export.h"
#include "baobab.h"
#include "baobab-treeview.h"


void
baobab_import (GFile *infileName)
{
	
}


static void
export_iter (GtkTreeModel* model, GtkTreeIter* it, GOutputStream* outFile)
{
	gboolean valid = TRUE;
	while (valid)
	{
		GString* s = g_string_new("<dir");
		GValue value = {0};

		gtk_tree_model_get_value(model, it, COL_H_PARSENAME, &value);
		g_assert( G_VALUE_HOLDS_STRING(&value) );
		GFile* path = g_file_new_for_path(g_value_get_string(&value));
		g_string_append_printf(s, " name=\"%s\"", g_file_get_basename(path));
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_SIZE, &value);
		g_assert( G_VALUE_HOLDS_UINT64(&value) );
		g_string_append_printf(s, " size=\"%" G_GUINT64_FORMAT "\"", g_value_get_uint64(&value) );
		g_value_unset(&value);

		gtk_tree_model_get_value(model, it, COL_H_ALLOCSIZE, &value);
		g_assert( G_VALUE_HOLDS_UINT64(&value) );
		g_string_append_printf(s, " allocated=\"%" G_GUINT64_FORMAT "\"", g_value_get_uint64(&value) );
		g_value_unset(&value);

		if (gtk_tree_model_iter_has_child(model, it))
			g_string_append(s, ">\n");
		else
			g_string_append(s, "/>\n");
		g_output_stream_write_all(outFile, s->str, s->len, NULL, NULL, NULL);
		g_string_free(s, TRUE);

		if (gtk_tree_model_iter_has_child(model, it))
		{
			GtkTreeIter childIt;
			gtk_tree_model_iter_children(model, &childIt, it);
			export_iter(model, &childIt, outFile);

			static const char DIR_END_TAG[] = "</dir>\n";
			g_output_stream_write_all(outFile, DIR_END_TAG, sizeof(DIR_END_TAG)-1, NULL, NULL, NULL);
		}

		valid = gtk_tree_model_iter_next(model, it);
	}
}

void
baobab_export (GFile *outfileName)
{
	printf("saving snapshot...\n");

	// preemptively delete file first:
	g_file_delete(outfileName, NULL, NULL);

	GError* error = NULL;
	GOutputStream* outFile = G_OUTPUT_STREAM(g_file_create(outfileName,
		G_FILE_CREATE_REPLACE_DESTINATION,
		NULL, &error));
	if (!outFile)
	{
		printf("error opening output file (%s)\n", error->message);
		g_error_free(error);
		return;
	}

	GString* s = g_string_new("");
	g_string_append(s, "<?xml version=\"1.0\" ?>\n");
	g_string_append(s, "<!-- Baobab directory structure dump -->\n");
	g_string_append(s, "<dump>\n");
	g_output_stream_write_all(outFile, s->str, s->len, NULL, NULL, NULL);

	GtkTreeModel* model = GTK_TREE_MODEL(baobab.model);
	GtkTreeIter it;
	const gboolean valid = gtk_tree_model_get_iter_first(model, &it);

	if (valid)
	{
		export_iter(model, &it, G_OUTPUT_STREAM(outFile));
	}

	g_string_assign(s, "</dump>\n");
	g_output_stream_write_all(outFile, s->str, s->len, NULL, NULL, NULL);
	g_string_free(s, TRUE);

	g_output_stream_close(outFile, NULL, NULL);
	printf("done\n");
}
