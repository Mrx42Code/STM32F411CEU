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
#if BreakPointMemory
    CpuSetBreakPointMemory(true, 0x0046);
#endif
#if BreakPointOpCode
    CpuSetBreakPointOpCode(true, 0x3469);                                            // if you get here everything went well 3469 : 4c6934 > jmp*; test passed, no errors
#endif
    CpuSetPC(0x0400);
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
//-Public----------------------------------------------------------------------
// Name: CpuMemoryMapRead(uint16_t& address)
//-----------------------------------------------------------------------------
uint8_t MC_Hardware6502::CpuMemoryMapRead(uint16_t& address)
{
    uint8_t Data = 0xFF;
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress) {
        Data = m_MemoryMap[address];
    } else  if (address >= MemoryRomAddress && address <= MemoryRomEndAddress) {
        Data = m_MemoryMap[address];
    }
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
    if (address >= MemoryRamAddress && address <= MemoryRamEndAddress && (MemoryRamRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    } else  if (address >= MemoryRomAddress && address <= MemoryRomEndAddress && (MemoryRomRWAddress || m_MemoryWriteOverride)) {
        m_MemoryMap[address] = value;
    }
#if BreakPointMemory
    TestForBreakPointMemory(address, value, false);
#endif
}
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
    memset(&m_BreakPointOpCode, 0, sizeof(m_BreakPointOpCode));
    memset(&m_BreakPointMemory, 0, sizeof(m_BreakPointMemory));
    memset(&m_MemoryMap, 0xFF, sizeof(m_MemoryMap));
    memset(&m_MemoryMap[MemoryRamAddress], 0x00, MemoryRamSizeAddress);
    memset(&m_MemoryMap[MemoryRomAddress], 0xFF, MemoryRomSizeAddress);
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
