/**********************************************************************************
* MIT License																	  *
*																				  *
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
 // MC_Processor6502.h is base on mos6502.h & Disassembler dcc6502.h
 //-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// File: MC_Processor6502.h: interface for the MC_Processor6502 class.
// Desc: Application interface for the MC_Processor6502 class.
//-----------------------------------------------------------------------------
#ifndef MC_Processor6502_H
	#define MC_Processor6502_H

#pragma once

#include "IncludeLibraries.h"

//-----------------------------------------------------------------------------
// Const
//-----------------------------------------------------------------------------
#define CrashDumpSize		32
#define DebugLineLen		512
#define DebugSubLineLen		DebugLineLen / 2

#define FLAGNEGATIVE		0x80
#define FLAGOVERFLOW		0x40
#define FLAGCONSTANT		0x20
#define FLAGBREAK			0x10
#define FLAGDECIMAL			0x08
#define FLAGINTERRUPT		0x04
#define FLAGZERO			0x02
#define FLAGCARRY			0x01

#define SET_NEGATIVE(x) (x ? (m_registers.status |= FLAGNEGATIVE) : (m_registers.status &= (~FLAGNEGATIVE)) )
#define SET_OVERFLOW(x) (x ? (m_registers.status |= FLAGOVERFLOW) : (m_registers.status &= (~FLAGOVERFLOW)) )
#define SET_CONSTANT(x) (x ? (m_registers.status |= FLAGCONSTANT) : (m_registers.status &= (~FLAGCONSTANT)) )
#define SET_BREAK(x) (x ? (m_registers.status |= FLAGBREAK) : (m_registers.status &= (~FLAGBREAK)) )
#define SET_DECIMAL(x) (x ? (m_registers.status |= FLAGDECIMAL) : (m_registers.status &= (~FLAGDECIMAL)) )
#define SET_INTERRUPT(x) (x ? (m_registers.status |= FLAGINTERRUPT) : (m_registers.status &= (~FLAGINTERRUPT)) )
#define SET_ZERO(x) (x ? (m_registers.status |= FLAGZERO) : (m_registers.status &= (~FLAGZERO)) )
#define SET_CARRY(x) (x ? (m_registers.status |= FLAGCARRY) : (m_registers.status &= (~FLAGCARRY)) )

#define IF_NEGATIVE() ((m_registers.status & FLAGNEGATIVE) ? true : false)
#define IF_OVERFLOW() ((m_registers.status & FLAGOVERFLOW) ? true : false)
#define IF_CONSTANT() ((m_registers.status & FLAGCONSTANT) ? true : false)
#define IF_BREAK() ((m_registers.status & FLAGBREAK) ? true : false)
#define IF_DECIMAL() ((m_registers.status & FLAGDECIMAL) ? true : false)
#define IF_INTERRUPT() ((m_registers.status & FLAGINTERRUPT) ? true : false)
#define IF_ZERO() ((m_registers.status & FLAGZERO) ? true : false)
#define IF_CARRY() ((m_registers.status & FLAGCARRY) ? true : false)

#define SHOW(T,V) do { T x = V; PrintBits(#T, #V, (unsigned char*) &x, sizeof(x)); } while(0)

/* Helper macros for disassemble() function */
#define HIGH_PART(val) (((val) >> 8) & 0xFFu)
#define LOW_PART(val) ((val) & 0xFFu)
#define LOAD_WORD(buffer, current_pc) ((uint16_t)buffer[(current_pc) + 1] | (((uint16_t)buffer[(current_pc) + 2]) << 8))


//-----------------------------------------------------------------------------
// Struct
//-----------------------------------------------------------------------------

typedef struct Cycles6502Info
{
	bool					CanHaveExCycles;
	uint8_t					Cycles;
	uint8_t					ExCycles;
	uint8_t					TotalCycles;
} _Cycles6502Info;

typedef struct Registers6502
{
	uint8_t					A;				// accumulator
	uint8_t					X;				// X-index
	uint8_t					Y;				// Y-index
	uint8_t					sp;				// stack pointer
	uint16_t				pc;				// program counter
	uint8_t					status;			// status register
	bool					IllegalOpcode;	// Illegal Op code Flag;
} _Registers6502;

enum Flags {
	CarryFlag = 0,
	ZeroFlag = 1,
	InterruptDisableFlag = 2,
	DecimalFlag = 3,
	BreakFlag = 4,
	UnknownFlag = 5,
	OverflowFlag = 6,
	NegativeFlag = 7
};

enum {
	IMMED = 0,                                      /* Immediate */
	ABSOL,                                          /* Absolute */
	ZEROP,                                          /* Zero Page */
	IMPLI,                                          /* Implied */
	INDIA,                                          /* Indirect Absolute */
	ABSIX,                                          /* Absolute indexed with X */
	ABSIY,                                          /* Absolute indexed with Y */
	ZEPIX,                                          /* Zero page indexed with X */
	ZEPIY,                                          /* Zero page indexed with Y */
	INDIN,                                          /* Indexed indirect (with X) */
	ININD,                                          /* Indirect indexed (with Y) */
	RELAT,                                          /* Relative */
	ACCUM,                                          /* Accumulator */
	ILLEGAL
};

//-----------------------------------------------------------------------------
// Name: class MC_Processor6502
// Desc: Application class
//-----------------------------------------------------------------------------
class MC_Processor6502
{

	public:

		enum class			CycleMethod { INST_COUNT, CYCLE_COUNT };

		typedef void		(MC_Processor6502::* CodeExec)(uint16_t);
		typedef uint16_t	(MC_Processor6502::* AddrExec)();

		typedef struct Instr {
			AddrExec		AddrMode;
			CodeExec		Code;
			uint8_t			Cycles;
			bool			CanHaveExCycles;
			const char*		Mnemonic;
			uint8_t			AddressingMode;
		} _Instr;

		typedef struct DebugInfo6502 {
			uint16_t		pc;
			Registers6502	Registers;
			uint8_t			TotalCycles;
			uint8_t			ExCycles;
			bool			Updated;
		} _DebugInfo6502;

		typedef struct CrashDump6502
		{
			DebugInfo6502   Info[CrashDumpSize];
			uint16_t		Index;

		} _CrashDump6502;

		typedef struct Instruction
		{
			uint8_t			OpCode;
			Instr			instr;
		} _Instruction;

		CrashDump6502		m_CrashDump;
		DebugInfo6502		m_Debug;
		uint64_t			m_TotalCyclesPerSec;

	protected:
		Instr				m_InstrTbl[256];
		Instruction			m_Instruction;
		Registers6502		m_registers;
		Cycles6502Info		m_Clock;

		// IRQ, reset, NMI vectors
		static const uint16_t s_irqVectorH = 0xFFFF;
		static const uint16_t s_irqVectorL = 0xFFFE;
		static const uint16_t s_rstVectorH = 0xFFFD;
		static const uint16_t s_rstVectorL = 0xFFFC;
		static const uint16_t s_nmiVectorH = 0xFFFB;
		static const uint16_t s_nmiVectorL = 0xFFFA;

		// read/write callbacks
		typedef void		(*BusWrite)(uint16_t, uint8_t);
		typedef uint8_t		(*BusRead)(uint16_t);
		BusRead				MemoryRead;
		BusWrite			MemoryWrite;

	private:

	//-----------------------------------------------------------------------------

	public:
							MC_Processor6502();
							MC_Processor6502(BusRead r, BusWrite w);
		virtual				~MC_Processor6502();
		void				NMI();
		void				IRQ();
		void				Reset();
		uint16_t			GetPC();
		void				SetPC(uint16_t PC);
		Registers6502		GetRegisters();
		void				SetRegisters(Registers6502 Registers);
		bool				RunOneOp();
		void				RunCode(int32_t cycles, uint64_t& cycleCount, CycleMethod cycleMethod = CycleMethod::CYCLE_COUNT);
		void				DebugInfo(uint8_t* MemoryMap);
		void				DebugCrashInfo(uint8_t* MemoryMap);

	protected:
		void				Initialize();
		uint8_t				Exec(Instr& instr);
		// addressing modes
		uint16_t			Addr_ACC(); // ACCUMULATOR
		uint16_t			Addr_IMM(); // IMMEDIATE
		uint16_t			Addr_ABS(); // ABSOLUTE
		uint16_t			Addr_ZER(); // ZERO PAGE
		uint16_t			Addr_ZEX(); // INDEXED-X ZERO PAGE
		uint16_t			Addr_ZEY(); // INDEXED-Y ZERO PAGE
		uint16_t			Addr_ABX(); // INDEXED-X ABSOLUTE
		uint16_t			Addr_ABY(); // INDEXED-Y ABSOLUTE
		uint16_t			Addr_IMP(); // IMPLIED
		uint16_t			Addr_REL(); // RELATIVE
		uint16_t			Addr_INX(); // INDEXED-X INDIRECT
		uint16_t			Addr_INY(); // INDEXED-Y INDIRECT
		uint16_t			Addr_ABI(); // ABSOLUTE INDIRECT
		// opcodes (grouped as per datasheet)
		void				Op_ADC(uint16_t src);
		void				Op_AND(uint16_t src);
		void				Op_ASL(uint16_t src);
		void				Op_ASL_ACC(uint16_t src);
		void				Op_BCC(uint16_t src);
		void				Op_BCS(uint16_t src);
		void				Op_BEQ(uint16_t src);
		void				Op_BIT(uint16_t src);
		void				Op_BMI(uint16_t src);
		void				Op_BNE(uint16_t src);
		void				Op_BPL(uint16_t src);
		void				Op_BRK(uint16_t src);
		void				Op_BVC(uint16_t src);
		void				Op_BVS(uint16_t src);
		void				Op_CLC(uint16_t src);
		void				Op_CLD(uint16_t src);
		void				Op_CLI(uint16_t src);
		void				Op_CLV(uint16_t src);
		void				Op_CMP(uint16_t src);
		void				Op_CPX(uint16_t src);
		void				Op_CPY(uint16_t src);
		void				Op_DEC(uint16_t src);
		void				Op_DEX(uint16_t src);
		void				Op_DEY(uint16_t src);
		void				Op_EOR(uint16_t src);
		void				Op_INC(uint16_t src);
		void				Op_INX(uint16_t src);
		void				Op_INY(uint16_t src);
		void				Op_JMP(uint16_t src);
		void				Op_JSR(uint16_t src);
		void				Op_LDA(uint16_t src);
		void				Op_LDX(uint16_t src);
		void				Op_LDY(uint16_t src);
		void				Op_LSR(uint16_t src);
		void				Op_LSR_ACC(uint16_t src);
		void				Op_NOP(uint16_t src);
		void				Op_ORA(uint16_t src);
		void				Op_PHA(uint16_t src);
		void				Op_PHP(uint16_t src);
		void				Op_PLA(uint16_t src);
		void				Op_PLP(uint16_t src);
		void				Op_ROL(uint16_t src);
		void				Op_ROL_ACC(uint16_t src);
		void				Op_ROR(uint16_t src);
		void				Op_ROR_ACC(uint16_t src);
		void				Op_RTI(uint16_t src);
		void				Op_RTS(uint16_t src);
		void				Op_SBC(uint16_t src);
		void				Op_SEC(uint16_t src);
		void				Op_SED(uint16_t src);
		void				Op_SEI(uint16_t src);
		void				Op_STA(uint16_t src);
		void				Op_STX(uint16_t src);
		void				Op_STY(uint16_t src);
		void				Op_TAX(uint16_t src);
		void				Op_TAY(uint16_t src);
		void				Op_TSX(uint16_t src);
		void				Op_TXA(uint16_t src);
		void				Op_TXS(uint16_t src);
		void				Op_TYA(uint16_t src);
		void				Op_ILLEGAL(uint16_t src);
		// stack operations
		inline void			StackPush(uint8_t byte);
		inline uint8_t		StackPop();

		void				Disassemble(char* output, size_t outputsize, uint8_t* buffer, uint16_t* pc);
		void				PrintByteAsBits(char val);
		void				PrintBits(const char* ty, const char* val, unsigned char* bytes, size_t num_bytes);
		void				PrintHexDump16Bit(const char* desc, void* addr, long len, long offset);

	private:

};
//-----------------------------------------------------------------------------
#endif // MC_Processor6502_H
