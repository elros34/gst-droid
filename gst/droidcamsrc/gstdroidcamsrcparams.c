/*
 * gst-droid
 *
 * Copyright (C) 2014 Mohammed Sameer <msameer@foolab.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gstdroidcamsrcparams.h"

GST_DEBUG_CATEGORY_EXTERN (gst_droidcamsrc_debug);
#define GST_CAT_DEFAULT gst_droidcamsrc_debug

static void
gst_droidcamsrc_params_destroy_list (gpointer data)
{
  GList *list = (GList *) data;

  g_list_free_full (list, (GDestroyNotify) g_free);
}

static void
gst_droidcamsrc_params_parse_key_values (GstDroidCamSrcParams * params,
    const char *key, const char *values)
{
  char **value = g_strsplit (values, ",", -1);
  char **val = value;
  GList *list = NULL;

  while (*val) {
    list = g_list_append (NULL, g_strdup (*val));
    ++val;
  }

  g_hash_table_insert (params->params, g_strdup (key), list);

  g_strfreev (value);
}

static void
gst_droidcamsrc_params_parse_key_value (GstDroidCamSrcParams * params,
    const char *key, const char *value)
{
  if (g_strrstr (value, ",")) {
    /* needs darther splitting */
    gst_droidcamsrc_params_parse_key_values (params, key, value);
  } else {
    /* we are done */
    g_hash_table_insert (params->params, g_strdup (key), g_list_append (NULL,
            g_strdup (value)));
  }
}

static void
gst_droidcamsrc_params_parse (GstDroidCamSrcParams * params, const char *part)
{
  gchar **parts = g_strsplit (part, "=", 2);
  gchar *key = parts[0];
  gchar *value = key ? parts[1] : NULL;

  GST_LOG ("param %s = %s", key, value);

  if (!key || !value) {
    goto out;
  }

  gst_droidcamsrc_params_parse_key_value (params, key, value);

out:
  g_strfreev (parts);
}

GstDroidCamSrcParams *
gst_droidcamsrc_params_new (const gchar * params)
{
  GstDroidCamSrcParams *param = g_slice_new0 (GstDroidCamSrcParams);
  gchar **parts = g_strsplit (params, ";", -1);
  gchar **part = parts;

  param->params = g_hash_table_new_full (g_str_hash, g_str_equal,
      (GDestroyNotify) g_free,
      (GDestroyNotify) gst_droidcamsrc_params_destroy_list);

  while (*part) {
    gst_droidcamsrc_params_parse (param, *part);
    ++part;
  }

  g_strfreev (parts);

  g_mutex_init (&param->lock);

  return param;
}

void
gst_droidcamsrc_params_destroy (GstDroidCamSrcParams * params)
{
  g_mutex_clear (&params->lock);
  g_hash_table_unref (params->params);
  g_slice_free (GstDroidCamSrcParams, params);
}
