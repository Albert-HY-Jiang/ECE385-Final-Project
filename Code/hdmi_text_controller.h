#ifndef HDMI_TEXT_CONTROLLER_H
#define HDMI_TEXT_CONTROLLER_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xparameters.h"

//#define COLUMNS 80
//#define ROWS 30

//define some colors
#define WIDTH 8
#define REMOVED 7
#define NUMELEMENTS 5
#define LTILE 6

/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a HDMI_TEXT_CONTROLLER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the HDMI_TEXT_CONTROLLERdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void HDMI_TEXT_CONTROLLER_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define HDMI_TEXT_CONTROLLER_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a HDMI_TEXT_CONTROLLER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the HDMI_TEXT_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 HDMI_TEXT_CONTROLLER_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define HDMI_TEXT_CONTROLLER_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the HDMI_TEXT_CONTROLLER instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
 
struct HDMI_TEXT_STRUCT {
	uint8_t  VRAM [72];
	uint32_t CTRL;
	uint32_t SCORE;
	uint32_t FRAME;
	uint32_t LASER;
	uint32_t START;
};

//you may have to change this line depending on the memory map of your block design
static volatile struct HDMI_TEXT_STRUCT* hdmi_ctrl = XPAR_HDMI_TEXT_CONTROLLER_0_AXI_BASEADDR;

void hdmiSetColor(int background, int foreground);
void hdmiClr();
void iniTile();
void getTiles(uint8_t * tiles);
void pushTiles(uint8_t * tiles);
void setTile(uint8_t tile, uint8_t offset, uint8_t color);
void setLaser(uint8_t on, uint8_t row, uint32_t offset, uint8_t spin);
void switchTile(uint8_t a, uint8_t b, uint8_t* arr);
void hdmiTestWeek1(uint8_t key); //Call this function from your main function for your demo.
uint8_t checkMatches(uint8_t* arr);
uint8_t checkMatchesSimple(uint8_t* arr);
void fillTiles(uint8_t* arr);
void printTiles(uint8_t* arr);
void addScore(uint8_t score);
void aniFill(uint8_t* arr);
void refresh(uint8_t* arr);
uint8_t empty(uint8_t* arr);

#endif // HDMI_TEXT_CONTROLLER_H
