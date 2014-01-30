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

#define printf xil_printf
#define NUM_TEST_WORDS 8
#define TEST_VECTOR 0x12345678 /* random word */

#define HRES 1280	// Pixels
#define VRES 720	// Pixels

#define FRAME_SIZE_BYTES 0x400000	// 1280*720 pixels * 4 bytes/pixel = 0x384000 + some extra space
#define FRAME0_OFFSET	0x0
#define FRAME1_OFFSET	(FRAME0_OFFSET + FRAME_SIZE_BYTES)

#define DEBUG 1

int main() {

	// -- INITIALIZATIONS -- //
	volatile u32 *ddr_addr = (volatile u32 *) XPAR_S6DDR_0_S0_AXI_BASEADDR;
	volatile u32 *frame_overlay_addr = (volatile u32 *) XPAR_FRAME_MERGE_0_BASEADDR;
	volatile int *hdmi_addr = (int *) XPAR_HDMI_OUT_0_BASEADDR;

	XStatus status;

	int i, j, k;
	int DELAY = 10000;

	status = XUtil_MemoryTest32((u32 *) ddr_addr,
								NUM_TEST_WORDS,
								TEST_VECTOR,
								XUT_ALLMEMTESTS);
	if (XST_SUCCESS != status)
	{
		printf("Error - Memory self test failed. Exiting...\r\n");
		return -1;
	}

	/*
	printf(
			"32-bit test: %s\n\r",
			(XST_SUCCESS == XUtil_MemoryTest32((u32 *)ddr_addr, NUM_TEST_WORDS,
					TEST_VECTOR, XUT_ALLMEMTESTS)) ? "passed" : "failed");
	 */


	// -- FRAME OPERATIONS --

	// Set slave registers for hdmi_out custom pcore
	// set stride
	hdmi_addr[0] = 1280; // hdmi_addr[0] corresponds to slv_reg2
	// set frame base address
	hdmi_addr[1] = (int)ddr_addr; // hdmi_addr[1] corresponds to slv_reg1
	// go
	hdmi_addr[2] = 1;

	// Set slave registers for frame_merge custom pcore
	frame_overlay_addr[0] = (int)(ddr_addr + FRAME1_OFFSET);	// Base addr for target frame in memory
	frame_overlay_addr[1] = 1; 		// Go register for IP
#ifdef DEBUG
	printf("DEBUG: frame slavereg0 = %x\n\r", frame_overlay_addr[0]);	// DEBUG
#endif

	// Initialize Frame 0
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			ddr_addr[FRAME0_OFFSET + j * HRES + i] = 0xffffffff;	// Pure white frame
		}
	}

	// Initialize Frame 1
	for (j = 0; j < VRES; j++)
	{
		for (i = 0; i < HRES; i++)
		{
			ddr_addr[FRAME1_OFFSET + j * HRES + i] = 0x0000ff00;	// Pure blue frame
		}
	}


	hdmi_addr[1] = (int)(ddr_addr + FRAME1_OFFSET);		// Display Frame1, not Frame0
	int max = hdmi_addr[1] + FRAME_SIZE_BYTES;	// word addressable

	// Currently just prints out Frame1 on top of Frame2 for max bytes (max/4 pixels)... -wr
	while(1)
	{
		if(frame_overlay_addr[0] <= max)
		{
			frame_overlay_addr[0] += 128;	// Move write pointer down 128 pixels (16 pixels)
		}
		for (k=0;k<DELAY;k++);
	}

	printf("Exiting\n\r");

	return 0;
}
