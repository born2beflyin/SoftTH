/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2005-2012 Keijo Ruotsalainen, www.kegetys.fi
              2014-     C. Justin Ratcliff, www.softth.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include "helper.h"


extern HINSTANCE hLibD3D10_1;



#ifndef _WIN64
//#if 1

#ifdef _WIN64
#define STACK_BASE rbp // x64: rbp is stack base
#else
#define STACK_BASE ebp // x86: ebp is stack base
#endif

// Passthrough exports
#pragma warning (disable : 4731)
#define DEXPORT(hLib, x) _declspec(dllexport) void x(void) {\
	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
	__asm {pop STACK_BASE};\
	__asm {jmp foo};\
	}

#define DEXPORTD(hLib, x) _declspec(dllexport) void x(void) {\
  dbg("DEXPORT: %s - %s", #hLib, #x);\
	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
	__asm {pop STACK_BASE};\
	__asm {jmp foo};\
	}

extern "C" {
/* Direct3D 10.1 */
DEXPORTD(hLibD3D10_1, D3D10CompileEffectFromMemory          );
DEXPORTD(hLibD3D10_1, D3D10CompileShader                    );
DEXPORTD(hLibD3D10_1, D3D10CreateBlob                       );
//DEXPORTD(hLibD3D10_1, D3D10CreateDevice1                    );
//DEXPORTD(hLibD3D10_1, D3D10CreateDeviceAndSwapChain1        );
DEXPORTD(hLibD3D10_1, D3D10CreateEffectFromMemory           );
DEXPORTD(hLibD3D10_1, D3D10CreateEffectPoolFromMemory       );
DEXPORTD(hLibD3D10_1, D3D10CreateStateBlock                 );
DEXPORTD(hLibD3D10_1, D3D10DisassembleEffect                );
DEXPORTD(hLibD3D10_1, D3D10DisassembleShader                );
DEXPORTD(hLibD3D10_1, D3D10GetGeometryShaderProfile         );
DEXPORTD(hLibD3D10_1, D3D10GetInputAndOutputSignatureBlob   );
DEXPORTD(hLibD3D10_1, D3D10GetInputSignatureBlob            );
DEXPORTD(hLibD3D10_1, D3D10GetOutputSignatureBlob           );
DEXPORTD(hLibD3D10_1, D3D10GetPixelShaderProfile            );
DEXPORTD(hLibD3D10_1, D3D10GetShaderDebugInfo               );
DEXPORTD(hLibD3D10_1, D3D10GetVersion                       );
DEXPORTD(hLibD3D10_1, D3D10GetVertexShaderProfile           );
DEXPORTD(hLibD3D10_1, D3D10PreprocessShader                 );
DEXPORTD(hLibD3D10_1, D3D10ReflectShader                    );
DEXPORTD(hLibD3D10_1, D3D10RegisterLayers                   );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskDifference         );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskDisableAll         );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskDisableCapture     );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskEnableAll          );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskEnableCapture      );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskGetSetting         );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskIntersect          );
DEXPORTD(hLibD3D10_1, D3D10StateBlockMaskUnion              );
DEXPORTD(hLibD3D10_1, RevertToOldImplementation             );
}
#pragma warning (default : 4731)

#endif
