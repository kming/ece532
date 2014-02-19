/*
 * Frame Merge IP Project C Code
 * Wahid Rahman, Kei-Ming Kwong, Opal Densmore
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
#include "frame_merge.h"
#include "track_wand.h"

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
Frame_Merge Merge_IP;
Position pos;

int main() {


	// -- INITIALIZATIONS -- //
	volatile u32 *ddr_addr = (volatile u32 *) XPAR_S6DDR_0_S0_AXI_BASEADDR;
	frame_merge_Initialize (&Merge_IP, XPAR_FRAME_MERGE_0_BASEADDR);
	volatile int *hdmi_addr = (int *) XPAR_HDMI_OUT_0_BASEADDR;

	u32 pixel_value = 0x0;

	XStatus status;

	int i, j;
	int k;
	int DELAY = 10000;
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

	// Initialize HDMI-OUT IP Block
	hdmi_addr[0] = 1280; 									// hdmi_addr[0] corresponds to slv_reg2 --> Set Stride
	hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);		// Display Comp Frame
	hdmi_addr[2] = 0;										// Ensure Go signal is low

	printf ("Setup of HDMI-OUT complete\n");

	// -- FRAME INIT OPERATIONS --
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

	// Initialize VideoFrame (input video, simulate as columns of solid colour (Blue/Red/Green) from left to right)
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES/3; i++)
		{
			pixel_value = BLUE;
			ddr_addr[VIDEOFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
		for (i = HRES/3; i < 2*HRES/3; i++)
		{
			pixel_value = RED;
			ddr_addr[VIDEOFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
		for (i = 2*HRES/3; i < HRES; i++)
		{
			pixel_value = GREEN;
			ddr_addr[VIDEOFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
	}

	// Initialize CompositeFrame (output video = VideoFrame + DrawFrame. Initialize as black)
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			pixel_value = BLACK;
			ddr_addr[COMPFRAME_OFFSET + j * HRES + i] = pixel_value;
		}
	}
	printf ("Frame Initialization complete!\n");

	// -- TRACK WAND OPERATION --
	track_wand (ddr_addr, &pos, HRES, VRES);

	// -- MERGE OPERATIONS --
	frame_merge_sw (ddr_addr, (int)(DRAWFRAME_OFFSET), (int)(VIDEOFRAME_OFFSET), (int)(COMPFRAME_OFFSET), HRES, VRES);

	// Enable HDMI-OUT once merging is finished
	hdmi_addr[2] = 1;

	printf ("Frame Merge complete!\n");

	printf ("Enter (0/1/2) for (draw/video/composite) frames:\n");

	// Allow user to switch between draw, video, and comp frames
	// to output to screen
	input = 0;
	while(1)
	{
		XUartLite_Recv(&UartLite, &input, 1);	// Recv 1 byte (char) and store into buffer
		switch (input)
		{
		case '0':	// DrawFrame
			hdmi_addr[1] = (int)(ddr_addr + DRAWFRAME_OFFSET);
			break;
		case '1':	// VideoFrame
			hdmi_addr[1] = (int)(ddr_addr + VIDEOFRAME_OFFSET);
			break;
		case '2':	// CompFrame
			hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);
			break;
		default:
			hdmi_addr[1] = (int)(ddr_addr + COMPFRAME_OFFSET);
			break;
		}
	}

	printf("Exiting\n\r");

	return 0;
}
