/*
 * track_wand.c
 *
 *  Created on: 2014-02-19
 *      Author: Keiming
 */

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xbasic_types.h"
#include "xutil.h"
#include "xil_assert.h"
#include "track_wand.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/
void track_wand (u32 *frame_addr, Position *pos, u32 hres, u32 vres)
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
					int scal = frame_addr[offsety*hres + offsetx];
					// measures absolute difference of the blue component in the sliding window
					sumofdiff = sumofdiff + 255 - (scal&0xFF);

					// measure intensity of the whole sliding window
					//intensity = intensity + scal.val[0] + scal.val[1] + scal.val[2];
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
