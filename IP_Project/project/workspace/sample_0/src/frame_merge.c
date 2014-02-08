/*
 * Frame Merge API source
 * Wahid Rahman, Kei-Ming Kwong, Opal Densmore
 * Feb 07, 2014
 * ECE532 - Winter 2014
 * University of Toronto
 *
 */

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xbasic_types.h"
#include "xutil.h"
#include "xil_assert.h"
#include "frame_merge.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

void frame_merge_Initialize(Frame_Merge *InstancePtr, u32 Base_Addr)
{
	/* Assures IP ptr is valid */
	Xil_AssertNonvoid(InstancePtr != NULL);	
	
	/* Initializes default Frame values */
	InstancePtr->DrawFrameAddr 	= 0;
	InstancePtr->VideoFrameAddr = 0;
	InstancePtr->CompFrameAddr 	= 0;
	
	/* Since Frame Addresses are not assigned a value, the IP is not ready */
	InstancePtr->isReady = FM_FALSE; 
	InstancePtr->isRunning = FM_FALSE; 
	
	/* Setup Base Address for IP */
	InstancePtr->BaseAddr = (volatile u32*) Base_Addr;
}

void frame_merge_SetAddr(Frame_Merge *InstancePtr, u32 Draw_Addr, u32 Video_Addr, u32 Comp_Addr)
{
	/* We would want some kind of check to ensure we dont rewrite over the control registers while in the middle of merging? */
	
	/* Assures IP ptr is valid */
	Xil_AssertNonvoid(InstancePtr != NULL);	
	
	/* Cannot change address if the IP is running */
	Xil_AssertNonvoid(InstancePtr->isRunning != FM_TRUE);
	
	/* Asserts to make sure Input Addresses are not the same as output address */
	Xil_AssertNonvoid(Draw_Addr != Comp_Addr);
	Xil_AssertNonvoid(Video_Addr != Comp_Addr);
	
	/* Sets Struct Frame Address values */
	InstancePtr->DrawFrameAddr 	= Draw_Addr;
	InstancePtr->VideoFrameAddr = Video_Addr;
	InstancePtr->CompFrameAddr 	= Comp_Addr;
	
	/* Initialize IP with Addresses */
	InstancePtr->BaseAddr[1] = InstancePtr->DrawFrameAddr;
	InstancePtr->BaseAddr[2] = InstancePtr->VideoFrameAddr;
	InstancePtr->BaseAddr[3] = InstancePtr->CompFrameAddr;
	
	/* Set the isReady Flag */ 
	InstancePtr->isReady = FM_TRUE; 
}

void frame_merge_Go(Frame_Merge *InstancePtr)
{
	/* Assures IP ptr is valid */
	Xil_AssertNonvoid(InstancePtr != NULL);	
	
	/* Cannot run again if the IP is running */
	Xil_AssertNonvoid(InstancePtr->isRunning != FM_TRUE);
	/* Check if IP is Ready before running */
	Xil_AssertNonvoid(InstancePtr->isReady == FM_TRUE);
	
	/* Set IP to run */
	InstancePtr->isRunning = FM_TRUE;
	InstancePtr->BaseAddr[0] = FM_TRUE;
}

void frame_merge_Stop(Frame_Merge *InstancePtr)
{
	/* Assures IP ptr is valid */
	Xil_AssertNonvoid(InstancePtr != NULL);	
	
	/* Set IP to stop */
	InstancePtr->isRunning = FM_FALSE;
	InstancePtr->BaseAddr[0] = FM_FALSE;
}


/* Software Variant of frame merging algorithm, doesn't make use of IP.  Up to the user to ensure it won't conflict with the hardware block during operation */
void frame_merge_sw(u32* ddr_Addr, u32 Draw_Offset, u32 Video_Offset, u32 Comp_Offset, u32 hres, u32 vres)
{
	int i, j;
	u32 draw_frame_pixel, video_frame_pixel, comp_frame_pixel;
	// Increment Rows
	for (j = 0; j < vres; j++)
	{
		// Increment Columns
		for(i = 0; i < hres; i++)
		{
			// 1) Read DrawFrame and VideoFrame Pixel(i,j) from DDR memory
			draw_frame_pixel = ddr_Addr[Draw_Offset + j*hres + i];
			video_frame_pixel = ddr_Addr[Video_Offset + j*hres + i];

			// 2) Evaluate new pixel
			if (FM_WHITE != draw_frame_pixel) // If white pixel, ignore
			{
				comp_frame_pixel = draw_frame_pixel;
			}
			else							  // Overlay with pixel
			{
				comp_frame_pixel = video_frame_pixel;
			}

			// 3) Write new pixel to DDR memory
			ddr_Addr[Comp_Offset + j*hres + i] = comp_frame_pixel;
		}
	}
	
}
