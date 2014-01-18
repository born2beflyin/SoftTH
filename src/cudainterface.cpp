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

//
// Crappy cuda interface thing, wraps cuda "tasks" into threads
//

#ifdef USE_CUDA

#include "cudainterface.h"
#include "../helper.h"
#include <time.h>

#include <delayimp.h>
#pragma comment(lib, "delayimp")
#pragma comment( lib, "cudart.lib" )

volatile bool cudaInitialized = false;
CRITICAL_SECTION cs;

bool initCudart(void) {
	ONCE
		InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);

	// Attempt to delay-init cudart.dll
	if(!cudaInitialized) {
		cudaInitialized = true;
		dbg("Initializing CUDA, thread 0x%04X...", GetCurrentThreadId());
		__try {
			if(FAILED(__HrLoadAllImportsForDll("cudart.dll"))) {
			   ShowMessage("cudart.dll not found, CUDA runtime not installed");
			   exit(0);
			}
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			ShowMessage("cudart.dll not found, CUDA runtime not installed (Exception)");
			exit(0);
			return false;
		}
	}

	LeaveCriticalSection(&cs);
	//DeleteCriticalSection(&cs);
	return true;
}


// Return true if CUDA is in use (useless)
bool isCudaEnabled(void) {
	return cudaInitialized;
}

bool cudaCheckCudaCompatibility(LPDIRECT3D9 d3dh) {
	dbg("cudaCheckCudaCompatibility");
	initCudart();
	D3DADAPTER_IDENTIFIER9 id;
	int devID;
	ONCE {
		d3dh->GetAdapterIdentifier(D3DADAPTER_DEFAULT, NULL, &id);
		if(cudaD3D9GetDevice(&devID, id.DeviceName) != cudaSuccess) {
			ShowMessage("Primary display device does not support CUDA");
			return false;
		}

		// Get some information on device
		cudaDeviceProp dp;	ZeroMemory(&dp, sizeof(cudaDeviceProp));
		cudaGetDeviceProperties(&dp, devID);

		dbg("Using CUDA device ID %d (%s):", devID, dp.name);
		dbg("     %d multiprocessors, %dMHz", dp.multiProcessorCount, dp.clockRate/1000);
		dbg("     Memory: %dMB total, %dKB max shared per block", dp.totalGlobalMem/1024/1024, dp.sharedMemPerBlock/1024);
		dbg("     %s", dp.deviceOverlap?"Overlap supported":"Overlap not supported");
		return true;
	}
	return true;
}

// Add a direct3D surface to cuda
int directCuda::addSurface(LPDIRECT3DSURFACE9 newsurf) {
	if(thread)
		return -1;	// Cannot add if thread is already active

	surf.push_back(newsurf);
	int id = surf.size()-1;
	return id;
}

// Return surface with ID
LPDIRECT3DSURFACE9 directCuda::getSurface(DWORD id) {
	if(id>surf.size())
		return 0;
	return surf[id];
}

// Return ID by surface
int directCuda::getID(LPDIRECT3DSURFACE9 psurf) {
	for(DWORD i=0;i<surf.size();i++) {
		if(psurf == surf[i])
			return i;
	}
	return -1;	// Not there
}

// true if surf is registered
bool directCuda::validSurface(LPDIRECT3DSURFACE9 psurf) {
	return getID(psurf) != -1;
}

// Set task function pointer
bool directCuda::setTask(DIRECTCUDATASK* ptask, void *params) {
	// TODO: add check if task is currently being executed & return false
	taskParams = params;
	task = ptask;
	return true;
}

// Begin current task
bool directCuda::taskBegin(void) {
	if(!thread) {
		dbg("directCuda <%s>: taskBegin: thread not active", name);
		return false;
	}

	if(!task)
		return false;
	SetEvent(go);
	return true;
}

// Wait for task to end, return return value
void* directCuda::taskWait(void) {
	WaitForSingleObject(done, INFINITE);
	return taskRetval;
}

// Begin task & wait for it to complete
void* directCuda::taskExecute(void) {
	if(!thread) {
		dbg("directCuda <%s>: taskExecute: thread not active", name);
		return 0;
	}

	if(!taskBegin())
		return false;
	taskWait();
	return taskRetval;
}

directCuda::directCuda(LPDIRECT3D9 pd3d, LPDIRECT3DDEVICE9 pdev, char *pname) {
	dbg("directCuda::directCuda");
	initCudart();	// Loads cudart.dll

	// Initialize cudathread
	d3d = pd3d;
	dev = pdev;
	alive = false;
	thread = NULL;
	task = NULL;
	devID = -1;
	memHost = NULL;
	strncpy(name, pname, 256);

	//int uid = GetCurrentThreadId();	// "unique" IDs for events
	static int uid = 0;uid++;
	char goname[128], doname[128];
	sprintf(goname, "cudago_%d_%.64s", uid, name);
	sprintf(doname, "cudadone_%d_%.64s", uid, name);

	// Creathe message events & begin thread
	dbg("directCuda <%s>: Initializing...", name);
	go = CreateEvent(NULL, FALSE, FALSE, goname);
	done = CreateEvent(NULL, FALSE, FALSE, doname);
}

bool directCuda::beginThread() {
	if(thread)
		return false;	// Already going!
	thread = (HANDLE) _beginthread(&directCuda::threadInitialize, 0, this);
	while(!alive) {
		// Wait for something to happen
		Sleep(10);
		if(!thread) {
			// Oops, it died before activating
			return false;
		}
	}
	return true;
}

directCuda::~directCuda(void) {
	dbg("directCuda <%s>: Destroying", name);
	if(thread) {
		// Set dead, signal go & wait for termination
		alive = false;
		SetEvent(go);
		if(WaitForSingleObject(thread, 7500) == WAIT_TIMEOUT) {
			dbg("directCuda <%s>: Warning! Timeout while waiting for thread termination", name);
		}
		thread = NULL;
	}
	CloseHandle(go);
	CloseHandle(done);
}

void directCuda::threadInitialize(void *pass) {
	// note to self: real programmers stick with C
	((directCuda*)pass)->cudaThread();
}

void directCuda::cudaThread(void) {
	dbg("cudaThread <%s>: begin, thread 0x%04X", name, GetCurrentThreadId());
	//bool wantExtraRelease = false;

	// Initialize this CUDA instance
	if(devID == -1) {

		// HACK: simultaneous initialization fails on cuda
		// TODO: fix this POS instead
		ONCE
			srand((DWORD)time(NULL));
		Sleep( rand()%100 );

		D3DDEVICE_CREATION_PARAMETERS devcp;
		D3DADAPTER_IDENTIFIER9 id;
		dev->GetCreationParameters(&devcp);
		d3d->GetAdapterIdentifier(devcp.AdapterOrdinal, NULL, &id);
		if(cudaD3D9GetDevice(&devID, id.DeviceName) != cudaSuccess) {
			dbg("cudaThread <%s>: Device does not support CUDA or unknown initialization error", name);
			devID = -1;
			thread = NULL;
			goto end;
			return;
		}

		cudaDeviceProp dp;	ZeroMemory(&dp, sizeof(cudaDeviceProp));
		cudaGetDeviceProperties(&dp, devID);
		dbg("cudaThread <%s>: Using device %s", name, dp.name);
	}

	// Init cuda for this device
	CUDACALL( cudaSetDevice(devID) );
	CUDACALL( cudaD3D9SetDirect3DDevice(dev) );

	// Register all surfaces
	for(DWORD i=0;i<surf.size();i++) {
		CUDACALL( cudaD3D9RegisterResource(surf[i], cudaD3D9RegisterFlagsNone) );

		// Get surface description, if rendertarget map as read only (cannot write to them!)
		D3DSURFACE_DESC desc;
		surf[i]->GetDesc(&desc);
		bool rt = desc.Usage & D3DUSAGE_RENDERTARGET;
		dbg("cudaThread <%s>: Registering surface %d as %s", name, i, rt?"ReadOnly":"WriteDiscard");
		if(rt) {
			CUDACALL( cudaD3D9ResourceSetMapFlags(surf[i], cudaD3D9MapFlagsReadOnly) );
		} else {
			CUDACALL( cudaD3D9ResourceSetMapFlags(surf[i], cudaD3D9MapFlagsWriteDiscard) );
		}
	}

	alive = true;
	while(alive) {
		WaitForSingleObject(go, INFINITE);
		if(!alive)
			break;

		bool fault = false;
		int numSurf = surf.size();
		size = new size_t[numSurf];	// Surface sizes
		dData = new DWORD[numSurf];	// Surface data pointers

		// Map resource, get pointer & size
		for(DWORD i=0;i<surf.size();i++) {
			CUDACALL( cudaD3D9MapResources(1, (IDirect3DResource9 **)&surf[i]) );
			int r = cudaD3D9ResourceGetMappedPointer((void**) &dData[i], surf[i], 0, 0);
			if(r == cudaSuccess) {
				CUDACALL( cudaD3D9ResourceGetMappedSize(&size[i], surf[i], 0, 0) );
			} else
				fault = true;
		}

		// Perform task
		if(!fault) {
			if(task)
				taskRetval = (*task)(this, taskParams);
			else
				dbg("cudaThread <%s>: No task to perform!", name);
		}

		// Unmap resource, signal done
		for(DWORD i=0;i<surf.size();i++) {
			CUDACALL( cudaD3D9UnmapResources(1, (IDirect3DResource9 **)&surf[i]) );
		}
		SetEvent(done);
		delete[] size;
		delete[] dData;

		//wantExtraRelease = true;	// CUDA adds an extra device reference somewhere along the way
	}

	// Deinitialize
	if(memHost)
		cudaFreeHost(memHost);
	memHost = NULL;

	// Unregister all surfaces
	for(DWORD i=0;i<surf.size();i++) {
		dbg("cudaThread <%s>: Unregistering surface %d", name, i);
		CUDACALL( cudaD3D9UnregisterResource(surf[i]) );
	}
end:
	/*if(wantExtraRelease)
		d3d->Release();	// TODO: remove, fix for CUDA bug*/
	CUDACALL( cudaThreadExit() );
	dbg("cudaThread <%s>: end, thread 0x%04X", name, GetCurrentThreadId());
	thread = NULL;
}

#endif
