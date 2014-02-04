
#ifndef FRAME_MERGE_H /* prevent circular inclusions */
#define FRAME_MERGE_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

/************************** Constant Definitions ****************************/
#define FM_FALSE 0
#define FM_TRUE 1
#define FM_WHITE 0xFFFFFFFF
/**************************** Type Definitions ******************************/

typedef struct {
	/* Base Address location */
	volatile u32* BaseAddr;
	
	/* Frame Memory Addresses */
	u32 DrawFrameAddr;
	u32 VideoFrameAddr;
	u32 CompFrameAddr;
	
	/* Status bit */
	u8 isReady;
	u8 isRunning;
} Frame_Merge;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

void frame_merge_Initialize(Frame_Merge *InstancePtr, u32 Base_Addr);
void frame_merge_SetAddr(Frame_Merge *InstancePtr, u32 Draw_Addr, u32 Video_Addr, u32 Comp_Addr);
void frame_merge_Go(Frame_Merge *InstancePtr);
void frame_merge_Stop(Frame_Merge *InstancePtr);

/* pure SW algorithm to perform merge instead of hardware */
void frame_merge_sw(u32* ddr_Addr, u32 Draw_Addr, u32 Video_Addr, u32 Comp_Addr, u32 hres, u32 vres);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

