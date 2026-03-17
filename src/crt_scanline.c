/*****************************************************************************
 * crt_scanline.c : CRT Scanline video filter for VLC
 *****************************************************************************
 * Copyright (C) 2026 Authors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_configuration.h>
#include "filter_picture.h"

static int  Create  ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t *Filter( filter_t *, picture_t * );

#define FILTER_PREFIX "crtscanline-"

#define DARKNESS_TEXT N_("Scanline darkness")
#define DARKNESS_LONGTEXT N_( \
    "How dark the scanlines are at 1080p. " \
    "Scales down for lower resolutions. 0 = off. Default: 35" )

#define SPACING_TEXT N_("Line spacing (pixels at 480p)")
#define SPACING_LONGTEXT N_( \
    "Base scanline spacing at 480p. Scales with resolution. Default: 2" )

#define BLEND_TEXT N_("Smooth blending")
#define BLEND_LONGTEXT N_( \
    "Smooth sine-wave vs hard lines. Default: on" )

vlc_module_begin ()
    set_description( N_("CRT Scanline video filter") )
    set_shortname( N_("CRT Scanlines") )
    set_category( CAT_VIDEO )
    set_subcategory( SUBCAT_VIDEO_VFILTER )
    set_capability( "video filter", 0 )

    add_integer_with_range( FILTER_PREFIX "darkness", 35, 0, 100,
                            DARKNESS_TEXT, DARKNESS_LONGTEXT, false )
        change_safe()
    add_integer_with_range( FILTER_PREFIX "spacing", 2, 1, 20,
                            SPACING_TEXT, SPACING_LONGTEXT, false )
        change_safe()
    add_bool( FILTER_PREFIX "blend", true,
              BLEND_TEXT, BLEND_LONGTEXT, false )
        change_safe()

    add_shortcut( "crtscanline" )
    set_callbacks( Create, Destroy )
vlc_module_end ()

static const char *const ppsz_filter_options[] = {
    "darkness", "spacing", "blend",
    NULL
};

struct filter_sys_t
{
    int i_dummy;
};

static int Create( vlc_object_t *p_this )
{
    filter_t *p_filter = (filter_t *)p_this;

    if( p_filter->fmt_in.video.i_chroma != p_filter->fmt_out.video.i_chroma )
    {
        msg_Err( p_filter, "Input and output chromas don't match" );
        return VLC_EGENERIC;
    }

    switch( p_filter->fmt_in.video.i_chroma )
    {
        CASE_PLANAR_YUV
            break;
        default:
            msg_Dbg( p_filter,
                     "Unsupported chroma (%4.4s), need planar YUV. "
                     "Try disabling hardware decoding.",
                     (char *)&p_filter->fmt_in.video.i_chroma );
            return VLC_EGENERIC;
    }

    filter_sys_t *p_sys = malloc( sizeof( *p_sys ) );
    if( !p_sys )
        return VLC_ENOMEM;
    p_filter->p_sys = p_sys;

    config_ChainParse( p_filter, FILTER_PREFIX, ppsz_filter_options,
                       p_filter->p_cfg );

    p_filter->pf_video_filter = Filter;

    msg_Info( p_filter, "CRT Scanline filter initialized (live control ready)" );

    return VLC_SUCCESS;
}

static void Destroy( vlc_object_t *p_this )
{
    filter_t *p_filter = (filter_t *)p_this;
    free( p_filter->p_sys );
}

/*****************************************************************************
 * Filter: apply CRT scanline effect
 *****************************************************************************
 *
 * Reads parameters from global config EVERY FRAME so that external tools
 * (Lua extensions, RC interface) can adjust them live via vlc.config.set().
 *
 * darkness=0 short-circuits to pure frame copy (zero processing cost).
 *
 *****************************************************************************/
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
    picture_t *p_outpic;

    if( !p_pic )
        return NULL;

    p_outpic = filter_NewPicture( p_filter );
    if( !p_outpic )
    {
        msg_Warn( p_filter, "can't get output picture" );
        picture_Release( p_pic );
        return NULL;
    }

    /* Read parameters from global config — live adjustable */
    int i_base_darkness = (int)config_GetInt( p_filter,
                                              FILTER_PREFIX "darkness" );
    int i_base_spacing  = (int)config_GetInt( p_filter,
                                              FILTER_PREFIX "spacing" );
    int b_blend         = config_GetInt( p_filter,
                                         FILTER_PREFIX "blend" ) != 0;

    /* Clamp */
    if( i_base_darkness < 0 )   i_base_darkness = 0;
    if( i_base_darkness > 100 ) i_base_darkness = 100;
    if( i_base_spacing < 1 )    i_base_spacing = 1;
    if( i_base_spacing > 20 )   i_base_spacing = 20;

    /* Short-circuit: darkness=0 means effect is off */
    if( i_base_darkness == 0 )
    {
        picture_Copy( p_outpic, p_pic );
        return CopyInfoAndRelease( p_outpic, p_pic );
    }

    const int i_height = p_pic->p[Y_PLANE].i_visible_lines;

    /* Scale spacing (ref: 480p) */
    double f_spacing = (double)i_base_spacing * (double)i_height / 480.0;
    if( f_spacing < 1.5 )
        f_spacing = 1.5;

    /* Scale darkness (ref: 1080p) */
    double f_dark_ratio = (double)i_height / 1080.0;
    if( f_dark_ratio > 1.0 )  f_dark_ratio = 1.0;
    if( f_dark_ratio < 0.15 ) f_dark_ratio = 0.15;

    int i_eff_darkness = (int)( (double)i_base_darkness * f_dark_ratio + 0.5 );

    const int i_bright = 256;
    const int i_dark   = 256 - ( ( i_eff_darkness * 256 ) / 100 );

    const double f_two_pi = 6.28318530717958647692;

    for( int i_plane = 0; i_plane < p_pic->i_planes; i_plane++ )
    {
        const plane_t *p_src = &p_pic->p[i_plane];
        plane_t *p_dst = &p_outpic->p[i_plane];

        const int i_lines = p_src->i_visible_lines;
        const int i_visible_pitch = p_src->i_visible_pitch;

        if( i_plane != Y_PLANE )
        {
            for( int y = 0; y < i_lines; y++ )
            {
                memcpy( &p_dst->p_pixels[y * p_dst->i_pitch],
                        &p_src->p_pixels[y * p_src->i_pitch],
                        i_visible_pitch );
            }
            continue;
        }

        for( int y = 0; y < i_lines; y++ )
        {
            const uint8_t *p_in  = &p_src->p_pixels[y * p_src->i_pitch];
            uint8_t       *p_out = &p_dst->p_pixels[y * p_dst->i_pitch];

            int i_scale;

            if( b_blend )
            {
                double f_phase = cos( (double)y * f_two_pi / f_spacing );
                int i_mid   = ( i_bright + i_dark ) / 2;
                int i_range = ( i_bright - i_dark ) / 2;
                i_scale = i_mid + (int)( f_phase * (double)i_range );
            }
            else
            {
                int i_sp = (int)( f_spacing + 0.5 );
                if( i_sp < 2 ) i_sp = 2;
                i_scale = ( ( y % i_sp ) < ( i_sp / 2 ) ) ? i_bright : i_dark;
            }

            if( i_scale >= i_bright )
            {
                memcpy( p_out, p_in, i_visible_pitch );
            }
            else
            {
                for( int x = 0; x < i_visible_pitch; x++ )
                {
                    p_out[x] = (uint8_t)( ( (unsigned)p_in[x]
                                           * (unsigned)i_scale ) >> 8 );
                }
            }
        }
    }

    return CopyInfoAndRelease( p_outpic, p_pic );
}
