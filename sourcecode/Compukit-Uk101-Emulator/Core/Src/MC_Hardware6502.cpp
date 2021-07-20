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

 //-----------------------------------------------------------------------------
 // File: MC_Hardware6502.cpp: implementation of the MC_Hardware6502 class.
 //-----------------------------------------------------------------------------
#include "MC_Hardware6502.h"
#include "MC_Processor6502.h"
#include "MC_FileSystem6502.h"

using namespace std;

//*****************************************************************************  
// Public Code
//*****************************************************************************
static const uint8_t m_KeyboardOutTable[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

// chr, row, col, Lshift, Rshift, caplock, ctrl
static const uint8_t m_KeyboardCodeTable[] = {
        '1', 7, 7, 0, 0, 0, 0,  '2', 7, 6, 0, 0, 0, 0,  '3', 7, 5, 0, 0, 0, 0,  '4', 7, 4, 0, 0, 0, 0,  '5', 7, 3, 0, 0, 0, 0,  '6', 7, 2, 0, 0, 0, 0,  '7', 7, 1, 0, 0, 0, 0,
        '!', 7, 7, 0, 1, 0, 0,  '"', 7, 6, 0, 1, 0, 0,  '#', 7, 5, 0, 1, 0, 0,  '$', 7, 4, 0, 1, 0, 0,  '%', 7, 3, 0, 1, 0, 0,  '&', 7, 2, 0, 1, 0, 0, 0x27, 7, 1, 0, 1, 0, 0, // '

        '8', 6, 7, 0, 0, 0, 0,  '9', 6, 6, 0, 0, 0, 0,  '0', 6, 5, 0, 0, 0, 0,  ':', 6, 4, 0, 0, 0, 0,  '-', 6, 3, 0, 0, 0, 0, 0x7F, 6, 2, 0, 0, 0, 0, // "Del"
        '(', 6, 7, 0, 1, 0, 0,  ')', 6, 6, 0, 1, 0, 0,  '*', 6, 4, 0, 1, 0, 0,  '=', 6, 3, 0, 1, 0, 0,  '.', 5, 7, 0, 0, 0, 0,

        'L', 5, 6, 0, 0, 1, 0,  'O', 5, 5, 0, 0, 1, 0, 0x09, 5, 4, 0, 0, 1, 0, 0x0D, 5, 3, 0, 0, 0, 0, // CR
        'l', 5, 6, 0, 0, 0, 0,  'o', 5, 5, 0, 0, 0, 0,
        '>', 5, 7, 0, 1, 0, 0, 0x5C, 5, 6, 0, 1, 1, 0, // Backward Slash

        'W', 4, 7, 0, 0, 1, 0,  'E', 4, 6, 0, 0, 1, 0,  'R', 4, 5, 0, 0, 1, 0,  'T', 4, 4, 0, 0, 1, 0,  'Y', 4, 3, 0, 0, 1, 0,  'U', 4, 2, 0, 0, 1, 0,  'I', 4, 1, 0, 0, 1, 0,
        'w', 4, 7, 0, 0, 0, 0,  'e', 4, 6, 0, 0, 0, 0,  'r', 4, 5, 0, 0, 0, 0,  't', 4, 4, 0, 0, 0, 0,  'y', 4, 3, 0, 0, 0, 0,  'u', 4, 2, 0, 0, 0, 0,  'i', 4, 1, 0, 0, 0, 0,

        'S', 3, 7, 0, 0, 1, 0,  'D', 3, 6, 0, 0, 1, 0,  'F', 3, 5, 0, 0, 1, 0,  'G', 3, 4, 0, 0, 1, 0,  'H', 3, 3, 0, 0, 1, 0,  'J', 3, 2, 0, 0, 1, 0,  'K', 3, 1, 0, 0, 1, 0,
        's', 3, 7, 0, 0, 0, 0,  'd', 3, 6, 0, 0, 0, 0,  'f', 3, 5, 0, 0, 0, 0,  'g', 3, 4, 0, 0, 0, 0,  'h', 3, 3, 0, 0, 0, 0,  'j', 3, 2, 0, 0, 0, 0,  'k', 3, 1, 0, 0, 0, 0,
        0x0A, 3, 2, 0, 0, 1, 1, '[', 3, 1, 1, 0, 1, 0,

        'X', 2, 7, 0, 0, 1, 0,  'C', 2, 6, 0, 0, 1, 0,  'V', 2, 5, 0, 0, 1, 0,  'B', 2, 4, 0, 0, 1, 0,  'N', 2, 3, 0, 0, 1, 0,  'M', 2, 2, 0, 0, 1, 0,  ',', 2, 1, 0, 0, 0, 0,
        'x', 2, 7, 0, 0, 0, 0,  'c', 2, 6, 0, 0, 0, 0,  'v', 2, 5, 0, 0, 0, 0,  'b', 2, 4, 0, 0, 0, 0,  'n', 2, 3, 0, 0, 0, 0,  'm', 2, 2, 0, 0, 0, 0,
        0x03, 2, 6, 0, 0, 0, 1, ']', 2, 2, 1, 0, 1, 0,  '<', 2, 1, 0, 1, 0, 0,

        'Q', 1, 7, 0, 0, 1, 0,  'A', 1, 6, 0, 0, 1, 0,  'Z', 1, 5, 0, 0, 1, 0,  ' ', 1, 4, 0, 0, 0, 0,  '/', 1, 3, 0, 0, 0, 0,  ';', 1, 2, 0, 0, 0, 0,  'P', 1, 1, 0, 0, 1, 0,
        'q', 1, 7, 0, 0, 0, 0,  'a', 1, 6, 0, 0, 0, 0,  'z', 1, 5, 0, 0, 0, 0,  'p', 1, 1, 0, 0, 0, 0,  '?', 1, 3, 0, 1, 0, 0,  '+', 1, 2, 0, 1, 0, 0,  '@', 1, 1, 1, 0, 1, 0,

        0x7B, 0, 1, 1, 0, 0, 0,  0x7D, 0, 2, 1, 0, 0, 0
};

//-----------------------------------------------------------------------------
// IMPLEMENT_DYNCREATE
//-----------------------------------------------------------------------------
MC_Hardware6502 mc_Hardware6502;
extern MC_FileSystem6502 mc_FileSystem6502;

extern IWDG_HandleTypeDef hiwdg;
uint8_t CpuMemoryRead(uint16_t address);
void CpuMemoryWrite(uint16_t address, uint8_t value);

//-----------------------------------------------------------------------------
// Name: CpuMemoryRead(uint16_t address)
//-----------------------------------------------------------------------------
uint8_t CpuMemoryRead(uint16_t address)
{
    return mc_Hardware6502.CpuMemoryMapRead(address);
}
//-----------------------------------------------------------------------------
// Name: CpuMemoryWrite(uint16_t address, uint8_t value)
//-----------------------------------------------------------------------------
void CpuMemoryWrite(uint16_t address, uint8_t value)
{
    mc_Hardware6502.CpuMemoryMapWrite(address, value);
}

MC_Processor6502 mc_Processor6502(CpuMemoryRead, CpuMemoryWrite);

//-----------------------------------------------------------------------------
// message handlers
//-----------------------------------------------------------------------------

//-Public----------------------------------------------------------------------
// Name: MC_Hardware6502()
// Desc: MC_Hardware6502 class
//-----------------------------------------------------------------------------
MC_Hardware6502::MC_Hardware6502()
{
    PrintStatus(false, "Console Create");
}
//-Public----------------------------------------------------------------------
// Name: ~MC_Hardware6502()
// Desc: ~MC_Hardware6502 Destruction class
//-----------------------------------------------------------------------------
MC_Hardware6502::~MC_Hardware6502()
{

}
//-Public----------------------------------------------------------------------
// Name: Initialize()
//-----------------------------------------------------------------------------
void MC_Hardware6502::Initialize()
{
    PrintStatus(false, "Hardware Initialize");
}
//-Public----------------------------------------------------------------------
// Name: Destroy()
//-----------------------------------------------------------------------------
void MC_Hardware6502::Destroy()
{
    PrintStatus(false, "Hardware Destroy");
}
//-Public----------------------------------------------------------------------
// Name: Create()
//-----------------------------------------------------------------------------
void MC_Hardware6502::Create()
{
    PrintStatus(false, "Hardware Create");
    CpuInitializeAndReset();
    m_Disassembler6502 = false;
}
//-Public----------------------------------------------------------------------
// Name: PrintStatus(bool Error, std::string Msg)
//-----------------------------------------------------------------------------
void MC_Hardware6502::PrintStatus(bool Error, std::string Msg)
{
    if (Error) {
        printf("\u001b[31;1m[Error]\u001b[0m ( \u001b[33;1m%s\u001b[0m )\r\n", Msg.c_str());
    } else {
        printf("\u001b[32;1m[ Ok  ]\u001b[0m ( \u001b[33;1m%s\u001b[0m )\r\n", Msg.c_str());
    }
}
//-Public----------------------------------------------------------------------
// Name: CpuSetParameters()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuSetParameters()
{
    PrintStatus(false, "Cpu SetParameters");
#if CompukitUK101

#else
	#if BreakPointMemory
		CpuSetBreakPointMemory(true, 0x0046);
	#endif
	#if BreakPointOpCode
		CpuSetBreakPointOpCode(true, 0x3469);                                            // if you get here everything went well 3469 : 4c6934 > jmp*; test passed, no errors
	#endif
		CpuSetPC(0x0400);
#endif
    m_Disassembler6502 = false;
    m_Cpu6502Step = false;
    m_Cpu6502Run = true;
}
//-Public----------------------------------------------------------------------
// Name: SetBreakPointOpCode(bool Enable, uint16_t Address)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuSetBreakPointOpCode(bool Enable, uint16_t Address)
{
    char buf[256];

    m_BreakPointOpCode.SetFlag = Enable;
    m_BreakPointOpCode.Address = Address;
    m_BreakPointOpCode.Found = false;
    if (m_BreakPointOpCode.SetFlag) {
        snprintf(buf, sizeof(buf), "Cpu Set BreakPoint OpCode $%04X", Address);
        PrintStatus(false, buf);
    }
}
//-Public----------------------------------------------------------------------
// Name: SetBreakPointMemory(bool Enable, uint16_t Address)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuSetBreakPointMemory(bool Enable, uint16_t Address)
{
    char buf[256];

    m_BreakPointMemory.SetFlag = Enable;
    m_BreakPointMemory.Address = Address;
    m_BreakPointMemory.Found = false;
    if (m_BreakPointMemory.SetFlag) {
        snprintf(buf, sizeof(buf), "Cpu Set BreakPoint Memory $%04X", Address);
        PrintStatus(false, buf);
    }
}
//-Public----------------------------------------------------------------------
// Name: SetPC(uint16_t PC)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuSetPC(uint16_t PC)
{
    char buf[256];

    snprintf(buf, sizeof(buf), "Cpu Set PC $%04X", PC);
    PrintStatus(false, buf);
    mc_Processor6502.SetPC(0x0400);
}
//-Public----------------------------------------------------------------------
// Name: CpuIRQ()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuIRQ()
{
    if (m_Cpu6502Run) {
        PrintStatus(false, "Cpu IRQ");
        mc_Processor6502.IRQ();
    }
}
//-Public----------------------------------------------------------------------
// Name: CpuNMI()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuNMI()
{
    if (m_Cpu6502Run) {
        PrintStatus(false, "Cpu NMI");
        mc_Processor6502.NMI();
    }
}
//-Public----------------------------------------------------------------------
// Name: CpuInitializeAndReset()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuInitializeAndReset()
{
    MemoryInit();
    MemoryLoadProgramFiles();
    mc_Processor6502.Reset();
    PrintStatus(false, "Cpu Initialize And Reset");
    CpuRun();
}
//-Public----------------------------------------------------------------------
// Name: CpuReset()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuReset()
{
    bool Cpu6502Run = m_Cpu6502Run;
    m_Cpu6502Run = false;
    mc_Processor6502.Reset();
    PrintStatus(false, "Cpu Reset");
    if (Cpu6502Run) {
        PrintStatus(false, "Cpu Run");
    } else {
        PrintStatus(false, "Cpu Stop");
    }
    m_Cpu6502Run = Cpu6502Run;
}
//-Public----------------------------------------------------------------------
// Name: CpuStop()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuStop()
{
    m_Cpu6502Step = false;
    m_Cpu6502Run = false;
    PrintStatus(false, "Cpu Stop");
}
//-Public----------------------------------------------------------------------
// Name: CpuRun()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuRun()
{
    PrintStatus(false, "Cpu Run");
    m_Cpu6502Step = false;
    m_Cpu6502Run = true;
}
//-Public----------------------------------------------------------------------
// Name: CpuStep()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuStep()
{
    m_Cpu6502Run = false;
    m_Cpu6502Step = true;
}
//-Public----------------------------------------------------------------------
// Name: CpuMainLoop()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMainLoop()
{
#if CompukitUK101
	CpuMainLoopCompukitUk101();
#else
		CpuMainLoopFunctionalTest6502();
#endif
}
#if CompukitUK101
//-Public----------------------------------------------------------------------
// Name: CpuMainLoopCompukitUk101()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMainLoopCompukitUk101()
{
	uint32_t Crc;
	static uint8_t LastKbChar = 0;

#if BreakPointMemory
	m_BreakPointMemory.Found = false;
#endif
	if (m_Cpu6502Run) {
#if BreakPointOpCode
		if (TestForBreakPointOpCode()) {
		   m_Cpu6502Run = false;
           m_Disassembler6502 = true;
        }
#endif
        if (mc_Processor6502.RunOneOp()) {
            m_Cpu6502Run = false;                                           // Cpu Crash (Stop Cpu + Memory Dump + Debug Info)
            mc_Processor6502.DebugCrashInfo(m_MemoryMap);
        } else {
            if (m_Disassembler6502) {
            	mc_Processor6502.DebugInfo(m_MemoryMap);
            }
#if BreakPointMemory
            if (!m_Disassembler6502 && m_BreakPointMemory.Found) {
            	mc_Processor6502.DebugInfo(m_MemoryMap);
            }
#endif
        }
	} else if (m_Cpu6502Step) {
		m_Cpu6502Step = false;
#if BreakPointOpCode
		if (TestForBreakPointOpCode()) {
		   m_Disassembler6502 = true;
		}
#endif
		if (mc_Processor6502.RunOneOp()) {
			mc_Processor6502.DebugCrashInfo(m_MemoryMap);
		} else {
			mc_Processor6502.DebugInfo(m_MemoryMap);
		}
	}
	HAL_IWDG_Refresh(&hiwdg);
	if(m_Screen.Update) {
		m_Screen.Update = false;
		m_Screen.TimeOut = 	HAL_GetTick() + 100;
		if(m_VideoRamUpdate || m_VideoRamCrc == 0) {
			m_VideoRamUpdate = false;
			Crc = mc_FileSystem6502.Crc32(&m_MemoryMap[MemoryVideoAddress], MemoryVideoSizeAddress);
			if(m_VideoRamCrc != Crc) {
				m_VideoRamCrc = Crc;
				printf("\033[2J");
				printf("\033[0;0H");
				PrintVideoRam(&m_MemoryMap[MemoryVideoAddress], MemoryVideoSizeAddress);
#if CompukitUK101_DebugKeypress
				printf("Debug Keypress = %02X\r\n", LastKbChar);
#endif
			}
		}
	} else {
		if(HAL_GetTick() > m_Screen.TimeOut) {
			m_Screen.Update = true;
			LastKbChar = m_KbChar;
			KeyboardMapKey(LastKbChar);
			if (m_KbChar == 0x60) {
				CpuReset();
			}
			m_KbChar = 0;
		}
	}
}
#else
//-Public----------------------------------------------------------------------
// Name: CpuMainLoopFunctionalTest6502()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMainLoopFunctionalTest6502()
{
    uint32_t OpCounter = 0;
    uint32_t TotalCycles = 0;
    uint32_t StartTime = 0;
    uint32_t EndTime = 0;
    uint32_t TotalTime = 0;
    double CpuClk = 0.0;
    char StatusMsg[256];

    StatusMsg[0] = 0;
    StartTime = HAL_GetTick();
    while (!m_Quit) {
#if BreakPointMemory
        m_BreakPointMemory.Found = false;
#endif
        if (m_Cpu6502Run) {
#if BreakPointOpCode
            if (TestForBreakPointOpCode()) {
                m_Cpu6502Run = false;
                m_Disassembler6502 = true;
                PrintStatus(false, "If you get here everything went well [$3469  4C 6934  JMP $3469] test passed, no errors");
                m_Quit = true;
            }
#endif
            if (mc_Processor6502.RunOneOp()) {
                m_Cpu6502Run = false;                                           // Cpu Crash (Stop Cpu + Memory Dump + Debug Info)
                mc_Processor6502.DebugCrashInfo(m_MemoryMap);
                m_Quit = true;
            } else {
                if (m_Disassembler6502) {
                    mc_Processor6502.DebugInfo(m_MemoryMap);
                }
#if BreakPointMemory
                if (!m_Disassembler6502 && m_BreakPointMemory.Found) {
                    mc_Processor6502.DebugInfo(m_MemoryMap);
                }
#endif
            }
        } else if (m_Cpu6502Step) {
            m_Cpu6502Step = false;
#if BreakPointOpCode
            if (TestForBreakPointOpCode()) {
                m_Disassembler6502 = true;
            }
#endif
            if (mc_Processor6502.RunOneOp()) {
                mc_Processor6502.DebugCrashInfo(m_MemoryMap);
                m_Quit = true;
            } else {
                mc_Processor6502.DebugInfo(m_MemoryMap);
            }
        }
        OpCounter++;
        TotalCycles += mc_Processor6502.m_Debug.TotalCycles;
        HAL_IWDG_Refresh(&hiwdg);
    }
    EndTime = HAL_GetTick();
    TotalTime = EndTime - StartTime;
    CpuClk = double(TotalCycles / TotalTime) * 1000.0;
    snprintf(StatusMsg, sizeof(StatusMsg), "Cpu OpCounter = %ld CpuTotalCycles = %ld TotalTime = %ld ms Cpu Clk = %.6f Mhz", OpCounter, TotalCycles, TotalTime, CpuClk / 1000000.0);
    PrintStatus(false, StatusMsg);

    Destroy();
    PrintStatus(false, "App Ends");
}
#endif
//-Public----------------------------------------------------------------------
// Name: CpuMemoryMapRead(uint16_t& address)
//-----------------------------------------------------------------------------
uint8_t MC_Hardware6502::CpuMemoryMapRead(uint16_t& address)
{
    uint8_t Data = 0xFF;
#if CompukitUK101
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress) {
        Data = m_MemoryMap[address];
    } else  if (address >= MemoryBasicRomAddress && address <= MemoryBasicRomEndAddress) {
        Data = m_MemoryMap[address];
    } else  if (address >= MemoryVideoAddress && address <= MemoryVideoEndAddress) {
        Data = m_MemoryMap[address];
    } else  if (address >= (MemoryVideoAddress + MemoryVideoSizeAddress) && address <= ((MemoryVideoAddress + MemoryVideoSizeAddress) + 0x3F) ) {     //Bug Fix 48 Line Scroll Up
        Data = m_MemoryMap[address];
    } else  if (address >= MemoryKeyboardAddress && address <= MemoryKeyboardEndAddress) {
        Data = CpuEmuKeyboard(address, true);
    } else  if (address >= Memory6850Address && address <= Memory6850EndAddress) {
        Data = CpuEmu6850UartRead(address);
    } else  if (address >= MemoryMonitorRomAddress && address <= MemoryMonitorRomEndAddress) {
        Data = m_MemoryMap[address];
    }
#else
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress) {
        Data = m_MemoryMap[address];
    } else  if (address >= MemoryRomAddress && address <= MemoryRomEndAddress) {
        Data = m_MemoryMap[address];
    }
#endif
#if BreakPointMemory
    TestForBreakPointMemory(address, Data, true);
#endif
    return Data;
}
//-Public----------------------------------------------------------------------
// Name: CpuMemoryMapWrite(uint16_t& address, uint8_t& value)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMemoryMapWrite(uint16_t& address, uint8_t& value)
{
#if CompukitUK101
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress && (MemoryRamRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    } else  if (address >= MemoryBasicRomAddress && address <= MemoryBasicRomEndAddress && (MemoryBasicRomRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    } else  if (address >= MemoryVideoAddress && address <= MemoryVideoEndAddress && (MemoryVideoRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;

        m_VideoRamUpdate = true;
    } else  if (address >= MemoryKeyboardAddress && address <= MemoryKeyboardEndAddress && (MemoryKeyboardRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
        CpuEmuKeyboard(address, false);
    } else  if (address >= Memory6850Address && address <= Memory6850EndAddress && (Memory6850RWAddress || m_MemoryWriteOverride)) {
        CpuEmu6850UartWrite(address, value);
    } else  if (address >= MemoryMonitorRomAddress && address <= MemoryMonitorRomEndAddress && (MemoryMonitorRomRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    }
#else
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress && (MemoryRamRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    } else  if (address >= MemoryRomAddress && address <= MemoryRomEndAddress && (MemoryRomRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    }
#endif
#if BreakPointMemory
    TestForBreakPointMemory(address, value, false);
#endif
}
#if CompukitUK101
//-Public----------------------------------------------------------------------
// Name: KeyboardMapKey(uint8_t& KeyPress)
//-----------------------------------------------------------------------------
void MC_Hardware6502::KeyboardMapKey(uint8_t& KeyPress)
{
    int i;
    int len = sizeof(m_KeyboardCodeTable);
    int index;
    uint8_t Keycp;

    if (KeyPress) {
        for (i = 0; i < len; i++) {
            index = (i * 7) + 0;
            if (index < len) {
                Keycp = m_KeyboardCodeTable[index];
                if (Keycp && Keycp == KeyPress) {
                    memset(&m_MemoryKeyScan, 0x00, sizeof(m_MemoryKeyScan));
                    m_MemoryKeyScan.Keyin = KeyPress;
                    m_MemoryKeyScan.Row = m_KeyboardCodeTable[index + 1];
                    m_MemoryKeyScan.Col = m_KeyboardCodeTable[index + 2];
                    m_MemoryKeyScan.LShift = m_KeyboardCodeTable[index + 3];
                    m_MemoryKeyScan.RShift = m_KeyboardCodeTable[index + 4];
                    m_MemoryKeyScan.caplock = m_KeyboardCodeTable[index + 5];
                    m_MemoryKeyScan.ctrl = m_KeyboardCodeTable[index + 6];
                    m_MemoryKeyScan.RowScanCode = m_KeyboardOutTable[(uint8_t)m_MemoryKeyScan.Row];
                    m_MemoryKeyScan.ColScanCode = m_KeyboardOutTable[(uint8_t)m_MemoryKeyScan.Col];
                    m_MemoryKeyScan.CpuCount = 0;
                    m_MemoryKeyScan.KeysDone = false;
                    m_MemoryKeyScan.CpuRow = 0xFF;
                }
            }
        }
    }
}
#endif
//-Public----------------------------------------------------------------------
// Name: CpuMemoryMapFullDump()
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMemoryMapFullDump()
{
    PrintHexDump16Bit("Memory Dump", &m_MemoryMap, sizeof(m_MemoryMap), 0);
}
//-Public----------------------------------------------------------------------
// Name: CpuMemoryMapDump(uint16_t StartAddress, uint16_t EndAddress)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuMemoryMapDump(uint16_t StartAddress, uint16_t EndAddress)
{
    long MemSize = (EndAddress - StartAddress) + 1;

    if (MemSize >= 0 && MemSize <= 0x10000 && StartAddress <= EndAddress) {
        PrintHexDump16Bit("Memory Dump", &m_MemoryMap[StartAddress], (EndAddress - StartAddress) + 1, StartAddress);
    } else {
        printf("CpuMemoryMapDump Error %04X %04X\r\n", StartAddress, EndAddress);
    }
}















//*****************************************************************************  
// Protected Code
//*****************************************************************************

//-Protected-------------------------------------------------------------------
// Name: MemoryInit()
//-----------------------------------------------------------------------------
void MC_Hardware6502::MemoryInit()
{
    m_Quit = false;
    m_Cpu6502Run = false;
    m_Cpu6502Step = false;
    m_Disassembler6502 = false;
    m_MemoryWriteOverride = false;
    m_KbChar = 0;
    m_VideoRamCrc = 0;
    memset(&m_BreakPointOpCode, 0, sizeof(m_BreakPointOpCode));
    memset(&m_BreakPointMemory, 0, sizeof(m_BreakPointMemory));
    memset(&m_MemoryMap, 0xFF, sizeof(m_MemoryMap));
#if CompukitUK101
    memset(&m_MemoryMap, 0xFF, sizeof(m_MemoryMap));
    memset(&m_MemoryMap[MemoryRamAddress], 0x00, MemoryRamSizeAddress);
    memset(&m_MemoryMap[MemoryBasicRomAddress], 0xFF, MemoryBasicRomSizeAddress);
    memset(&m_MemoryMap[MemoryVideoAddress], 0x00, MemoryVideoSizeAddress);
    memset(&m_MemoryMap[MemoryVideoAddress + MemoryVideoSizeAddress], 0x20, 0x40);    //Bug Fix 48 Line Scroll Up
    memset(&m_MemoryMap[MemoryKeyboardAddress], 0xFF, MemoryKeyboardSizeAddress);
    memset(&m_MemoryMap[Memory6850Address], 0x00, Memory6850SizeAddress);
    memset(&m_MemoryMap[MemoryMonitorRomAddress], 0xFF, MemoryMonitorRomSizeAddress);
    memset(&m_Screen, 0x00, sizeof(m_Screen));
    memset(&m_MemoryKeyScan, 0x00, sizeof(m_MemoryKeyScan));
    m_MemoryKeyScan.KeysDone = true;
    m_VideoRamUpdate = false;
    memset(&m_Uart6850, 0x00, sizeof(m_Uart6850));
    Cpu6850Uartinit();
#else
    memset(&m_MemoryMap[MemoryRamAddress], 0x00, MemoryRamSizeAddress);
    memset(&m_MemoryMap[MemoryRomAddress], 0xFF, MemoryRomSizeAddress);
#endif
    PrintStatus(false, "Hardware Cpu Memory Initialize");
}
//-Public----------------------------------------------------------------------
// Name: MemoryLoadProgramFiles()
//-----------------------------------------------------------------------------
void MC_Hardware6502::MemoryLoadProgramFiles()
{
	mc_FileSystem6502.LoadFiles();
    //MemoryLoad(0x000A, 65526, "GoodRoms/6502_functional_test.bin");
}
//-Protected-------------------------------------------------------------------
// Name: MemoryLoad(uint16_t MemoryAddress , uint16_t MemorySize, std::string FileName)
//-----------------------------------------------------------------------------
void MC_Hardware6502::MemoryLoad(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName)
{
    streampos size;
    uint8_t* memblock;
    uint32_t FileSize = 0;
    uint32_t MemMapMaxSize = MemoryAddress + MemorySize;
    char StatusMsg[256];
    bool Error = false;

    StatusMsg[0] = 0;
    ifstream file(FileName, ios::in | ios::binary | ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        FileSize = (uint32_t)size;
        memblock = new uint8_t[FileSize];
        file.seekg(0, ios::beg);
        file.read((char*)memblock, size);
        file.close();
        if (FileSize <= MemorySize && MemMapMaxSize <= 0x10000) {
            memcpy(&m_MemoryMap[MemoryAddress], memblock, FileSize);
            snprintf(StatusMsg, sizeof(StatusMsg), "Load Ram/Rom ($%04X - $%04X) %s", MemoryAddress, (unsigned int)(MemoryAddress + (FileSize - 1)), (char*)FileName.c_str());
        } else {
            snprintf(StatusMsg, sizeof(StatusMsg), "Load Ram/Rom file to Big for Memory(%06lX,%06lX) Slot File %s", FileSize, (unsigned long)(MemMapMaxSize - 1), (char*)FileName.c_str());
            Error = true;
        }
        delete[] memblock;
    } else {
        snprintf(StatusMsg, sizeof(StatusMsg), "Load Ram/Rom  Unable to open file %s", FileName.c_str());
        Error = true;
    }
    PrintStatus(Error, StatusMsg);
}
//-Protected-------------------------------------------------------------------
// Name: MemorySave(uint16_t MemoryAddress , uint16_t MemorySize, std::string FileName)
//-----------------------------------------------------------------------------
void MC_Hardware6502::MemorySave(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName)
{
    streampos size;
    uint8_t* memblock;
    char StatusMsg[256];
    bool Error = false;

    StatusMsg[0] = 0;
    size = MemorySize;
    fstream file(FileName, ios::out | ios::binary | ios::ate);
    if (file.is_open()) {
        memblock = new uint8_t[(uint16_t)size];
        memcpy(memblock, &m_MemoryMap[MemoryAddress], (size_t)size);
        file.seekg(0, ios::beg);
        file.write((char*)memblock, size);
        file.close();
        delete[] memblock;
        snprintf(StatusMsg, sizeof(StatusMsg), "Save Ram/Rom ($%04X - $%04X) %s", MemoryAddress, MemoryAddress + (MemorySize - 1), FileName.c_str());
    } else {
        snprintf(StatusMsg, sizeof(StatusMsg), "Save Ram/Rom Unable to open file %s", FileName.c_str());
        Error = true;
    }
    PrintStatus(Error, StatusMsg);
}
//-Protected-------------------------------------------------------------------
// Name: MemoryLoadIntelFormat(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName)
//-----------------------------------------------------------------------------
void MC_Hardware6502::MemoryLoadIntelFormat(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName)
{
    uint16_t l;
    size_t LineLen;
    size_t EndOfDataLine;
    std::string StringLine;
    std::string StringLineStart;
    std::string StringNumberOfBytes;
    std::string StringMemAddress;
    std::string StringRecordType;
    std::string StringData;
    std::string StringByte;
    std::string StringCrc;
    std::string StringCrcCal;
    uint8_t NumberOfBytes;
    uint16_t MemAddress;
    uint8_t RecordType;
    uint8_t Data;
    uint8_t Crc;
    uint8_t LineCrc;
    char StatusMsg[256];

    StatusMsg[0] = 0;
    if (FileName.length() == 0) {
        return;
    }
    ifstream file(FileName, ios::in | ios::ate);
    if (file.is_open()) {
        snprintf(StatusMsg, sizeof(StatusMsg), "Load Intel File Format %s", FileName.c_str());
        PrintStatus(false, StatusMsg);
        while (std::getline(file, StringLine)) {
            if (StringLine.length() >= 11) {
                LineLen = strlen(StringLine.c_str());
                StringCrc = StringLine.substr(LineLen - 2, 2);
                StringCrcCal = StringLine.substr(1, LineLen - 3);
                LineCrc = 0x00;
                for (l = 0; l < StringCrcCal.length(); l += 2) {
                    StringByte = StringCrcCal.substr(l, 2);
                    Data = (uint8_t)Hex2Dec(StringByte);
                    LineCrc += Data;
                }
                LineCrc ^= 0xFF;
                LineCrc++;
                Crc = (uint8_t)Hex2Dec(StringCrc);
                if (LineCrc == Crc) {
                    StringLineStart = StringLine.substr(0, 1);
                    StringNumberOfBytes = StringLine.substr(1, 2);
                    StringMemAddress = StringLine.substr(3, 4);
                    StringRecordType = StringLine.substr(7, 2);
                    EndOfDataLine = LineLen - (8 + 2);
                    StringData = StringLine.substr(9, EndOfDataLine);
                    if (StringData.length()) {
                        NumberOfBytes = (uint8_t)Hex2Dec(StringNumberOfBytes);
                        MemAddress = Hex2Dec(StringMemAddress);
                        RecordType = (uint8_t)Hex2Dec(StringRecordType);
                        if (RecordType == 0 && NumberOfBytes > 0) {
                            for (l = 0; l < StringData.length(); l += 2) {
                                StringByte = StringData.substr(l, 2);
                                Data = (uint8_t)Hex2Dec(StringByte);
                                if (MemAddress < MemorySize) {
                                    m_MemoryWriteOverride = true;
                                    CpuMemoryWrite(MemoryAddress + MemAddress, Data);
                                    m_MemoryWriteOverride = false;
                                }
                                MemAddress++;
                            }
                        }
                    }
                } else {
                    StringNumberOfBytes = StringLine.substr(1, 2);
                    NumberOfBytes = (uint8_t)Hex2Dec(StringNumberOfBytes);
                    if (NumberOfBytes > 0) {
                        snprintf(StatusMsg, sizeof(StatusMsg), "Crc Error %02X %02X %s", Crc, LineCrc, StringLine.c_str());
                        PrintStatus(true, StatusMsg);
                    }
                }
            }
        }
        file.close();
    } else {
        snprintf(StatusMsg, sizeof(StatusMsg), "Unable to open file %s", FileName.c_str());
        PrintStatus(true, StatusMsg);
    }
}
//-Protected-------------------------------------------------------------------
// Name: Hex2Dec(std::string StrHex)
//-----------------------------------------------------------------------------
uint16_t MC_Hardware6502::Hex2Dec(std::string StrHex)
{
    uint16_t i = 0;

    std::stringstream ssHex(StrHex);
    ssHex >> std::hex >> i;
    return i;
}
//-Protected-------------------------------------------------------------------
// Name: PrintHexDump16Bit(const char* desc, void* addr, long len, long offset)
//-----------------------------------------------------------------------------
void MC_Hardware6502::PrintHexDump16Bit(const char* desc, void* addr, long len, long offset)
{
    long i;
    uint8_t* pc = (uint8_t*)addr;
    char buff[20];
    uint8_t Data;

    if (desc != NULL) {
        printf("\n%s:\r\n", desc);
    }
    if (len == 0) {
        printf("\nZERO LENGTH\r\n");
        return;
    }
    else if (len < 0) {
        printf("\nNEGATIVE LENGTH: %ld\r\n", len);
        return;
    }
    printf("Addr:  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F\r\n");
    printf("-----  ------------------------------------------------\r\n");
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                printf("  | %s |\r\n", buff);
            }
            printf("$%04lX ", i + offset);
        }
        Data = pc[i];
        if ((i % 16) == 8)
            printf("  %02X", Data);
        else
            printf(" %02X", Data);

        if ((Data < 0x20) || (Data > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = Data;

        buff[(i % 16) + 1] = '\0';
    }
    while ((i % 16) != 0) {
        if ((i % 16) == 8)
            printf("    ");
        else
            printf("   ");

        i++;
    }
    printf("  | %s |\r\n", buff);
    printf("-----  ------------------------------------------------\r\n");
}
//-Protected-------------------------------------------------------------------
// Name: TestForBreakPointOpCode()
//-----------------------------------------------------------------------------
bool MC_Hardware6502::TestForBreakPointOpCode()
{
    if (m_BreakPointOpCode.SetFlag && m_BreakPointOpCode.Address == mc_Processor6502.GetPC()) {
        m_BreakPointOpCode.Found = true;
        PrintStatus(false, "Cpu BreakPoint OpCode");
    } else {
        m_BreakPointOpCode.Found = false;
    }
    return m_BreakPointOpCode.Found;
}
//-Protected-------------------------------------------------------------------
// Name: TestForBreakPointMemory(uint16_t& address, uint8_t& data, bool ReadWrite)
//-----------------------------------------------------------------------------
void MC_Hardware6502::TestForBreakPointMemory(uint16_t& address, uint8_t& data, bool ReadWrite)
{
    char buf[256];

    if (m_BreakPointMemory.SetFlag && m_BreakPointMemory.Address == address) {
        m_BreakPointMemory.Found = true;
        if (ReadWrite) {
            snprintf(buf, sizeof(buf), "Cpu BreakPoint Memory $%04X Read %02X", address, data);
            PrintStatus(false, buf);
        } else {
            snprintf(buf, sizeof(buf), "Cpu BreakPoint Memory $%04X Write %02X", address, data);
            PrintStatus(false, buf);
        }
    } 
}
#if CompukitUK101
//-----------------------------------------------------------------------------
// Name: PrintVideoRam(,void *addr,long len)
//-----------------------------------------------------------------------------
void  MC_Hardware6502::PrintVideoRam(void* addr, long len)
{
    long i;
    uint8_t* pc = (uint8_t*)addr;
    char buff[100] = {0};
    uint8_t Data;

    if (len == 0) {
        printf("\nZERO LENGTH\r\n");
        return;
    }
    else if (len < 0) {
        printf("\nNEGATIVE LENGTH: %ld\r\n", len);
        return;
    }
    for (i = 0; i < len; i++) {
        Data = pc[i];
        if ((i % 64) == 0 && i != 0) {
            printf("%s\r\n", buff);
        }
        if ((Data < 0x20) || (Data > 0x7e) || (Data == 0x60)) {
        	if(Data == 0xA1) {
        		buff[i % 64] = '_';
        	} else {
        		buff[i % 64] = ' ';
        	}
        } else {
       		buff[i % 64] = Data;
        }
        buff[(i % 64) + 1] = '\0';


    }
    //printf("%s\r\n", buff);
    printf("%s\r\n", buff);
    //printf("--------------------------------------------------------------------------\r\n");
}
//-Protected-------------------------------------------------------------------
// Name: CpuEmuKeyboard(uint8_t* MemData, bool RW)
//-----------------------------------------------------------------------------
uint8_t MC_Hardware6502::CpuEmuKeyboard(uint16_t address, bool RW)
{
    uint8_t Data;
    uint8_t CpuRow;
    uint8_t RowScanCode;

    if (RW) {
        Data = 0xFF;
        CpuRow = ((m_MemoryKeyScan.CpuRow ^ 0xFF) & 0x01);
        if (CpuRow && m_MemoryKeyScan.KeysDone == false) {
            if (m_MemoryKeyScan.caplock) {
                Data = (Data & 0xFE);
            }
            if (m_MemoryKeyScan.RShift) {
                Data = (Data & 0xFD);
            }
            if (m_MemoryKeyScan.LShift) {
                Data = (Data & 0xFB);
            }
            if (m_MemoryKeyScan.ctrl) {
                Data = (Data & 0xBF);
            }
        }
        CpuRow = ((m_MemoryKeyScan.CpuRow | 0x01) ^ 0xFF);
        RowScanCode = (m_MemoryKeyScan.RowScanCode | 0x01) ^ 0xFF;
        if ((CpuRow & RowScanCode) && m_MemoryKeyScan.KeysDone == false) {
            Data = (Data & m_MemoryKeyScan.ColScanCode);
            m_MemoryKeyScan.CpuCount++;
            if (m_MemoryKeyScan.CpuCount > 4) {
                m_MemoryKeyScan.CpuCount = 0;
                m_MemoryKeyScan.KeysDone = true;
            }
        }
    } else {
        Data = m_MemoryMap[address];
        m_MemoryKeyScan.CpuRow = Data;
    }
    return Data;
}
//-Protected-------------------------------------------------------------------
// Name: Cpu6850Uartinit()
//-----------------------------------------------------------------------------
void MC_Hardware6502::Cpu6850Uartinit()
{
    m_Uart6850.Registers_SR.byte = 0;
    m_Uart6850.Registers_SR.bits.TDRE = 1;                                     // we are always ready to output data
    m_Uart6850.Registers_SR.bits.RDRF = 0;
    m_Uart6850.Registers_SR.bits.DCD = 1;
    m_Uart6850.Registers_SR.bits.CTS = 1;
    m_Uart6850.Registers_SR.bits.FE = 1;
    m_Uart6850.Registers_SR.bits.OVRN = 0;
    m_Uart6850.Registers_SR.bits.PE = 1;
    m_Uart6850.Registers_SR.bits.IRQ = 1;
    memset(&m_Uart6850.Input.Buffer, 0x00, sizeof(m_Uart6850.Input.Buffer));
    m_Uart6850.Input.CharData = 0;
    m_Uart6850.Input.ProcessedIndex = 0;
    m_Uart6850.Input.Index = 0;
    memset(&m_Uart6850.Output.Buffer, 0x00, sizeof(m_Uart6850.Output.Buffer));
    m_Uart6850.Output.CharData = 0;
    m_Uart6850.Output.ProcessedIndex = 0;
    m_Uart6850.Output.Index = 0;
#if CPU6502_TESTMODE == false
    m_MemoryMap[DATA_6850ADDR] = m_Uart6850.Input.CharData;
    m_MemoryMap[CTRL_6850ADDR] = m_Uart6850.Registers_SR.byte;
#endif
    PrintStatus(false, "Hardware 6850 ACIA Initialize");
}
//-Protected-------------------------------------------------------------------
// Name: CpuEmu6850UartRead(uint16_t address)
//-----------------------------------------------------------------------------
uint8_t MC_Hardware6502::CpuEmu6850UartRead(uint16_t address)
{
    static uint16_t s_RxDelayTick = 0;
    static uint16_t s_TxDelayTick = 0;
    switch (address) {
        case CTRL_6850ADDR: {
            s_RxDelayTick++;
            s_TxDelayTick++;
            uint8_t flags = 0;
            if (m_Uart6850.Input.ProcessedIndex < m_Uart6850.Input.Index) {
                if (m_Uart6850.Registers_SR.bits.RDRF) {
                    if (s_RxDelayTick > 10) {
                        s_RxDelayTick = 0;
                        m_Uart6850.Registers_SR.bits.RDRF = 0;
                    }
                    flags |= RDRF6850;
                }
            }
            if (m_Uart6850.Registers_SR.bits.TDRE) {
                if (s_TxDelayTick > 10) {
                    s_TxDelayTick = 0;
                    m_Uart6850.Registers_SR.bits.TDRE = 0;
                 }
            } else {
                flags |= TDRE6850;
            }
            return flags;
            break;
        }
        case DATA_6850ADDR: {
            m_Uart6850.Input.CharData = m_Uart6850.Input.Buffer[m_Uart6850.Input.ProcessedIndex++];
            m_Uart6850.Input.ProcessedIndex %= sizeof(m_Uart6850.Input.Buffer);
            m_Uart6850.Registers_SR.bits.RDRF = 1;
            return m_Uart6850.Input.CharData;
            break;
        }
        default:
            break;
    }
    return 0xff;
}
//-Protected-------------------------------------------------------------------
// Name: CpuEmu6850UartWrite(uint16_t address, uint8_t value)
//-----------------------------------------------------------------------------
void MC_Hardware6502::CpuEmu6850UartWrite(uint16_t address, uint8_t value)
{
    switch (address) {
        case CTRL_6850ADDR: {
            m_MemoryMap[address] = value;
            m_Uart6850.Registers_SR.byte = value;
            // TODO: decode baudrate, mode, break control, interrupt
            break;
        }
        case DATA_6850ADDR: {
            m_MemoryMap[address] = value;
            m_Uart6850.Output.CharData = value;
            m_Uart6850.Output.Buffer[m_Uart6850.Output.ProcessedIndex++] = m_Uart6850.Output.CharData;
            m_Uart6850.Output.ProcessedIndex %= sizeof(m_Uart6850.Output.Buffer);
            m_Uart6850.Output.Index = m_Uart6850.Output.ProcessedIndex;
            m_Uart6850.Registers_SR.bits.TDRE = 1;
            break;
        }
        default: {
            break;
        }
    }
}
#endif
