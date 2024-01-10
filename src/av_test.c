/* hacktv - Analogue video transmitter for the HackRF                    */
/*=======================================================================*/
/* Copyright 2017 Philip Heron <phil@sanslogic.co.uk>                    */
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

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "hacktv.h"
#include "graphics.h"

/* AV test pattern source */
typedef struct {
	int vid_width;
	int vid_height;
	uint32_t *video;
	int16_t *audio;
	size_t audio_samples;
	int img_width;
	int img_height;
	image_t image;
	av_font_t *font[10];
} av_test_t;

static int _test_read_video(void *ctx, av_frame_t *frame)
{
	av_test_t *s = ctx;
	av_frame_init(frame, s->vid_width, s->vid_height, s->video, 1, s->vid_width);
	av_set_display_aspect_ratio(frame, (rational_t) { 4, 3 });

	/* Get current time */
	char timestr[9];
	time_t secs = time(0);
	struct tm *local = localtime(&secs);
	sprintf(timestr, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
		
	/* Print clock */
	if(s->font[0])
	{
		print_generic_text(	s->font[0],
							s->video,
							timestr,
							s->font[0]->x_loc, s->font[0]->y_loc, 0, 1, 0, 1);
	}
						
	return(AV_OK);
}

static int16_t *_test_read_audio(void *ctx, size_t *samples)
{
	av_test_t *s = ctx;
	*samples = s->audio_samples;
	return(s->audio);
}

static int _test_close(void *ctx)
{
	av_test_t *s = ctx;
	if(s->video) free(s->video);
	if(s->audio) free(s->audio);
	free(s);
	return(HACKTV_OK);
}

static uint8_t _hamming_bars(int x, int sr, int frequency)
{
	double sample, y;
	
	y = sr;
	y = (frequency / y) * x;
	sample = sin(y * 2 * M_PI) + 1;

	return ((sample * 0x7F));
}

int av_test_open(vid_t *vid, char *test_screen)
{
	uint32_t const bars[8] = {
		0x000000,
		0x0000BF,
		0xBF0000,
		0xBF00BF,
		0x00BF00,
		0x00BFBF,
		0xBFBF00,
		0xFFFFFF,
	};

	/* Frequency of the sine-wave for each 'bar' in KHz */
	uint16_t sine_bars[5] = { 800, 1800, 2800, 3800, 4800 };
	int sine_bars_pos[5] = { 3, 0, 0, 0, 0 };

	av_test_t *av;

	vid_config_t *conf = &vid->conf;
	int c, x, y, z;
	double d;
	int16_t l;
	int y_start, y_end, x_start, x_end, start_pos, ycentre_start, ycentre_end;
	
	av = calloc(1, sizeof(av_test_t));
	if(!av)
	{
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	/* Generate a basic test pattern */
	av->vid_width = vid->av.width;
	av->vid_height = vid->av.height;
	av->video = malloc(av->vid_width * av->vid_height * sizeof(uint32_t));
	if(!av->video)
	{
		free(av);
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	for(y = 0; y < av->vid_height; y++)
	{
		for(x = 0; x < av->vid_width; x++)
		{
			if(y < av->vid_height - 140)
			{
				/* 75% colour bars */
				c = 7 - x * 8 / av->vid_width;
				c = bars[c];
			}
			else if(y < av->vid_height - 120)
			{
				/* 75% red */
				c = 0xBF0000;
			}
			else if(y < av->vid_height - 100)
			{
				/* Gradient black to white */
				c = x * 0xFF / (av->vid_width - 1);
				c = c << 16 | c << 8 | c;
			}
			else
			{
				/* 8 level grey bars */
				c = x * 0xFF / (av->vid_width - 1);
				c &= 0xE0;
				c = c | (c >> 3) | (c >> 6);
				c = c << 16 | c << 8 | c;
			}
			
			av->video[y * av->vid_width + x] = c;
		}
	}
	
	if(test_screen == NULL) test_screen = "pm5544";
	
	float img_ratio = (strcmp(test_screen, "pm5644") == 0) ? 16.0 / 9.0 : 4.0 / 3.0;
	
	/* Initialise default fonts */
	
	/* Clock */
	font_init(vid, 56, img_ratio);
	av->font[0] = vid->av_font;
	av->font[0]->x_loc = 50;
	av->font[0]->y_loc = 50;
	
	/* HACKTV text*/
	font_init(vid, 72, img_ratio);
	av->font[1] = vid->av_font;
	av->font[1]->x_loc = 50;
	av->font[1]->y_loc = 25;
	
	int sr = 20250.0 * (av->vid_width / 1052.0);
	
	/* Overlay test screen */
	if(av->vid_height == 576 && strcmp(test_screen, "colourbars") != 0)
	{
		if(load_png(&av->image, av->vid_width, av->vid_height, test_screen, 1.0, img_ratio, IMG_TEST) == HACKTV_OK)
		{	
			overlay_image(av->video, &av->image, av->vid_width, av->vid_height, IMG_POS_FULL);
			
			if(strcmp(test_screen, "pm5544") == 0)
			{
				av->font[0]->y_loc = 82.3;

				y_start = av->vid_height - 270;
				y_end = av->vid_height - 180;
				x_start = (av->vid_width / 18.0) * 8.5;
				x_end = (av->vid_width / 18.0) * 9.53;

				start_pos = (av->vid_width / 8.0) * 1.75;

				ycentre_start = 308;
				ycentre_end = 354;
				
				/* Generate hamming bars */
				for(y = 0; y < av->vid_height; y++)
				{
					for(x = 0; x < av->vid_width; x++)
					{
						if((y - 2 > y_start && y < y_end) && !((x > x_start && x < x_end) && y > ycentre_start && y < ycentre_end))
						{
							if(x > start_pos - 3 && x < av->vid_width - start_pos - 3)
							{
								z = x - start_pos;
								c = 4 - z * 9 / av->vid_width;
								c = _hamming_bars(z - (start_pos * 4) + sine_bars_pos[4 - c], sr, sine_bars[4 - (c < 0 ? 0 : c)]);
								c = c << 16 | c << 8 | c;
								av->video[y * av->vid_width + x] = c;
							}				
						}
					}
				}
			}
			else if(strcmp(test_screen, "pm5644") == 0)
			{
				av->font[0]->y_loc = 82;

				y_start = av->vid_height - 271;
				y_end = av->vid_height - 181;
				x_start = (av->vid_width / 24.0) * 11.51;
				x_end = (av->vid_width / 24.0) * 12.5;
				start_pos = (av->vid_width / 6.0) * 1.75;
				ycentre_start = 307;
				ycentre_end = 349;

				/* Generate hamming bars */
				for(y = 0; y < av->vid_height; y++)
				{
					for(x = 0; x < av->vid_width; x++)
					{
						if((y - 2 > y_start && y < y_end) && !((x > x_start && x < x_end) && y > ycentre_start && y < ycentre_end))
						{
							if(x > start_pos && x < av->vid_width - start_pos)
							{
								z = x - start_pos;
								c = 5 - z * ((9 / (4.0/3.0)) * (16.0/9.0)) / av->vid_width;
								c = _hamming_bars(z - (start_pos * 4) + 2, sr, sine_bars[4 - (c < 0 ? 0 : c)]);
								c = c << 16 | c << 8 | c;
								av->video[y * av->vid_width + x] = c;
							}				
						}

						/* Vertical grating */
						if(y > 181 && y < 393)
						{
							if(x > (av->vid_width / 24.0) * 1.52 && x < (av->vid_width / 24.0) * 2.45)
							{
								c = _hamming_bars(y, sr, 1800 - (y * 12));
								c = c << 16 | c << 8 | c;
								av->video[y * av->vid_width + x] = c;
							}

							if(x > (av->vid_width / 24.0) * 21.56 && x < (av->vid_width / 24.0) * 22.47)
							{
								c = _hamming_bars(y, sr, 1800 - (y * 12));
								c = c << 16 | c << 8 | c;
								av->video[(574 - y) * av->vid_width + x] = c;
							}
						}
					}
				}
			}
			else if(strcmp(test_screen, "fubk") == 0)
			{
				/* Reinit font with new size */
				font_init(vid, 44, img_ratio);
				av->font[0] = vid->av_font;
				av->font[0]->x_loc = 52;
				av->font[0]->y_loc = 55.5;
			}
			else if(strcmp(test_screen, "ueitm") == 0)
			{
				/* Don't display clock */
				av->font[0] = NULL;
			}
			
		}
		else
		{
			print_generic_text(	av->font[1], av->video, "HACKTV", av->font[1]->x_loc, av->font[1]->y_loc, 0, 1, 0, 1);
		}
	}
	else
	{
		print_generic_text(	av->font[1], av->video, "HACKTV", av->font[1]->x_loc, av->font[1]->y_loc, 0, 1, 0, 1);
	}
	
	/* Print logo, if enabled */
	if(conf->logo)
	{
		if(load_png(&vid->vid_logo, av->vid_width, av->vid_height, conf->logo, 0.75, img_ratio, IMG_LOGO) == HACKTV_OK)
		{
			overlay_image(av->video, &vid->vid_logo, av->vid_width, av->vid_height, vid->vid_logo.position);
		}
		else
		{
			conf->logo = NULL;
		}
	}
	
	/* Generate the 1khz test tones (BBC 1 style) */
	d = 1000.0 * 2 * M_PI * vid->av.sample_rate.den / vid->av.sample_rate.num;
	y = vid->av.sample_rate.num / vid->av.sample_rate.den * 64 / 100; /* 640ms */
	av->audio_samples = y * 10; /* 6.4 seconds */
	av->audio = malloc(av->audio_samples * 2 * sizeof(int16_t));
	if(!av->audio)
	{
		free(av->video);
		free(av);
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	for(x = 0; x < av->audio_samples; x++)
	{
		l = sin(x * d) * INT16_MAX * 0.1;
		
		if(x < y)
		{
			/* 0 - 640ms, interrupt left channel */
			av->audio[x * 2 + 0] = 0;
			av->audio[x * 2 + 1] = l;
		}
		else if(x >= y * 2 && x < y * 3)
		{
			/* 1280ms - 1920ms, interrupt right channel */
			av->audio[x * 2 + 0] = l;
			av->audio[x * 2 + 1] = 0;
		}
		else if(x >= y * 4 && x < y * 5)
		{
			/* 2560ms - 3200ms, interrupt right channel again */
			av->audio[x * 2 + 0] = l;
			av->audio[x * 2 + 1] = 0;
		}
		else
		{
			/* Use both channels for all other times */
			av->audio[x * 2 + 0] = l; /* Left */
			av->audio[x * 2 + 1] = l; /* Right */
		}
	}
	
	/* Register the callback functions */
	vid->av.av_source_ctx = av;
	vid->av.read_video = _test_read_video;
	vid->av.read_audio = _test_read_audio;
	vid->av.close = _test_close;
	
	return(HACKTV_OK);
}

