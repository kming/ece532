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
Position tracked_pos;
Position actual_pos;

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
	hdmi_addr[1] = (int)(ddr_addr + DRAWFRAME_OFFSET);		// Display Comp Frame
	hdmi_addr[2] = 1;										// Ensure Go signal is low

	printf ("Setup of HDMI-OUT complete\n\r");

	// -- TRACK WAND OPERATION --
	printf ("Initiating Track Wand Test\r\n");
	actual_pos.x = 0;
	actual_pos.y = 0;
	track_wand_single_test (ddr_addr, &tracked_pos, &actual_pos, HRES, VRES);

	printf ("Position tracked %d, %d\r\n", tracked_pos.x, tracked_pos.y);
	printf ("Actual Position  %d, %d\r\n", actual_pos.x, actual_pos.y);

	printf ("Enter (4/6/2/8) to move box (Left/Right/Down/Up)\n\r");
	// Allow user to switch between draw, video, and comp frames
	// to output to screen
	input = 0;
	while(1)
	{
		XUartLite_Recv(&UartLite, &input, 1);	// Recv 1 byte (char) and store into buffer
		switch (input)
		{
		case '8':	// DrawFrame
			if (actual_pos.y > 0)
			{
				actual_pos.y = actual_pos.y - TW_BOX_SIZE;
				track_wand_single_test (ddr_addr, &tracked_pos, &actual_pos, HRES, VRES);
				printf ("Position tracked %d, %d\r\n", tracked_pos.x, tracked_pos.y);
				printf ("Actual Position  %d, %d\r\n", actual_pos.x, actual_pos.y);
			}
			break;
		case '4':	// VideoFrame
			if (actual_pos.x > 0)
			{
				actual_pos.x = actual_pos.x - TW_BOX_SIZE;
				track_wand_single_test (ddr_addr, &tracked_pos, &actual_pos, HRES, VRES);
				printf ("Position tracked %d, %d\r\n", tracked_pos.x, tracked_pos.y);
				printf ("Actual Position  %d, %d\r\n", actual_pos.x, actual_pos.y);
			}

			break;
		case '2':	// CompFrame
			if (actual_pos.y < VRES - 2*TW_BOX_SIZE)
			{
				actual_pos.y = actual_pos.y + TW_BOX_SIZE;
				track_wand_single_test (ddr_addr, &tracked_pos, &actual_pos, HRES, VRES);
				printf ("Position tracked %d, %d\r\n", tracked_pos.x, tracked_pos.y);
				printf ("Actual Position  %d, %d\r\n", actual_pos.x, actual_pos.y);
			}

			break;
		case '6':	// CompFrame
			if (actual_pos.x < HRES - 2*TW_BOX_SIZE)
			{
				actual_pos.x = actual_pos.x + TW_BOX_SIZE;
				track_wand_single_test (ddr_addr, &tracked_pos, &actual_pos, HRES, VRES);
				printf ("Position tracked %d, %d\r\n", tracked_pos.x, tracked_pos.y);
				printf ("Actual Position  %d, %d\r\n", actual_pos.x, actual_pos.y);
			}
			break;
		default:
			break;
		}
		input = ' ';
	}

	printf("Exiting\n\r");

	return 0;
}
