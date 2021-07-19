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
 // File: MC_FileSystem6502.h: interface for the MC_FileSystem6502 class.
 // Desc: Application interface for the MC_FileSystem6502 class.
 //-----------------------------------------------------------------------------
#ifndef MC_FileSystem6502_H
    #define MC_FileSystem6502_H

#pragma once

#include "IncludeLibraries.h"

//-----------------------------------------------------------------------------
// Const
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Struct
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name: class MC_FileSystem6502
// Desc: Application class
//-----------------------------------------------------------------------------
class MC_FileSystem6502
{

    public:

    protected:

    private:

        //-----------------------------------------------------------------------------

    public:
							MC_FileSystem6502();
        virtual				~MC_FileSystem6502();
        void				LoadFiles();
        void 				MemoryLoadfromFlash(const char* FileName[] ,uint16_t FileItems ,uint16_t address ,uint16_t MemSize);
        uint32_t 			Crc32(const uint8_t* buf, uint32_t buflen);

    protected:
        uint16_t 			Hex2Dec(std::string s);

    private:

};
//-----------------------------------------------------------------------------
#endif // MC_FileSystem6502_H
