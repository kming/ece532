/*
 * track_wand.c
 *
 *  Created on: 2014-02-19
 *      Author: Keiming
 */

/***************************** Include Files ********************************/

#include <stdio.h>
#include "xparameters.h"
#include "xbasic_types.h"
#include "xutil.h"
#include "xil_assert.h"
#include "track_wand.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/
void track_wand (u32 *frame_ptr, Position *pos, u32 hres, u32 vres)
{
	pos->x = -1;
	pos->y = -1;

	int prev_sumofdiff = TW_THRESHOLD;
	int x;
	int y;
	int offsetx;
	int offsety;

	// Performance enhancement --> instead of a sliding window that shifts by 1 pixel, shifts over by the size of the window. (not as smooth, but much faster since it goes through a lot less pixels)
	for (x = 0; x < hres - TW_XSTEP; x = x + TW_XSTEP)
	{
		for (y = 0; y <vres - TW_YSTEP; y = y + TW_YSTEP)
		{
			// reset intensity and sum of difference calculations
			int sumofdiff = 0;
			int intensity = 0;

			// iterate over the whole window
			for (offsetx = x;  offsetx < x + TW_XSTEP; offsetx++)
			{
				for (offsety = y;  offsety < y + TW_YSTEP; offsety++)
				{
					// Determine the RGB values for the pixel
					u32 scal = frame_ptr[offsety*hres + offsetx];

					// obtain the individual colour contributions
					u32 blue = (scal & TW_BLUE) >> TW_BLUE_OFFSET;
					u32 green = (scal & TW_GREEN) >> TW_GREEN_OFFSET;
					u32 red = (scal & TW_RED) >> TW_RED_OFFSET;

					// measures absolute difference of the blue component in the sliding window
					sumofdiff = sumofdiff + 255 - blue;

					// measure intensity of the whole sliding window
					intensity = intensity + blue + green + red;
				}
			}


			// Several checking factors:
			//  - Blue colour is dominant
			//  - Find the best match in the frame --> only 1
			//  - Intensity is high enough to be a light
			//  - Intensity isn't high enough to be a white light
			if ((sumofdiff < TW_THRESHOLD) && (sumofdiff < prev_sumofdiff) && (intensity > TW_INT_MIN) && (intensity < TW_INT_MAX))
			{
				// the center of the sliding window is the x,y coordinates
				prev_sumofdiff = sumofdiff;
				//pos->x = (int) ceil(x + TW_XSTEP/2);
				//pos->y = (int) ceil(y + TW_YSTEP/2);
				pos->x = x;
				pos->y = y;
			}
		}
	}
}

// This function is used to test the track wand algorithm
// by moving a small blue square across the screen and determining
// the position from the computer generated frame
int track_wand_test (u32* frame_ptr, Position *pos, Position *actual_pos, u32 hres, u32 vres)
{
	int x,y,i,j;
	int status = TW_PASS;
	u32 pixel_value = 0;

	for (x=0; x < hres - TW_BOX_SIZE; x++)
	{
		for (y=0; y < vres - TW_BOX_SIZE; y++)
		{
			// Generate frame with a small blue box at the specific location
			// Initialize DrawFrame - image to draw on top of VideoFrame
			for (j = 0; j < vres; j++)
			{
				for (i = 0; i < hres; i++)
				{
					if(i >= x && i < x + TW_BOX_SIZE && j >= y && j < y + TW_BOX_SIZE)
					{
						pixel_value = TW_BLUE;	// Yellow box
					} else
					{
						pixel_value = TW_WHITE;	// in a white frame
					}
					frame_ptr[j * hres + i] = pixel_value;
				}
			}

			// Determine the location of the wand at the specific location.
			track_wand (frame_ptr, pos, hres, vres);
			if ((pos->x > x + TW_BOX_SIZE) || (pos->x < x - TW_BOX_SIZE) || (pos->y > y + TW_BOX_SIZE) || (pos->y < y - TW_BOX_SIZE))
			{
				// failed track wand!
				status = TW_FAIL;
				actual_pos->x = x;
				actual_pos->y = y;
				break;
			}
		}
		if (status == TW_FAIL)
		{
			break;
		}
	}
	return status;
}

// This function is used to test the track wand algorithm
void track_wand_single_test (u32* frame_ptr, Position *pos, Position *actual_pos, u32 hres, u32 vres)
{
	int x,y,i,j;
	int status = TW_PASS;
	u32 pixel_value = 0;

	x = actual_pos->x;
	y = actual_pos->y;
	// Generate frame with a small blue box at the specific location
	// Initialize DrawFrame - image to draw on top of VideoFrame
	for (j = 0; j < vres; j++)
	{
		for (i = 0; i < hres; i++)
		{
			if(i >= x && i < x + TW_BOX_SIZE && j >= y && j < y + TW_BOX_SIZE)
			{
				pixel_value = TW_BRIGHT_BLUE;	// Yellow box
			} else
			{
				pixel_value = TW_BLACK;	// in a white frame
			}
			frame_ptr[j * hres + i] = pixel_value;
		}
	}

	// Determine the location of the wand at the specific location.
	track_wand (frame_ptr, pos, hres, vres);
}
