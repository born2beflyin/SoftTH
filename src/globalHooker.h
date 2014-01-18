/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2005-2012 Keijo Ruotsalainen, www.kegetys.fi

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

#ifndef _WIN32HOOKNEW_H_
#define _WIN32HOOKNEW_H_

#include <windows.h>

#define JMPDATALEN	5
#define STUBLEN	10	// Length of jumper stub data (original instructions + jmp)
//#define HOOK(x,y,z) (ULONG_PTR*) x, #y, #z , NULL, {NULL}, false, 0,
#define HOOK(x,y,z) (ULONG_PTR) x, #y, #z , NULL, {NULL}, false, 0,
#define HOOKEND 0, 0, 0, 0, 0, 0, 0,

typedef struct {
	ULONG_PTR	funcOver;	// Pointer to overridden function (user set)
  //DWORD funcOver;
	char	dll[256];	// Name of dll (user set)
	char	name[64];	// Name of original function (user set)
	ULONG_PTR	funcOrig;	// Pointer to original function
  //DWORD funcOrig;
	union {
		BYTE	stub[STUBLEN+JMPDATALEN]; // Original data from function start
		FARPROC	stubCall; // this is called by hook function
	};
	bool	active;		// Hook successfully written?
	int		stublen;	// Length of stub data (without jmp)
} GHOOK;

BYTE* instructionCount(BYTE *func, int tcount);

void setHooks(GHOOK *hooks);
void unsetHooks(GHOOK *hooks);

#endif