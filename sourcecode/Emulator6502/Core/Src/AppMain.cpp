/**********************************************************************************
* MIT License																	  *
*																				  *
* Copyright(c) 2021 Mrx42Code                                                     *
* https://github.com/Mrx42Code                           				          *
*																				  *
* Permission is hereby granted, free of charge, to any person obtaining a copy    *
* of this softwareand associated documentation files(the "Software"), to deal	  *
* in the Software without restriction, including without limitation the rights	  *
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell	  *
* copies of the Software, and to permit persons to whom the Software is			  *
* furnished to do so, subject to the following conditions :						  *
*																				  *
* The above copyright noticeand this permission notice shall be included in all   *
* copies or substantial portions of the Software.								  *
*																				  *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,		  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE	  *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER		  *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE	  *
* SOFTWARE.																		  *
 **********************************************************************************/

#include "AppMain.h"
#include "main.h"
#include "iwdg.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "MC_Hardware6502.h"

extern MC_Hardware6502 mc_Hardware6502;
//-----------------------------------------------------------------------------
// Name: AppMainInit()
//-----------------------------------------------------------------------------
void AppMainInit()
{

}
//-----------------------------------------------------------------------------
// Name: AppMainSetup()
//-----------------------------------------------------------------------------
void AppMainSetup()
{
	HAL_Delay(5000);
	HAL_GPIO_WritePin(PCB_LED_GPIO_Port, PCB_LED_Pin, GPIO_PIN_SET);

    mc_Hardware6502.Initialize();
    mc_Hardware6502.Create();
    mc_Hardware6502.CpuSetParameters();

}
//-----------------------------------------------------------------------------
// Name: AppMainLoop()
//-----------------------------------------------------------------------------
void AppMainLoop()
{
	if(!mc_Hardware6502.m_Quit) {
		mc_Hardware6502.CpuMainLoop();
	}
	HAL_IWDG_Refresh(&hiwdg);

}
//-----------------------------------------------------------------------------
// Name: AppMainUSBCallBackRx(uint8_t* Buf, uint32_t *Len)
//-----------------------------------------------------------------------------
void AppMainUSBCallBackRx(uint8_t* Buf, uint32_t *Len)
{
	if(*Len > 0 ) {

	}
}

