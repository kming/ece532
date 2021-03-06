/*
 * Frame Merge IP Project C Code
 * Wahid Rahman
 * Jan. 30, 2014
 * ECE532 - Winter 2014
 * University of Toronto
 * (based on HDMI Video Out code by Victor Zhang, 2013)
 */

#include <stdio.h>
#include <stdbool.h>
#include "xparameters.h"
#include "xbasic_types.h"
#include "xutil.h"
#include "xuartlite.h"

#define printf xil_printf
#define NUM_TEST_WORDS 8
#define TEST_VECTOR 0x12345678 /* random word */

#define HRES 1280	// Pixels
#define VRES 720	// Pixels

// Define memory base address for 3 key frames (Draw, Video, Composite)
#define FRAME_SIZE_BYTES 0x400000	// 1280*720 pixels * 4 bytes/pixel = 0x384000 + some extra space
#define DRAWFRAME_OFFSET	0x0
#define VIDEOFRAME_OFFSET	(DRAWFRAME_OFFSET + FRAME_SIZE_BYTES)
#define COMPFRAME_OFFSET	(VIDEOFRAME_OFFSET + FRAME_SIZE_BYTES)


// Define Box for Frame1 (x1,y1), (x2,y2)
#define BOX_X0 	HRES/3
#define BOX_Y0 	VRES/3
#define BOX_X1 	2*HRES/3
#define BOX_Y1 	2*VRES/3

// Define colours
#define WHITE	0xFFFFFFFF		// R_G_B_(Don't care, assume 0 in SW)
#define BLACK	0x00000000
#define RED		0xFF000000
#define BLUE	0x0000FF00
#define GREEN	0x00FF0000
#define YELLOW	0xFFFF0000

#define DEBUG 1

XUartLite UartLite;

int main() {

	// -- INITIALIZATIONS -- //
	volatile u32 *ddr_addr = (volatile u32 *) XPAR_S6DDR_0_S0_AXI_BASEADDR;
	volatile u32 *frame_merge_addr = (volatile u32 *) XPAR_FRAME_MERGE_0_BASEADDR;
	volatile int *hdmi_addr = (int *) XPAR_HDMI_OUT_0_BASEADDR;
	volatile int *zero_addr = (volatile u32 *) 0x00000000;

	u32 pixel_value = 0x0;
	u32 draw_frame_pixel = 0x0;
	u32 video_frame_pixel = 0x0;
	u32 comp_frame_pixel = 0x0;

	XStatus status;

	int i, j;
	//int k;
	//int DELAY = 10000;
	char input;

	status = XUtil_MemoryTest32((u32 *) ddr_addr,
								NUM_TEST_WORDS,
								TEST_VECTOR,
								XUT_ALLMEMTESTS);
	if (XST_SUCCESS != status)
	{
		printf("Error - Memory self test failed. Exiting...\r\n");
		return -1;
	}

	status = XUartLite_Initialize(&UartLite, XPAR_UARTLITE_1_DEVICE_ID);
	if (XST_SUCCESS != status)
	{
		printf("Error - UartLite self test failed. Exiting...\r\n");
		return -1;
	}

	// -- FRAME OPERATIONS --

	// Set slave registers for hdmi_out custom pcore
	// set stride
	hdmi_addr[0] = 1280; // hdmi_addr[0] corresponds to slv_reg2
	// set frame base address
	hdmi_addr[1] = (int)ddr_addr; // hdmi_addr[1] corresponds to slv_reg1
	// go
	hdmi_addr[2] = 1;

	// Set slave registers for frame_merge custom pcore
	//--frame_overlay_addr[0] = (int)(ddr_addr + DRAWFRAME_OFFSET);	// Base addr for target frame in memory
	//--frame_overlay_addr[1] = 1; 		// Go register for IP
#ifdef DEBUG
	//printf("DEBUG: frame slavereg0 = %x\n\r", frame_overlay_addr[0]);	// DEBUG
#endif

	// Initialize DrawFrame - image to draw on top of VideoFrame
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			if(i >= BOX_X0 && i < BOX_X1 && j >= BOX_Y0 && j < BOX_Y1)
			{
				pixel_value = YELLOW;	// Yellow box
			} else
			{
				pixel_value = WHITE;	// in a white frame
			}
			ddr_addr[DRAWFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
	}

	// Initialize VideoFrame (input video, simulate as constant blue)
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			pixel_value = BLUE;
			ddr_addr[VIDEOFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
	}

	// Initialize CompositeFrame (output video = VideoFrame + DrawFrame. Initialize as black)
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			//pixel_value = BLACK;
			pixel_value = GREEN;
			ddr_addr[COMPFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
	}

	hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);		// Display Frame1, not Frame0

	// -- MERGE FRAMES
	// Read pixel from DrawFrame and VideoFrame.
	// If pure white, white VideoFrame pixel to CompFrame.
	// else write DrawFrame pixel to CompFrame.
	// Repeat while pixels in (DrawFrame, VideoFrame)
/*	for (j = 0; j < VRES; j++)
	{
		for(i = 0; i < HRES; i++)
		{
			// 1) Read DrawFrame and VideoFrame from DDR memory
			draw_frame_pixel = ddr_addr[DRAWFRAME_OFFSET + j*HRES + i];
			video_frame_pixel = ddr_addr[VIDEOFRAME_OFFSET + j*HRES + i];

			// 2) Evaluate new pixel
			if (WHITE != draw_frame_pixel)
			{
				comp_frame_pixel = draw_frame_pixel;
			}
			else
			{
				comp_frame_pixel = video_frame_pixel;
			}

			// 3) Write new pixel to DDR memory
			ddr_addr[COMPFRAME_OFFSET + j*HRES + i] = comp_frame_pixel;
		}
	}*/
	printf("before zero = %x\n\r", zero_addr[0]);
	printf("after zero = %x\n\r", zero_addr[0]);
	// Allow user to switch between draw, video, and comp frames
	// to output to screen
	input = 0;
	frame_merge_addr[0] = 1;
	while(1)
	{
		XUartLite_Recv(&UartLite, &input, 1);	// Recv 1 byte (char) and store into buffer
		switch (input)
		{
		case '0':	// DrawFrame
			//printf("frame_merge[0] = %x\n\r", frame_merge_addr[0]);
			hdmi_addr[1] = (int)(ddr_addr + DRAWFRAME_OFFSET);
			break;
		case '1':	// VideoFrame
			hdmi_addr[1] = (int)(ddr_addr + VIDEOFRAME_OFFSET);
			break;
		case '2':	// CompFrame
			hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);
/*
			for (j = 0; j < VRES; j++)
			{
				for (i = 0; i < HRES; i++)
				{
					pixel_value = BLACK;
					printf("CompFrame[%d][%d] = %x\r\n", i, j, ddr_addr[COMPFRAME_OFFSET + j * HRES + i]);
				}
			}
*/
			break;
		default:
			hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);
			break;
		}
	}

	printf("Exiting\n\r");

	return 0;
}
