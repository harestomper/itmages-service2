/*
 *      service-imaging.c
 *      
 *      Copyright 2012 Voldemar Khramtsov <harestomper@gmail.com>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "service-imaging.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define magick_wand_new()                     (NewMagickWand ())
#define magick_wand_init()                    (MagickWandGenesis ())
#define magick_wand_uninit()                  (MagickWandTerminus ())
#define magick_wand_load(wand, fname)         (MagickReadImage ((wand), (fname)))
#define magick_wand_get_width(wand)           (gint)  (MagickGetImageWidth ((wand)))
#define magick_wand_get_height(wand)          (gint)  (MagickGetImageHeight ((wand)))
#define magick_wand_get_hash(wand)            (gchar*)(MagickGetImageSignature ((wand)))
#define magick_wand_get_format(wand)          (gchar*)(MagickGetImageFormat ((wand)))
#define magick_wand_destroy(wand)             (DestroyMagickWand ((wand)))
#define magick_wand_rotate(wand, pixels, angle) (MagickRotateImage ((wand), (pixels), angle))
#define magick_wand_write(wand, filename)     (MagickWriteImage ((wand), (filename)))
#define magick_wand_thumb(wand, w, h)         (MagickThumbnailImage ((wand), (size_t) w, (size_t) h))
#define magick_wand_scale(wand, w, h)         (MagickScaleImage ((wand), (size_t) w, (size_t) h))
#define magick_wand_region(wand, x, y, w, h)  (MagickGetImageRegion ((wand), w, h, x, y));
#define magick_pixel_new()                    (NewPixelWand ())
#define magick_pixel_destroy(pixel_wand)      (DestroyPixelWand ((pixel_wand)))
#define magick_wand_ping_image(wand, filename) (MagickPingImage ((wand), (filename)))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
typedef struct {
    gint x;
    gint y;
    gint width_in;
    gint height_in;
    gint width_out;
    gint height_out;
    gint width;
    gint height;
} Area;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void calculate_new_region (gint limit_w, gint limit_h, gint tolerance, Area *area)
{
    gdouble win, hin, k;

    win = (gdouble) area->width_in;
    hin = (gdouble) area->height_in;

    k = win / hin;

    area->x = 0;
    area->y = 0;

    if (limit_w == -1)
        limit_w = limit_h > 0 ? limit_h : area->width_in;

    if (limit_h == -1)
        limit_h = limit_w > 0 ? limit_w : area->height_in;
        
    area->height = area->height_out = limit_h;
    area->width = area->width_out = limit_w;

    if (win >= hin)
        area->width = area->width_out = MAX (area->height_out * k, 1);
    else
        area->height = area->height_out = MAX (area->width_out / k, 1);

    if (area->width_out - tolerance > limit_w
                 || area->width_out + tolerance < limit_w
                 || area->height_out - tolerance > limit_h
                 || area->height_out + tolerance < limit_h)
    {
        area->width_out = area->height_out = MAX (limit_w, limit_h);

        if (win >= hin)
            area->width_out = area->height_out * k;
        else
            area->height_out = area->width_out / k;

        area->x = MAX ((area->width_out - limit_w) / 2.0, 0);
        area->y = MAX ((area->height_out - limit_h) / 2.0, 0);
        area->width_out = limit_w;
        area->height_out = limit_h;
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_imaging_resize (const gchar *filein,
                                 const gchar *fileout,
                                 gint width,
                                 gint height,
                                 gboolean aspect)
{
    gchar *end_file;
    MagickWand *wand;
    gboolean result = FALSE;

    if (filein == NULL || strlen (filein) == 0)
        return FALSE;

    if (fileout == NULL || strlen (fileout) == 0)
        end_file = service_build_modified_filename (filein, "resized");
    else
        end_file = service_build_modified_filename (fileout, NULL);

    if (!service_check_path (filein, FALSE) || !service_check_path (fileout, TRUE))
        return FALSE;

    if (width <= 0 || height <= 0)
        return FALSE;

    magick_wand_init ();
    wand = magick_wand_new ();

    if (magick_wand_load (wand, filein) == MagickTrue) {
        Area area = {0};

        area.width = area.width_in = magick_wand_get_width (wand);
        area.height = area.height_in = magick_wand_get_height (wand);

        if (aspect)
            calculate_new_region (width, height, MAX (width, height), &area);

        if (magick_wand_scale (wand, area.width, area.height) == MagickTrue) {
            result = magick_wand_write (wand, end_file) == MagickTrue;
        }
    }

    g_free (end_file);
    magick_wand_destroy (wand);
    magick_wand_uninit ();

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_imaging_thombnail (const gchar *filename,
                                    const gchar *thumb,
                                    gint width,
                                    gint height,
                                    gboolean cut_out)
{
    gchar *end_file;
    MagickWand *wand;
    gboolean result = FALSE;

    if (filename == NULL || strlen (filename) == 0)
        return FALSE;

    if (!service_check_path (filename, FALSE) || !service_check_path (thumb, TRUE))
        return FALSE;

    if (width <= 0 || height <= 0)
        return FALSE;

    if (thumb == NULL || strlen (thumb) == 0)
        end_file = service_build_modified_filename (filename, "thumb");
    else
        end_file = service_build_modified_filename (thumb, NULL);

    magick_wand_init ();
    wand = magick_wand_new ();

    if (magick_wand_load (wand, filename) == MagickTrue) {
        Area area = {0};

        area.width_in = magick_wand_get_width (wand);
        area.height_in = magick_wand_get_height (wand);

        calculate_new_region (width, height, 20, &area);

        if (magick_wand_thumb (wand, area.width, area.height) == MagickTrue) {

            if (cut_out) {
                MagickWand *temp;

                temp = magick_wand_region (wand, area.width_out, area.height_out, area.x, area.y);
                magick_wand_destroy (wand);
                wand = temp;
            }

            result = magick_wand_write (wand, end_file) == MagickTrue;
        }
    }

    g_free (end_file);
    magick_wand_destroy (wand);
    magick_wand_uninit ();

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_imaging_rotate (const gchar *filein, const gchar *fileout, gint orientation)
{
    gboolean result = FALSE;
    gchar *end_file = NULL;
    gint angle = 0;

    if (filein == NULL || strlen (filein) == 0)
        return FALSE;

    switch (orientation) {
      case 0: angle = 0; break;
      case 1: angle = -90; break;
      case 2: angle = -180; break;
      case 3: angle = -270; break;
      default: return FALSE;
    }
    
    if (service_check_path (filein, FALSE))
    {
      MagickWand *wand;

      magick_wand_init ();
      wand = magick_wand_new ();
      
      if (magick_wand_load (wand, filein) == MagickTrue)
      {
        PixelWand *pixel_wand;

        pixel_wand = magick_pixel_new ();
        PixelSetColor (pixel_wand, "none");

        magick_wand_rotate (wand, pixel_wand, angle);

        if (fileout == NULL || strlen (fileout) == 0)
            end_file = service_build_modified_filename (filein, "rotated");
        else
            end_file = service_build_modified_filename (fileout, NULL);

        result = magick_wand_write (wand, end_file) == MagickTrue;

        magick_wand_destroy (wand);
        magick_pixel_destroy (pixel_wand);
        g_free (end_file);
      }

      magick_wand_uninit ();
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_imaging_geometry (const gchar *filename,
                                   gint *width,
                                   gint *height,
                                   gint64 *size,
                                   gchar **mime,
                                   gchar **hash)
{
    MagickWand *wand;
    gboolean result = FALSE;
    struct stat st;
    
    g_return_val_if_fail (filename != NULL, FALSE);

    if (stat (filename, &st) == -1 || st.st_size > 6291456)
        return FALSE;

    magick_wand_init ();
    wand = magick_wand_new ();

    if (magick_wand_ping_image (wand, filename) == MagickTrue) {
        gchar* format;

        format = magick_wand_get_format (wand);

        if (mime) {
            if (g_strcmp0 ("PNG", format) == 0) {
              *mime = "image/png";
            } else if (g_strcmp0 ("GIF", format) == 0) {
              *mime = "image/gif";
            } else if (g_strcmp0 ("JPEG", format) == 0) {
              *mime = "image/jpeg";
            } else {
                magick_wand_uninit ();
                g_free (format);
                return FALSE;
            }
            result = TRUE;
        }

        if (width)
          *width = magick_wand_get_width (wand);

        if (height)
          *height = magick_wand_get_height (wand);

        if (hash)
          *hash = service_image_info_get_hash (filename);
//          *hash = magick_wand_get_hash (wand);

        if (size)
          *size = (gint64) st.st_size;

        g_free (format);
    }

    magick_wand_uninit ();

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_imaging_save_buffer (const gchar *filename, const gchar *filename_out)
{
    MagickWand *wand;
    gboolean result = FALSE;

    if (filename == NULL)
        return FALSE;
        
    magick_wand_init ();
    wand = magick_wand_new ();
    
    if (magick_wand_load (wand, filename) == MagickTrue) {
        if (filename)
            result = magick_wand_write (wand, filename_out) == MagickTrue;
        else
            result = TRUE;
    }

    magick_wand_destroy (wand);
    magick_wand_uninit ();

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

