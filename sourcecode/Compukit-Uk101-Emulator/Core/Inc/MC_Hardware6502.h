/**********************************************************************************
* MIT License																	  *
*																				  *
* Copyright(c) 2021 Mrx42Code                                                     *
* https://github.com/Mrx42Code                          				          *
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
 // File: MC_Hardware6502.h: interface for the MC_Hardware6502 class.
 // Desc: Application interface for the MC_Hardware6502 class.
 //-----------------------------------------------------------------------------
#ifndef MC_Hardware6502_H
    #define MC_Hardware6502_H

#pragma once

#include "IncludeLibraries.h"

//-----------------------------------------------------------------------------
// Const
//-----------------------------------------------------------------------------
#define CompukitUK101						true
#define CompukitUK101_DebugKeypress			false

#define BreakPointMemory                    true
#define BreakPointOpCode                    true

#define MemoryMapAddress					0x0000								// 0000-FFFF Cpu Memory Map
#define MemoryMapSizeAddress				0x10000								// 64k
#define MemoryMapEndAddress				    (MemoryMapAddress + MemoryMapSizeAddress) - 1

#if CompukitUK101
	#define MemoryRamAddress					0x0000								// 0000-7FFF Ram
	#define MemoryRamSizeAddress				0x8000								// 32k
	#define MemoryRamEndAddress				    (MemoryRamAddress + MemoryRamSizeAddress) - 1
	#define MemoryRamRWAddress					true								// R/W

	#define MemoryBasicRomAddress				0xA000								// A000 - BFFF Basic Rom
	#define MemoryBasicRomSizeAddress			0x2000								// 8k
	#define MemoryBasicRomEndAddress			(MemoryBasicRomAddress + MemoryBasicRomSizeAddress) - 1
	#define MemoryBasicRomRWAddress				false								// R

	#define MemoryVideoAddress					0xD000								// D000 - D3FF Video Ram
	#define MemoryVideoSizeAddress				0x0400								// (1k 0x0400 48x16) (2k 0x0800 48x32) (3k 0x0C00 48x48)
	#define MemoryVideoEndAddress			    (MemoryVideoAddress + MemoryVideoSizeAddress) - 1
	#define MemoryVideoRWAddress				true								// R/W

	#define MemoryKeyboardAddress				0xDF00								// DF00 Keyboard  Controller
	#define MemoryKeyboardSizeAddress			0x0001								// 1
	#define MemoryKeyboardEndAddress		    (MemoryKeyboardAddress + MemoryKeyboardSizeAddress) - 1
	#define MemoryKeyboardRWAddress				true								// R/W

	#define Memory6850Address					0xF000								// F000 - F001 6850  Controller
	#define Memory6850SizeAddress				0x0002								// 2
	#define Memory6850EndAddress		    	(Memory6850Address + Memory6850SizeAddress) - 1
	#define Memory6850RWAddress					true								// R/W

	#define MemoryMonitorRomAddress				0xF800								// F800 - FFFF Monitor Rom
	#define MemoryMonitorRomSizeAddress			0x0800								// 4k
	#define MemoryMonitorRomEndAddress			(MemoryMonitorRomAddress + MemoryMonitorRomSizeAddress) - 1
	#define MemoryMonitorRomRWAddress			false								// R

	#define CTRL_6850ADDR						Memory6850Address					// F000 6850 Controller Reg Ctrl 6850
	#define DATA_6850ADDR						Memory6850Address + 1				// F001 6850 Controller Reg Data 6850

	#define RDRF6850                            (1 << 0)
	#define TDRE6850                            (1 << 1)
	#define UARTBUFFER6850                      (1024 * 1)
#else
	#define MemoryRamAddress					0x0000								// 0000-7FFF Ram
	#define MemoryRamSizeAddress				0x8000								// 32k
	#define MemoryRamEndAddress				    (MemoryRamAddress + MemoryRamSizeAddress) - 1
	#define MemoryRamRWAddress					true								// R/W

	#define MemoryRomAddress					0x8000								// 8000 - FFFF Rom
	#define MemoryRomSizeAddress				0x8000								// 32k
	#define MemoryRomEndAddress				    (MemoryRomAddress + MemoryRomSizeAddress) - 1
	#define MemoryRomRWAddress				    false					            // R
#endif

//-----------------------------------------------------------------------------
// Struct
//-----------------------------------------------------------------------------
typedef struct BreakPoint
{
    bool                SetFlag;
    bool                Found;
    uint16_t            Address;
} _BreakPoint;

#if CompukitUK101
	typedef struct KeyInputType
	{
		uint8_t	                Keyin;
		uint8_t	                Row;
		uint8_t	                RowScanCode;
		uint8_t	                Col;
		uint8_t	                ColScanCode;
		uint8_t	                LShift;
		uint8_t	                RShift;
		uint8_t	                caplock;
		uint8_t	                ctrl;
		bool	                KeysDone;
		uint8_t	                CpuRow;
		uint8_t	                CpuCount;
	} _KeyInputType;

	typedef struct UartStatusBits
	{
		bool                    RDRF : 1; // bit 0
		bool                    TDRE : 1;
		bool                    DCD : 1;
		bool                    CTS : 1;
		bool                    FE : 1;
		bool                    OVRN : 1;
		bool                    PE : 1;
		bool                    IRQ : 1;  // bit 7
	} _UartStatusBits;

	union UartStatusReg {
		struct UartStatusBits   bits;
		uint8_t                 byte;
	};

	typedef struct Uart6850InOut
	{
		uint8_t                 CharData;
		char                    Buffer[UARTBUFFER6850];
		int                     ProcessedIndex;
		int                     Index;
	} _Uart6850InOut;

	typedef struct Uart6850
	{
		union UartStatusReg     Registers_SR;
		Uart6850InOut           Input;
		Uart6850InOut           Output;
	} _Uart6850;

	typedef struct ScreenUpdate
	{
		bool					Update;
		uint32_t				TimeOut;
	} _ScreenUpdate;
#endif
//-----------------------------------------------------------------------------
// Name: class MC_Hardware6502
// Desc: Application class
//-----------------------------------------------------------------------------
class MC_Hardware6502
{

    public:
        bool                m_Quit;
        bool                m_Disassembler6502;
        bool                m_Cpu6502Run;
        bool                m_Cpu6502Step;
        BreakPoint          m_BreakPointOpCode;
        BreakPoint          m_BreakPointMemory;
        uint8_t             m_MemoryMap[MemoryMapSizeAddress];
        bool                m_MemoryWriteOverride;

        uint8_t 			m_KbChar;
        uint32_t 			m_VideoRamCrc;

    protected:
#if CompukitUK101
        KeyInputType        m_MemoryKeyScan;
        Uart6850            m_Uart6850;
        bool				m_VideoRamUpdate;
        ScreenUpdate		m_Screen;
#endif
    private:

        //-----------------------------------------------------------------------------

    public:
                            MC_Hardware6502();
        virtual				~MC_Hardware6502();
        void				Initialize();
        void				Destroy();
        void				Create();
        void                PrintStatus(bool Error, std::string Msg);

        void                CpuSetParameters();
        void                CpuSetBreakPointOpCode(bool Enable, uint16_t Address);
        void                CpuSetBreakPointMemory(bool Enable, uint16_t Address);
        void				CpuSetPC(uint16_t PC);

        void                CpuIRQ();
        void                CpuNMI();
        void                CpuInitializeAndReset();
        void                CpuReset();
        void                CpuStop();
        void                CpuRun();
        void                CpuStep();
        void                CpuMainLoop();
        void 				CpuMainLoopCompukitUk101();
        void 				CpuMainLoopFunctionalTest6502();
        void 				KeyboardMapKey(uint8_t& KeyPress);
        void                CpuMemoryMapFullDump();
        void                CpuMemoryMapDump(uint16_t StartAddress, uint16_t EndAddress);

        uint8_t             CpuMemoryMapRead(uint16_t& address);
        void                CpuMemoryMapWrite(uint16_t& address, uint8_t& value);

    protected:
        void                MemoryInit();
        void                MemoryLoadProgramFiles();
        void                MemoryLoad(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName);
        void                MemorySave(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName);
        void 				MemoryLoadIntelFormat(uint16_t MemoryAddress, uint16_t MemorySize, std::string FileName);
        uint16_t            Hex2Dec(std::string StrHex);
        void                PrintHexDump16Bit(const char* desc, void* addr, long len, long offset);
        bool                TestForBreakPointOpCode();
        void                TestForBreakPointMemory(uint16_t& address, uint8_t& data, bool ReadWrite);
#if CompukitUK101
        void  				PrintVideoRam(void* addr, long len);
        uint8_t 			CpuEmuKeyboard(uint16_t address, bool RW);
        void 				Cpu6850Uartinit();
        uint8_t 			CpuEmu6850UartRead(uint16_t address);
        void 				CpuEmu6850UartWrite(uint16_t address, uint8_t value);
#endif
    private:

};
//-----------------------------------------------------------------------------
#endif // MC_Hardware6502_H
