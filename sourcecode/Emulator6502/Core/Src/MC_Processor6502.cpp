/**********************************************************************************
* MIT License																	  *
*																				  *
* mos6502																		  *
* Copyright(c) 2017 Gianluca Ghettini											  *
* https://github.com/gianlucag/mos6502											  *
*																				  *
* Disassembler for the 6502 microprocessor										  *
* Copyright (c) 1998-2014 Tennessee Carmel-Veilleux <veilleux@tentech.ca>         *
* https://github.com/tcarmelveilleux/dcc6502									  *
*																				  *
* Copyright(c) 2021 Mrx42Code                                                     *
* https://github.com/Mrx42Code											          *
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
 // MC_Processor6502.cpp is base on mos6502.cpp & Disassembler dcc6502.c
 //-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// File: MC_Processor6502.cpp: implementation of the MC_Processor6502 class.
//-----------------------------------------------------------------------------
#include "MC_Processor6502.h"

using namespace std;

//*****************************************************************************  
// Public Code
//*****************************************************************************
const char StatusBits[8] = { 'C', 'Z' , 'I' , 'D', 'B', '-', 'O', 'N' };

//-----------------------------------------------------------------------------
// IMPLEMENT_DYNCREATE
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// message handlers
//-----------------------------------------------------------------------------

//-Public----------------------------------------------------------------------
// Name: MC_Processor6502()
// Desc: MC_Processor6502 class
//-----------------------------------------------------------------------------
MC_Processor6502::MC_Processor6502()
{
	Initialize();
}
//-Public----------------------------------------------------------------------
// Name: mos6502(BusRead r, BusWrite w)
// Desc: MC_Processor6502 class
//-----------------------------------------------------------------------------
MC_Processor6502::MC_Processor6502(BusRead r, BusWrite w)
{
	Initialize();
	MemoryWrite = (BusWrite)w;
	MemoryRead = (BusRead)r;
	// fill jump table with ILLEGALs
	for (int i = 0; i < 256; i++) {
		//				{ AddrMode , Code , Cycles, CanHaveExCycles , Mnemonic, AddressingMode };
		m_InstrTbl[i] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_ILLEGAL, 0	, false, nullptr, ILLEGAL };
	}
	// insert opcodes
	m_InstrTbl[0x69] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_ADC, 2, false , "ADC", IMMED };
	m_InstrTbl[0x6D] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_ADC, 4, false , "ADC", ABSOL };
	m_InstrTbl[0x65] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_ADC, 3, false , "ADC", ZEROP };
	m_InstrTbl[0x61] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_ADC, 6, false , "ADC", INDIN };
	m_InstrTbl[0x71] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_ADC, 5, true  , "ADC", ININD };
	m_InstrTbl[0x75] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_ADC, 4, false , "ADC", ZEPIX };
	m_InstrTbl[0x7D] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_ADC, 4, true  , "ADC", ABSIX };
	m_InstrTbl[0x79] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_ADC, 4, true  , "ADC", ABSIY };
	m_InstrTbl[0x29] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_AND, 2, false , "AND", IMMED };
	m_InstrTbl[0x2D] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_AND, 4, false , "AND", ABSOL };
	m_InstrTbl[0x25] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_AND, 3, false , "AND", ZEROP };
	m_InstrTbl[0x21] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_AND, 6, false , "AND", INDIN };
	m_InstrTbl[0x31] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_AND, 5, true  , "AND", ININD };
	m_InstrTbl[0x35] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_AND, 4, false , "AND", ZEPIX };
	m_InstrTbl[0x3D] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_AND, 4, true  , "AND", ABSIX };
	m_InstrTbl[0x39] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_AND, 4, true  , "AND", ABSIY };
	m_InstrTbl[0x0E] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_ASL, 6, false , "ASL", ABSOL };
	m_InstrTbl[0x06] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_ASL, 5, false , "ASL", ZEROP };
	m_InstrTbl[0x0A] = { &MC_Processor6502::Addr_ACC, &MC_Processor6502::Op_ASL_ACC, 2, false , "ASL", ACCUM };
	m_InstrTbl[0x16] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_ASL, 6, false , "ASL", ZEPIX };
	m_InstrTbl[0x1E] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_ASL, 7, false , "ASL", ABSIX };
	m_InstrTbl[0x90] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BCC, 2, true  , "BCC", RELAT };
	m_InstrTbl[0xB0] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BCS, 2, true  , "BCS", RELAT };
	m_InstrTbl[0xF0] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BEQ, 2, true  , "BEQ", RELAT };
	m_InstrTbl[0x2C] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_BIT, 4, false , "BIT", ABSOL };
	m_InstrTbl[0x24] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_BIT, 3, false , "BIT", ZEROP };
	m_InstrTbl[0x30] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BMI, 2, true  , "BMI", RELAT };
	m_InstrTbl[0xD0] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BNE, 2, true  , "BNE", RELAT };
	m_InstrTbl[0x10] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BPL, 2, true  , "BPL", RELAT };
	m_InstrTbl[0x00] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_BRK, 7, false , "BRK", IMPLI };
	m_InstrTbl[0x50] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BVC, 2, true  , "BVC", RELAT };
	m_InstrTbl[0x70] = { &MC_Processor6502::Addr_REL, &MC_Processor6502::Op_BVS, 2, true  , "BVS", RELAT };
	m_InstrTbl[0x18] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_CLC, 2, false , "CLC", IMPLI };
	m_InstrTbl[0xD8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_CLD, 2, false , "CLD", IMPLI };
	m_InstrTbl[0x58] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_CLI, 2, false , "CLI", IMPLI };
	m_InstrTbl[0xB8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_CLV, 2, false , "CLV", IMPLI };
	m_InstrTbl[0xC9] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_CMP, 2, false , "CMP", IMMED };
	m_InstrTbl[0xCD] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_CMP, 4, false , "CMP", ABSOL };
	m_InstrTbl[0xC5] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_CMP, 3, false , "CMP", ZEROP };
	m_InstrTbl[0xC1] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_CMP, 6, false , "CMP", INDIN };
	m_InstrTbl[0xD1] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_CMP, 3, true  , "CMP", ININD };
	m_InstrTbl[0xD5] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_CMP, 4, false , "CMP", ZEPIX };
	m_InstrTbl[0xDD] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_CMP, 4, true  , "CMP", ABSIX };
	m_InstrTbl[0xD9] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_CMP, 4, true  , "CMP", ABSIY };
	m_InstrTbl[0xE0] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_CPX, 2, false , "CPX", IMMED };
	m_InstrTbl[0xEC] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_CPX, 4, false , "CPX", ABSOL };
	m_InstrTbl[0xE4] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_CPX, 3, false , "CPX", ZEROP };
	m_InstrTbl[0xC0] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_CPY, 2, false , "CPY", IMMED };
	m_InstrTbl[0xCC] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_CPY, 4, false , "CPY", ABSOL };
	m_InstrTbl[0xC4] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_CPY, 3, false , "CPY", ZEROP };
	m_InstrTbl[0xCE] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_DEC, 6, false , "DEC", ABSOL };
	m_InstrTbl[0xC6] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_DEC, 5, false , "DEC", ZEROP };
	m_InstrTbl[0xD6] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_DEC, 6, false , "DEC", ZEPIX };
	m_InstrTbl[0xDE] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_DEC, 7, false , "DEC", ABSIX };
	m_InstrTbl[0xCA] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_DEX, 2, false , "DEX", IMPLI };
	m_InstrTbl[0x88] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_DEY, 2, false , "DEY", IMPLI };
	m_InstrTbl[0x49] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_EOR, 2, false , "EOR", IMMED };
	m_InstrTbl[0x4D] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_EOR, 4, false , "EOR", ABSOL };
	m_InstrTbl[0x45] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_EOR, 3, false , "EOR", ZEROP };
	m_InstrTbl[0x41] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_EOR, 6, false , "EOR", INDIN };
	m_InstrTbl[0x51] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_EOR, 5, true  , "EOR", ININD };
	m_InstrTbl[0x55] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_EOR, 4, false , "EOR", ZEPIX };
	m_InstrTbl[0x5D] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_EOR, 4, true  , "EOR", ABSIX };
	m_InstrTbl[0x59] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_EOR, 4, true  , "EOR", ABSIY };
	m_InstrTbl[0xEE] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_INC, 6, false , "INC", ABSOL };
	m_InstrTbl[0xE6] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_INC, 5, false , "INC", ZEROP };
	m_InstrTbl[0xF6] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_INC, 6, false , "INC", ZEPIX };
	m_InstrTbl[0xFE] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_INC, 7, false , "INC", ABSIX };
	m_InstrTbl[0xE8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_INX, 2, false , "INX", IMPLI };
	m_InstrTbl[0xC8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_INY, 2, false , "INY", IMPLI };
	m_InstrTbl[0x4C] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_JMP, 3, false , "JMP", ABSOL };
	m_InstrTbl[0x6C] = { &MC_Processor6502::Addr_ABI, &MC_Processor6502::Op_JMP, 5, false , "JMP", INDIA };
	m_InstrTbl[0x20] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_JSR, 6, false , "JSR", ABSOL };
	m_InstrTbl[0xA9] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_LDA, 2, false , "LDA", IMMED };
	m_InstrTbl[0xAD] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_LDA, 4, false , "LDA", ABSOL };
	m_InstrTbl[0xA5] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_LDA, 3, false , "LDA", ZEROP };
	m_InstrTbl[0xA1] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_LDA, 6, false , "LDA", INDIN };
	m_InstrTbl[0xB1] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_LDA, 5, true  , "LDA", ININD };
	m_InstrTbl[0xB5] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_LDA, 4, false , "LDA", ZEPIX };
	m_InstrTbl[0xBD] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_LDA, 4, true  , "LDA", ABSIX };
	m_InstrTbl[0xB9] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_LDA, 4, true  , "LDA", ABSIY };
	m_InstrTbl[0xA2] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_LDX, 2, false , "LDX", IMMED };
	m_InstrTbl[0xAE] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_LDX, 4, false , "LDX", ABSOL };
	m_InstrTbl[0xA6] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_LDX, 3, false , "LDX", ZEROP };
	m_InstrTbl[0xBE] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_LDX, 4, true  , "LDX", ABSIY };
	m_InstrTbl[0xB6] = { &MC_Processor6502::Addr_ZEY, &MC_Processor6502::Op_LDX, 4, false , "LDX", ZEPIY };
	m_InstrTbl[0xA0] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_LDY, 2, false , "LDY", IMMED };
	m_InstrTbl[0xAC] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_LDY, 4, false , "LDY", ABSOL };
	m_InstrTbl[0xA4] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_LDY, 3, false , "LDY", ZEROP };
	m_InstrTbl[0xB4] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_LDY, 4, false , "LDY", ZEPIX };
	m_InstrTbl[0xBC] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_LDY, 4, true  , "LDY", ABSIX };
	m_InstrTbl[0x4E] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_LSR, 6, false , "LSR", ABSOL };
	m_InstrTbl[0x46] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_LSR, 5, false , "LSR", ZEROP };
	m_InstrTbl[0x4A] = { &MC_Processor6502::Addr_ACC, &MC_Processor6502::Op_LSR_ACC, 2, false , "LSR", ACCUM };
	m_InstrTbl[0x56] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_LSR, 6, false , "LSR", ZEPIX };
	m_InstrTbl[0x5E] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_LSR, 7, false , "LSR", ABSIX };
	m_InstrTbl[0xEA] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_NOP, 2, false , "NOP", IMPLI };
	m_InstrTbl[0x09] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_ORA, 2, false , "ORA", IMMED };
	m_InstrTbl[0x0D] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_ORA, 4, false , "ORA", ABSOL };
	m_InstrTbl[0x05] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_ORA, 3, false , "ORA", ZEROP };
	m_InstrTbl[0x01] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_ORA, 6, false , "ORA", INDIN };
	m_InstrTbl[0x11] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_ORA, 5, true  , "ORA", ININD };
	m_InstrTbl[0x15] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_ORA, 4, false , "ORA", ZEPIX };
	m_InstrTbl[0x1D] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_ORA, 4, true  , "ORA", ABSIX };
	m_InstrTbl[0x19] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_ORA, 4, true  , "ORA", ABSIY };
	m_InstrTbl[0x48] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_PHA, 3, false , "PHA", IMPLI };
	m_InstrTbl[0x08] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_PHP, 3, false , "PHP", IMPLI };
	m_InstrTbl[0x68] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_PLA, 4, false , "PLA", IMPLI };
	m_InstrTbl[0x28] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_PLP, 4, false , "PLP", IMPLI };
	m_InstrTbl[0x2E] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_ROL, 6, false , "ROL", ABSOL };
	m_InstrTbl[0x26] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_ROL, 5, false , "ROL", ZEROP };
	m_InstrTbl[0x2A] = { &MC_Processor6502::Addr_ACC, &MC_Processor6502::Op_ROL_ACC, 2, false , "ROL", ACCUM };
	m_InstrTbl[0x36] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_ROL, 6, false , "ROL", ZEPIX };
	m_InstrTbl[0x3E] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_ROL, 7, false , "ROL", ABSIX };
	m_InstrTbl[0x6E] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_ROR, 6, false , "ROR", ABSOL };
	m_InstrTbl[0x66] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_ROR, 5, false , "ROR", ZEROP };
	m_InstrTbl[0x6A] = { &MC_Processor6502::Addr_ACC, &MC_Processor6502::Op_ROR_ACC, 2, false , "ROR", ACCUM };
	m_InstrTbl[0x76] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_ROR, 6, false , "ROR", ZEPIX };
	m_InstrTbl[0x7E] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_ROR, 7, false , "ROR", ABSIX };
	m_InstrTbl[0x40] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_RTI, 6, false , "RTI", IMPLI };
	m_InstrTbl[0x60] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_RTS, 6, false , "RTS", IMPLI };
	m_InstrTbl[0xE9] = { &MC_Processor6502::Addr_IMM, &MC_Processor6502::Op_SBC, 2, false , "SBC", IMMED };
	m_InstrTbl[0xED] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_SBC, 4, false , "SBC", ABSOL };
	m_InstrTbl[0xE5] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_SBC, 3, false , "SBC", ZEROP };
	m_InstrTbl[0xE1] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_SBC, 6, false , "SBC", INDIN };
	m_InstrTbl[0xF1] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_SBC, 5, true  , "SBC", ININD };
	m_InstrTbl[0xF5] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_SBC, 4, false , "SBC", ZEPIX };
	m_InstrTbl[0xFD] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_SBC, 4, true  , "SBC", ABSIX };
	m_InstrTbl[0xF9] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_SBC, 4, true  , "SBC", ABSIY };
	m_InstrTbl[0x38] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_SEC, 2, false , "SEC", IMPLI };
	m_InstrTbl[0xF8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_SED, 2, false , "SED", IMPLI };
	m_InstrTbl[0x78] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_SEI, 2, false , "SEI", IMPLI };
	m_InstrTbl[0x8D] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_STA, 4, false , "STA", ABSOL };
	m_InstrTbl[0x85] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_STA, 3, false , "STA", ZEROP };
	m_InstrTbl[0x81] = { &MC_Processor6502::Addr_INX, &MC_Processor6502::Op_STA, 6, false , "STA", INDIN };
	m_InstrTbl[0x91] = { &MC_Processor6502::Addr_INY, &MC_Processor6502::Op_STA, 6, false , "STA", ININD };
	m_InstrTbl[0x95] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_STA, 4, false , "STA", ZEPIX };
	m_InstrTbl[0x9D] = { &MC_Processor6502::Addr_ABX, &MC_Processor6502::Op_STA, 5, false , "STA", ABSIX };
	m_InstrTbl[0x99] = { &MC_Processor6502::Addr_ABY, &MC_Processor6502::Op_STA, 5, false , "STA", ABSIY };
	m_InstrTbl[0x8E] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_STX, 4, false , "STX", ABSOL };
	m_InstrTbl[0x86] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_STX, 3, false , "STX", ZEROP };
	m_InstrTbl[0x96] = { &MC_Processor6502::Addr_ZEY, &MC_Processor6502::Op_STX, 4, false , "STX", ZEPIY };
	m_InstrTbl[0x8C] = { &MC_Processor6502::Addr_ABS, &MC_Processor6502::Op_STY, 4, false , "STY", ABSOL };
	m_InstrTbl[0x84] = { &MC_Processor6502::Addr_ZER, &MC_Processor6502::Op_STY, 3, false , "STY", ZEROP };
	m_InstrTbl[0x94] = { &MC_Processor6502::Addr_ZEX, &MC_Processor6502::Op_STY, 4, false , "STY", ZEPIX };
	m_InstrTbl[0xAA] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TAX, 2, false , "TAX", IMPLI };
	m_InstrTbl[0xA8] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TAY, 2, false , "TAY", IMPLI };
	m_InstrTbl[0xBA] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TSX, 2, false , "TSX", IMPLI };
	m_InstrTbl[0x8A] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TXA, 2, false , "TXA", IMPLI };
	m_InstrTbl[0x9A] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TXS, 2, false , "TXS", IMPLI };
	m_InstrTbl[0x98] = { &MC_Processor6502::Addr_IMP, &MC_Processor6502::Op_TYA, 2, false , "TYA", IMPLI };
	return;
}
//-Public----------------------------------------------------------------------
// Name: ~MC_Processor6502()
// Desc: ~MC_Processor6502 Destruction class
//-----------------------------------------------------------------------------
MC_Processor6502::~MC_Processor6502()
{

}
//-Public----------------------------------------------------------------------
// Name: NMI()
//-----------------------------------------------------------------------------
void MC_Processor6502::NMI()
{
	SET_BREAK(0);
	StackPush((m_registers.pc >> 8) & 0xFF);
	StackPush(m_registers.pc & 0xFF);
	StackPush(m_registers.status);
	SET_INTERRUPT(1);
	m_registers.pc = (MemoryRead(s_nmiVectorH) << 8) + MemoryRead(s_nmiVectorL);
	return;
}
//-Public----------------------------------------------------------------------
// Name: IRQ()
//-----------------------------------------------------------------------------
void MC_Processor6502::IRQ()
{
	if (!IF_INTERRUPT()) {
		SET_BREAK(0);
		StackPush((m_registers.pc >> 8) & 0xFF);
		StackPush(m_registers.pc & 0xFF);
		StackPush(m_registers.status);
		SET_INTERRUPT(1);
		m_registers.pc = (MemoryRead(s_irqVectorH) << 8) + MemoryRead(s_irqVectorL);
	}
	return;
}
//-Public----------------------------------------------------------------------
// Name: Reset()
//-----------------------------------------------------------------------------
void MC_Processor6502::Reset()
{
	memset(&m_CrashDump, 0, sizeof(m_CrashDump));
	memset(&m_Debug, 0, sizeof(m_Debug));
	memset(&m_Instruction, 0, sizeof(m_Instruction));
	m_registers.A = 0x00;
	m_registers.Y = 0x00;
	m_registers.X = 0x00;
	m_registers.pc = (MemoryRead(s_rstVectorH) << 8) + MemoryRead(s_rstVectorL);	// load PC from reset vector
	m_registers.sp = 0xFD;
	m_registers.status = 0x00;
	m_registers.status |= FLAGCONSTANT;
	m_registers.IllegalOpcode = false;
	return;
}
//-Public----------------------------------------------------------------------
// Name: GetPC()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::GetPC()
{
	return m_registers.pc;
}
//-Public----------------------------------------------------------------------
// Name: SetPC(uint16_t PC)
//-----------------------------------------------------------------------------
void MC_Processor6502::SetPC(uint16_t PC)
{
	m_registers.pc = PC;
}
//-Public----------------------------------------------------------------------
// Name: GetRegisters()
//-----------------------------------------------------------------------------
Registers6502 MC_Processor6502::GetRegisters()
{
	return m_registers;
}
//-Public----------------------------------------------------------------------
// Name: SetRegisters(Registers6502 Registers)
//-----------------------------------------------------------------------------
void MC_Processor6502::SetRegisters(Registers6502 Registers)
{
	m_registers = Registers;
}
//-Public----------------------------------------------------------------------
// Name: RunOneOp()
//-----------------------------------------------------------------------------
bool MC_Processor6502::RunOneOp()
{
	if (!m_registers.IllegalOpcode) {
		m_Debug.pc = m_registers.pc;
		m_Instruction.OpCode = MemoryRead(m_registers.pc++);					// fetch
		m_Instruction.instr = m_InstrTbl[m_Instruction.OpCode];					// decode
		m_Debug.TotalCycles = Exec(m_Instruction.instr);						// execute
		m_Debug.Registers = m_registers;
		m_Debug.ExCycles = m_Clock.ExCycles;
		m_Debug.Updated = true;
		m_TotalCyclesPerSec += m_Debug.TotalCycles;
		m_CrashDump.Info[m_CrashDump.Index] = m_Debug;
		m_CrashDump.Index++;
		if (m_CrashDump.Index >= CrashDumpSize) {
			m_CrashDump.Index = 0;
		}
	} else {
		m_registers.IllegalOpcode = false;
	}
	return m_registers.IllegalOpcode;
}
//-Public----------------------------------------------------------------------
// Name: RunCode(int32_t cyclesRemaining, uint64_t& cycleCount, CycleMethod cycleMethod)
//-----------------------------------------------------------------------------
void MC_Processor6502::RunCode(int32_t cyclesRemaining, uint64_t& cycleCount, CycleMethod cycleMethod)
{
	uint8_t Cycle;

	while (cyclesRemaining > 0 && !m_registers.IllegalOpcode) {
		m_Instruction.OpCode = MemoryRead(m_registers.pc++);					// fetch
		m_Instruction.instr = m_InstrTbl[m_Instruction.OpCode];					// decode
		Cycle = Exec(m_Instruction.instr);										// execute
		cycleCount += Cycle;
		cyclesRemaining -= cycleMethod == CycleMethod::CYCLE_COUNT ? Cycle : 1;	// cycleMethod == INST_COUNT
	}
}
//-Public----------------------------------------------------------------------
// Name: DebugInfo(uint8_t * MemoryMap)
//-----------------------------------------------------------------------------
void MC_Processor6502::DebugInfo(uint8_t * MemoryMap)
{
	char tmpstr[DebugLineLen];

	Disassemble(tmpstr, sizeof(tmpstr), MemoryMap, &m_Debug.pc);
	printf("%s A=%02X X=%02X Y=%02X SP=$%04X Cycles=%d(%d) ", tmpstr, m_Debug.Registers.A, m_Debug.Registers.X, m_Debug.Registers.Y, 0x0100 + m_Debug.Registers.sp, m_Debug.TotalCycles, m_Debug.ExCycles);
	SHOW(uint8_t, m_Debug.Registers.status);
}
//-Protected-------------------------------------------------------------------
// Name: DebugCrashInfo(uint8_t* MemoryMap)
//-----------------------------------------------------------------------------
void MC_Processor6502::DebugCrashInfo(uint8_t* MemoryMap)
{
	uint16_t index;
	char tmpstr[DebugLineLen];

	PrintHexDump16Bit("Crash Memory Dump $0000-$03FF", MemoryMap, 0x0400, 0);
	index = m_CrashDump.Index;
	index++;
	if (index >= CrashDumpSize) {
		index = 0;
	}
	printf("\r\nCrash Debug Info\r\n");
	printf("----------------------------------------------------------\r\n");
	while (index != m_CrashDump.Index) {
		if (m_CrashDump.Info[index].Updated) {
			Disassemble(tmpstr, sizeof(tmpstr), MemoryMap, &m_CrashDump.Info[index].pc);
			printf("%s A=%02X X=%02X Y=%02X SP=$%04X Cycles=%d(%d) ", tmpstr, m_CrashDump.Info[index].Registers.A, m_CrashDump.Info[index].Registers.X, m_CrashDump.Info[index].Registers.Y, 0x0100 + m_CrashDump.Info[index].Registers.sp, m_CrashDump.Info[index].TotalCycles, m_CrashDump.Info[index].ExCycles);
			SHOW(uint8_t, m_CrashDump.Info[index].Registers.status);
		}
		index++;
		if (index >= CrashDumpSize) {
			index = 0;
		}
	}
}














//*****************************************************************************  
// Protected Code
//*****************************************************************************

//-Protected-------------------------------------------------------------------
// Name: Initialize()
//-----------------------------------------------------------------------------
void MC_Processor6502::Initialize()
{
	memset(&m_Instruction, 0, sizeof(m_Instruction));
	memset(&m_registers, 0, sizeof(m_registers));
	memset(&m_CrashDump, 0, sizeof(m_CrashDump));
	memset(&m_InstrTbl, 0, sizeof(m_InstrTbl));
	memset(&m_Debug, 0, sizeof(m_Debug));
	memset(&m_Clock, 0, sizeof(m_Clock));
	MemoryRead = nullptr;
	MemoryWrite = nullptr;
	m_TotalCyclesPerSec = 0;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ACC()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ACC()
{
	return 0;																	// not used
}
//-Protected-------------------------------------------------------------------
// Name: Addr_IMM()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_IMM()
{
	return m_registers.pc++;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ABS()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ABS()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t addr;

	addrL = MemoryRead(m_registers.pc++);
	addrH = MemoryRead(m_registers.pc++);
	addr = addrL + (addrH << 8);
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ZER()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ZER()
{
	return MemoryRead(m_registers.pc++);
}
//-Protected-------------------------------------------------------------------
// Name: Addr_IMP()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_IMP()
{
	return 0;																	// not used
}
//-Protected-------------------------------------------------------------------
// Name: Addr_REL()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_REL()
{
	uint16_t offset;
	uint16_t addr;

	offset = (uint16_t)MemoryRead(m_registers.pc++);
	if (offset & 0x80)
		offset |= 0xFF00;

	addr = m_registers.pc + (int16_t)offset;
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ABI()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ABI()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t effL;
	uint16_t effH;
	uint16_t abs;
	uint16_t addr;

	addrL = MemoryRead(m_registers.pc++);
	addrH = MemoryRead(m_registers.pc++);
	abs = (addrH << 8) | addrL;
	effL = MemoryRead(abs);
#ifndef CMOS_INDIRECT_JMP_FIX
	effH = MemoryRead((abs & 0xFF00) + ((abs + 1) & 0x00FF));
#else
	effH = Read(abs + 1);
#endif
	addr = effL + 0x100 * effH;
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ZEX()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ZEX()
{
	uint16_t addr = (MemoryRead(m_registers.pc++) + m_registers.X) % 256;
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ZEY()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ZEY()
{
	uint16_t addr = (MemoryRead(m_registers.pc++) + m_registers.Y) % 256;
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ABX()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ABX()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;

	addrL = MemoryRead(m_registers.pc++);
	addrH = MemoryRead(m_registers.pc++);
	addr = addrL + (addrH << 8) + m_registers.X;
	if (m_Clock.CanHaveExCycles) {
		bool CrossedPageBoundary = ((addrL + (addrH << 8)) ^ m_registers.X) >> 8;
		if (CrossedPageBoundary) {
			m_Clock.ExCycles++;
		}
	}
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_ABY()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_ABY()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;

	addrL = MemoryRead(m_registers.pc++);
	addrH = MemoryRead(m_registers.pc++);
	addr = addrL + (addrH << 8) + m_registers.Y;
	if (m_Clock.CanHaveExCycles) {
		bool CrossedPageBoundary = ((addrL + (addrH << 8)) ^ m_registers.Y) >> 8;
		if (CrossedPageBoundary) {
			m_Clock.ExCycles++;
		}
	}
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_INX()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_INX()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;

	zeroL = (MemoryRead(m_registers.pc++) + m_registers.X) % 256;
	zeroH = (zeroL + 1) % 256;
	addr = MemoryRead(zeroL) + (MemoryRead(zeroH) << 8);
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: Addr_INY()
//-----------------------------------------------------------------------------
uint16_t MC_Processor6502::Addr_INY()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;

	zeroL = MemoryRead(m_registers.pc++);
	zeroH = (zeroL + 1) % 256;
	addr = MemoryRead(zeroL) + (MemoryRead(zeroH) << 8) + m_registers.Y;
	if (m_Clock.CanHaveExCycles) {
		bool CrossedPageBoundary = ((MemoryRead(zeroL) + (MemoryRead(zeroH) << 8)) ^ m_registers.Y) >> 8;
		if (CrossedPageBoundary) {
			m_Clock.ExCycles++;
		}
	}
	return addr;
}
//-Protected-------------------------------------------------------------------
// Name: StackPush(uint8_t byte)
//-----------------------------------------------------------------------------
void MC_Processor6502::StackPush(uint8_t byte)
{
	MemoryWrite(0x0100 + m_registers.sp, byte);
	if (m_registers.sp == 0x00)
		m_registers.sp = 0xFF;

	else m_registers.sp--;
}
//-Protected-------------------------------------------------------------------
// Name: StackPop()
//-----------------------------------------------------------------------------
uint8_t MC_Processor6502::StackPop()
{
	if (m_registers.sp == 0xFF)
		m_registers.sp = 0x00;
	else
		m_registers.sp++;

	return MemoryRead(0x0100 + m_registers.sp);
}
//-Protected-------------------------------------------------------------------
// Name: Exec(Instr& instr)
//-----------------------------------------------------------------------------
uint8_t MC_Processor6502::Exec(Instr& instr)
{
	m_Clock.CanHaveExCycles = instr.CanHaveExCycles;
	m_Clock.Cycles = instr.Cycles;
	m_Clock.ExCycles = 0;
	uint16_t src = (this->*instr.AddrMode)();
	(this->*instr.Code)(src);
	if (m_Clock.CanHaveExCycles) {
		m_Clock.TotalCycles = m_Clock.Cycles + m_Clock.ExCycles;
	} else {
		m_Clock.TotalCycles = m_Clock.Cycles;
	}
	return m_Clock.TotalCycles;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ILLEGAL(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ILLEGAL(uint16_t src)
{
	m_registers.IllegalOpcode = true;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ADC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ADC(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	unsigned int tmp = m + m_registers.A + (IF_CARRY() ? 1 : 0);
	SET_ZERO(!(tmp & 0xFF));
	if (IF_DECIMAL()) {
		if (((m_registers.A & 0xF) + (m & 0xF) + (IF_CARRY() ? 1 : 0)) > 9)
			tmp += 6;

		SET_NEGATIVE(tmp & 0x80);
		SET_OVERFLOW(!((m_registers.A ^ m) & 0x80) && ((m_registers.A ^ tmp) & 0x80));
		if (tmp > 0x99) {
			tmp += 96;
		}
		SET_CARRY(tmp > 0x99);
	} else {
		SET_NEGATIVE(tmp & 0x80);
		SET_OVERFLOW(!((m_registers.A ^ m) & 0x80) && ((m_registers.A ^ tmp) & 0x80));
		SET_CARRY(tmp > 0xFF);
	}
	m_registers.A = tmp & 0xFF;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_AND(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_AND(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	uint8_t res = m & m_registers.A;
	SET_NEGATIVE(res & 0x80);
	SET_ZERO(!res);
	m_registers.A = res;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ASL(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ASL(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	SET_CARRY(m & 0x80);
	m <<= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	MemoryWrite(src, m);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ASL_ACC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ASL_ACC(uint16_t src)
{
	uint8_t m = m_registers.A;
	SET_CARRY(m & 0x80);
	m <<= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BCC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BCC(uint16_t src)
{
	if (!IF_CARRY()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BCS(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BCS(uint16_t src)
{
	if (IF_CARRY()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BEQ(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BEQ(uint16_t src)
{
	if (IF_ZERO()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BIT(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BIT(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	uint8_t res = m & m_registers.A;
	SET_NEGATIVE(res & 0x80);
	m_registers.status = (m_registers.status & 0x3F) | (uint8_t)(m & 0xC0);
	SET_ZERO(!res);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BMI(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BMI(uint16_t src)
{
	if (IF_NEGATIVE()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name Op_BNE(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BNE(uint16_t src)
{
	if (!IF_ZERO()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BPL(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BPL(uint16_t src)
{
	if (!IF_NEGATIVE()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BRK(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BRK(uint16_t src)
{
	m_registers.pc++;
	StackPush((m_registers.pc >> 8) & 0xFF);
	StackPush(m_registers.pc & 0xFF);
	StackPush(m_registers.status | FLAGBREAK);
	SET_INTERRUPT(1);
	m_registers.pc = (MemoryRead(s_irqVectorH) << 8) + MemoryRead(s_irqVectorL);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BVC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BVC(uint16_t src)
{
	if (!IF_OVERFLOW()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_BVS(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_BVS(uint16_t src)
{
	if (IF_OVERFLOW()) {
		if (m_Clock.CanHaveExCycles) {
			bool CrossedPageBoundary = (m_registers.pc ^ src) >> 8;
			if (CrossedPageBoundary) {
				m_Clock.ExCycles++;
			}
		}
		m_Clock.ExCycles++;
		m_registers.pc = src;
	}
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CLC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CLC(uint16_t src)
{
	SET_CARRY(0);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CLD(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CLD(uint16_t src)
{
	SET_DECIMAL(0);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CLI(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CLI(uint16_t src)
{
	SET_INTERRUPT(0);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CLV(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CLV(uint16_t src)
{
	SET_OVERFLOW(0);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CMP(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CMP(uint16_t src)
{
	unsigned int tmp = m_registers.A - MemoryRead(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CPX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CPX(uint16_t src)
{
	unsigned int tmp = m_registers.X - MemoryRead(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_CPY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_CPY(uint16_t src)
{
	unsigned int tmp = m_registers.Y - MemoryRead(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_DEC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_DEC(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	m = (m - 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	MemoryWrite(src, m);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_DEX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_DEX(uint16_t src)
{
	uint8_t m = m_registers.X;
	m = (m - 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.X = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_DEY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_DEY(uint16_t src)
{
	uint8_t m = m_registers.Y;
	m = (m - 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.Y = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_EOR(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_EOR(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	m = m_registers.A ^ m;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_INC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_INC(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	m = (m + 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	MemoryWrite(src, m);
}
//-Protected-------------------------------------------------------------------
// Name: Op_INX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_INX(uint16_t src)
{
	uint8_t m = m_registers.X;
	m = (m + 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.X = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_INY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_INY(uint16_t src)
{
	uint8_t m = m_registers.Y;
	m = (m + 1) % 256;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.Y = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_JMP(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_JMP(uint16_t src)
{
	m_registers.pc = src;
}
//-Protected-------------------------------------------------------------------
// Name: Op_JSR(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_JSR(uint16_t src)
{
	m_registers.pc--;
	StackPush((m_registers.pc >> 8) & 0xFF);
	StackPush(m_registers.pc & 0xFF);
	m_registers.pc = src;
}
//-Protected-------------------------------------------------------------------
// Name: Op_LDA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_LDA(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_LDX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_LDX(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.X = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_LDY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_LDY(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.Y = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_LSR(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_LSR(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	MemoryWrite(src, m);
}
//-Protected-------------------------------------------------------------------
// Name: Op_LSR_ACC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_LSR_ACC(uint16_t src)
{
	uint8_t m = m_registers.A;
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	m_registers.A = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_NOP(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_NOP(uint16_t src)
{
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ORA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ORA(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	m = m_registers.A | m;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
}
//-Protected-------------------------------------------------------------------
// Name: Op_PHA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_PHA(uint16_t src)
{
	StackPush(m_registers.A);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_PHP(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_PHP(uint16_t src)
{
	StackPush(m_registers.status | FLAGBREAK);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_PLA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_PLA(uint16_t src)
{
	m_registers.A = StackPop();
	SET_NEGATIVE(m_registers.A & 0x80);
	SET_ZERO(!m_registers.A);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_PLP(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_PLP(uint16_t src)
{
	m_registers.status = StackPop();
	SET_CONSTANT(1);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ROL(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ROL(uint16_t src)
{
	uint16_t m = MemoryRead(src);
	m <<= 1;
	if (IF_CARRY())
		m |= 0x01;

	SET_CARRY(m > 0xFF);
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	MemoryWrite(src, (uint8_t)m);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ROL_ACC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ROL_ACC(uint16_t src)
{
	uint16_t m = m_registers.A;
	m <<= 1;
	if (IF_CARRY())
		m |= 0x01;

	SET_CARRY(m > 0xFF);
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = (uint8_t)m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ROR(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ROR(uint16_t src)
{
	uint16_t m = MemoryRead(src);
	if (IF_CARRY())
		m |= 0x100;

	SET_CARRY(m & 0x01);
	m >>= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	MemoryWrite(src, (uint8_t)m);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_ROR_ACC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_ROR_ACC(uint16_t src)
{
	uint16_t m = m_registers.A;
	if (IF_CARRY())
		m |= 0x100;

	SET_CARRY(m & 0x01);
	m >>= 1;
	m &= 0xFF;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = (uint8_t)m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_RTI(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_RTI(uint16_t src)
{
	uint8_t lo, hi;

	m_registers.status = StackPop();
	lo = StackPop();
	hi = StackPop();
	m_registers.pc = (hi << 8) | lo;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_RTS(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_RTS(uint16_t src)
{
	uint8_t lo, hi;

	lo = StackPop();
	hi = StackPop();
	m_registers.pc = ((hi << 8) | lo) + 1;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_SBC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_SBC(uint16_t src)
{
	uint8_t m = MemoryRead(src);
	unsigned int tmp = m_registers.A - m - (IF_CARRY() ? 0 : 1);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
	SET_OVERFLOW(((m_registers.A ^ tmp) & 0x80) && ((m_registers.A ^ m) & 0x80));
	if (IF_DECIMAL()) {
		if (((m_registers.A & 0x0F) - (IF_CARRY() ? 0 : 1)) < (m & 0x0F))
			tmp -= 6;

		if (tmp > 0x99) {
			tmp -= 0x60;
		}
	}
	SET_CARRY(tmp < 0x100);
	m_registers.A = (tmp & 0xFF);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_SEC(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_SEC(uint16_t src)
{
	SET_CARRY(1);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_SED(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_SED(uint16_t src)
{
	SET_DECIMAL(1);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_SEI(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_SEI(uint16_t src)
{
	SET_INTERRUPT(1);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_STA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_STA(uint16_t src)
{
	MemoryWrite(src, m_registers.A);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_STX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_STX(uint16_t src)
{
	MemoryWrite(src, m_registers.X);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_STY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_STY(uint16_t src)
{
	MemoryWrite(src, m_registers.Y);
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TAX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TAX(uint16_t src)
{
	uint8_t m = m_registers.A;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.X = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TAY(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TAY(uint16_t src)
{
	uint8_t m = m_registers.A;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.Y = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TSX(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TSX(uint16_t src)
{
	uint8_t m = m_registers.sp;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.X = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TXA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TXA(uint16_t src)
{
	uint8_t m = m_registers.X;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TXS(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TXS(uint16_t src)
{
	m_registers.sp = m_registers.X;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Op_TYA(uint16_t src)
//-----------------------------------------------------------------------------
void MC_Processor6502::Op_TYA(uint16_t src)
{
	uint8_t m = m_registers.Y;
	SET_NEGATIVE(m & 0x80);
	SET_ZERO(!m);
	m_registers.A = m;
	return;
}
//-Protected-------------------------------------------------------------------
// Name: Disassemble(char *output, size_t outputsize, uint8_t *buffer, uint16_t *pc)
//-----------------------------------------------------------------------------
void MC_Processor6502::Disassemble(char* output, size_t outputsize, uint8_t* buffer, uint16_t* pc)
{
	char opcode_repr[DebugSubLineLen], hex_dump[DebugSubLineLen];
	int len = 0;
	uint8_t entry = 0;
	uint8_t byte_operand;
	uint16_t word_operand = 0;
	uint16_t current_addr = *pc;
	uint8_t opcode = buffer[current_addr];
	const char* mnemonic;

	opcode_repr[0] = '\0';
	hex_dump[0] = '\0';
	output[0] = '\0';
	if (m_InstrTbl[opcode].Mnemonic != nullptr && m_InstrTbl[opcode].AddressingMode != ILLEGAL) {
		entry = opcode;
	} else {
		snprintf(opcode_repr, sizeof(opcode_repr), ".byte $%02X", opcode);
		snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X ", current_addr, opcode);
		snprintf(output, outputsize, "%-16s%-16s  INVALID OPCODE", hex_dump, opcode_repr);
		return;
	}
	mnemonic = m_InstrTbl[entry].Mnemonic;
	snprintf(hex_dump, sizeof(hex_dump), "$%04X", current_addr);
	switch (m_InstrTbl[entry].AddressingMode) {
		case IMMED:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s #$%02X", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case ABSOL:
			word_operand = LOAD_WORD(buffer, *pc);
			*pc += 2;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X%02X", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X%02X ", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
			break;
		case ZEROP:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case IMPLI:
			snprintf(opcode_repr, sizeof(opcode_repr), "%s", mnemonic);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X ", current_addr, opcode);
			break;
		case INDIA:
			word_operand = LOAD_WORD(buffer, *pc);
			*pc += 2;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s ($%02X%02X)", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X%02X ", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
			break;
		case ABSIX:
			word_operand = LOAD_WORD(buffer, *pc);
			*pc += 2;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X%02X,X", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X%02X ", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
			break;
		case ABSIY:
			word_operand = LOAD_WORD(buffer, *pc);
			*pc += 2;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X%02X,Y", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X%02X ", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
			break;
		case ZEPIX:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X,X", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case ZEPIY:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%02X,Y", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case INDIN:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s ($%02X,X)", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case ININD:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			snprintf(opcode_repr, sizeof(opcode_repr), "%s ($%02X),Y", mnemonic, byte_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case RELAT:
			byte_operand = buffer[*pc + 1];
			*pc += 1;
			word_operand = current_addr + 2;
			if (byte_operand > 0x7Fu) {
				word_operand -= ((~byte_operand & 0x7Fu) + 1);
			} else {
				word_operand += byte_operand & 0x7Fu;
			}
			snprintf(opcode_repr, sizeof(opcode_repr), "%s $%04X", mnemonic, word_operand);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X %02X ", current_addr, opcode, byte_operand);
			break;
		case ACCUM:
			snprintf(opcode_repr, sizeof(opcode_repr), "%s A", mnemonic);
			snprintf(hex_dump, sizeof(hex_dump), "$%04X  %02X ", current_addr, opcode);
			break;
		default:
			// Will not happen since each entry in opcode_table has address mode set
			break;
	}
	len = snprintf(output, outputsize, "%-16s%-16s ", hex_dump, opcode_repr);
	output += len;
}
//-Protected-------------------------------------------------------------------
// Name: :PrintByteAsBits(char val)
//-----------------------------------------------------------------------------
void MC_Processor6502::PrintByteAsBits(char val)
{
	for (int i = 7; 0 <= i; i--) {
		printf("%c", (val & (1 << i)) ? StatusBits[i] : '.');
	}
}
//-Protected-------------------------------------------------------------------
// Name: PrintBits(const char* ty, const char* val, unsigned char* bytes, size_t num_bytes)
//-----------------------------------------------------------------------------
void MC_Processor6502::PrintBits(const char* ty, const char* val, unsigned char* bytes, size_t num_bytes)
{
	printf("StatusFlags [ ");
	for (size_t i = 0; i < num_bytes; i++) {
		PrintByteAsBits(bytes[i]);
		printf(" ");
	}
	printf("]\r\n");
}
//-Protected-------------------------------------------------------------------
// Name: PrintHexDump16Bit(const char* desc, void* addr, long len, long offset)
//-----------------------------------------------------------------------------
void MC_Processor6502::PrintHexDump16Bit(const char* desc, void* addr, long len, long offset)
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
	} else if (len < 0) {
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
