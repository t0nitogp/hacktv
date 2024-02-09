/* hacktv - Analogue video transmitter for the HackRF                    */
/*=======================================================================*/
/* Copyright 2020 Philip Heron <phil@sanslogic.co.uk>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SUBTITLES_H_
#define SUBTITLES_H_

#include <libavcodec/avcodec.h>
#include "video.h"

#define SUB_BITMAP 0
#define SUB_TEXT 1

typedef struct {
	int pos;
	int type;
    int index;
    int start_time;
    int end_time;
    char text[256];
	int number_of_subs;
	uint32_t *bitmap;
	int bitmap_width;
	int bitmap_height;
	void *font;
} av_subs_t;

extern void load_text_subtitle(av_subs_t *subs, uint32_t start_time, uint32_t duration, char *fmt);
extern int subs_init_file(char *filename, av_subs_t **s);
extern int subs_init_ffmpeg(av_subs_t **s);
extern char *get_text_subtitle(av_subs_t *subs, uint32_t ts);
extern int get_bitmap_subtitle(av_subs_t *subs, uint32_t current_timestamp, int *w, int *h);
extern void load_bitmap_subtitle(AVSubtitle *av_sub, av_subs_t *subs, int bitmap_width, int w, int h, uint32_t pts, int bitmap_scale);
extern int get_subtitle_type(av_subs_t *subs);
#endif
