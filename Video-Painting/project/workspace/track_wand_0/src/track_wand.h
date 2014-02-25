/*
 * track_wand.h
 *
 *  Created on: 2014-02-19
 *      Author: Keiming
 */

#ifndef TRACK_WAND_H /* prevent circular inclusions */
#define TRACK_WAND_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

/************************** Constant Definitions ****************************/

#define TW_XSTEP 4								// horizontal window size
#define TW_YSTEP 4								// vertical window size
#define TW_TOL_PIXEL 30							// tolerance per pixel, decrease value to make it stricter
#define TW_INT_MIN_TH 150							// average minimum intensity per pixel/colour space (0-255)
#define TW_INT_MAX_TH 200							// average max intensity per pixel/colour space (0-255)

#define TW_NUM_PIXEL (TW_XSTEP*TW_YSTEP)			// number of pixels in the window
#define TW_THRESHOLD (TW_NUM_PIXEL*TW_TOL_PIXEL) 	// number of pixels in the window * the total tolerance per pixel
#define TW_INT_MIN (TW_NUM_PIXEL*3*TW_INT_MIN_TH)	// minimum intensity threshold, a minimum ensures that it is activated by a bright enough source, i.e a light, not a random object
#define TW_INT_MAX (TW_NUM_PIXEL*3*TW_INT_MAX_TH)	// maximum intensity threshold, a maximum to ensure it isn't a bright white light


// Defines how the pixel structure and colour space values are organized
#define TW_BRIGHT_BLUE 0x9696FF00
#define TW_BLUE 0x0000FF00
#define TW_BLUE_OFFSET 8
#define TW_GREEN 0x00FF0000
#define TW_GREEN_OFFSET 16
#define TW_RED 0xFF000000
#define TW_RED_OFFSET 24
#define TW_WHITE 0xFFFFFFFF
#define TW_BLACK 0x00000000

// Testing params
#define TW_BOX_SIZE 9
#define TW_PASS 1
#define TW_FAIL 1



/**************************** Type Definitions ******************************/

typedef struct {
	int x;
	int y;
} Position;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

void track_wand (u32 *frame_ptr, Position *pos, u32 hres, u32 vres);
int track_wand_test (u32* frame_ptr, Position *pos, Position *actual_pos, u32 hres, u32 vres);
void track_wand_single_test (u32* frame_ptr, Position *pos, Position *actual_pos, u32 hres, u32 vres);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

