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
#define BreakPointMemory                    true
#define BreakPointOpCode                    true

#define MemoryMapAddress					0x0000								// 0000-FFFF Cpu Memory Map
#define MemoryMapSizeAddress				0x10000								// 64k
#define MemoryMapEndAddress				    (MemoryMapAddress + MemoryMapSizeAddress) - 1

#define MemoryRamAddress					0x0000								// 0000-7FFF Ram 
#define MemoryRamSizeAddress				0x8000								// 32k
#define MemoryRamEndAddress				    (MemoryRamAddress + MemoryRamSizeAddress) - 1
#define MemoryRamRWAddress					true								// R/W    

#define MemoryRomAddress					0x8000								// 8000 - FFFF Rom
#define MemoryRomSizeAddress				0x8000								// 32k
#define MemoryRomEndAddress				    (MemoryRomAddress + MemoryRomSizeAddress) - 1
#define MemoryRomRWAddress				    false					            // R    

//-----------------------------------------------------------------------------
// Struct
//-----------------------------------------------------------------------------
typedef struct BreakPoint
{
    bool                SetFlag;
    bool                Found;
    uint16_t            Address;
} _BreakPoint;

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

    protected:

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

    private:

};
//-----------------------------------------------------------------------------
#endif // MC_Hardware6502_H
