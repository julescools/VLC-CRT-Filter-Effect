/*****************************************************************************
 * filter_picture.h: Common picture functions for filters
 *****************************************************************************
 * Copyright (C) 2007 VLC authors and VideoLAN
 *
 * Authors: Antoine Cellerier <dionoea at videolan dot org>
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

#define CASE_PLANAR_YUV_SQUARE              \
        case VLC_CODEC_I420:   \
        case VLC_CODEC_J420:   \
        case VLC_CODEC_YV12:   \
        case VLC_CODEC_I411:   \
        case VLC_CODEC_I410:   \
        case VLC_CODEC_I444:   \
        case VLC_CODEC_J444:   \
        case VLC_CODEC_YUVA:

#define CASE_PLANAR_YUV_NONSQUARE           \
        case VLC_CODEC_I422:   \
        case VLC_CODEC_J422:

#define CASE_PLANAR_YUV                     \
        CASE_PLANAR_YUV_SQUARE              \
        CASE_PLANAR_YUV_NONSQUARE           \

static inline picture_t *CopyInfoAndRelease( picture_t *p_outpic, picture_t *p_inpic )
{
    picture_CopyProperties( p_outpic, p_inpic );
    picture_Release( p_inpic );
    return p_outpic;
}
