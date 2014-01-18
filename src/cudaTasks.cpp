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
// CUDA tasks
//

#ifdef USE_CUDA
#include "cudainterface.h"
#include "../helper.h"

// Use events or cudaThreadSynchronize
// cudaThreadSynchronize appears to be a busy loop... intelligent.
bool useEvents = true;

// Copies first bound surface from video memory, returns pointer to data
void* cudaTaskVMtoSM(directCuda* dc, void* param) {
	LPDIRECT3DSURFACE9 surf = dc->getSurface(0);	// surfA
	if(!surf) {
		dbg("cudaTaskVMtoSM: task failed");
		return 0;
	}
	DWORD size = dc->size[0];
	void* dData = (void*)dc->dData[0];	// Device data pointer

	if(!dc->memHost) {
		dbg("cudaCopyThread <%s>: Allocating %dKB of host memory", dc->name, size/1024);
		cudaMallocHost((void**)&dc->memHost, size);
		//cudaHostAlloc((void**)&dc->memHost, size, cudaHostAllocWriteCombined);
	}

	cudaEvent_t event = NULL;
	if(useEvents) {		
		cudaEventCreate(&event);
		cudaEventRecord(event, 0);
	}

	// Begin copy, then sync
	cudaMemcpyAsync(dc->memHost, dData, size, cudaMemcpyDeviceToHost, 0);

	// cudaThreadSynchronize is a busy loop D:
	if(!event)
		cudaThreadSynchronize();
	else {
		while(cudaEventQuery(event) == cudaErrorNotReady)
			Sleep(1);
	}

	return (void*) dc->memHost;
  return 0;
}

// 
void* cudaTaskUploader(directCuda* dc, void* param) {	
  /*
	monitorThreadParam *p = (monitorThreadParam*) param;
	LPDIRECT3DSURFACE9 surf = dc->getSurface(0);	// surfA
	if(!surf) {
		DebugMessage("cudaTaskUploader: task failed");
		return 0;
	}
	DWORD size = dc->size[0];
	void* dData = (void*)dc->dData[0];	// Device data pointer

	// Get surface pitch
	size_t pitch = 0;
	CUDACALL( cudaD3D9ResourceGetMappedPitch(&pitch, NULL, surf, 0, 0) );
	
	cudaEvent_t event = NULL;
	if(useEvents) {		
		cudaEventCreate(&event);
		cudaEventRecord(event, 0);
	}

	// Memcpy, if pitch doesnt match do 2D copy
	DWORD Bpp = p->surfp->lock.Pitch / p->surfp->desc.Width;	// Source bytes per pixel
	if(pitch == p->surfp->lock.Pitch) {
		CUDACALL( cudaMemcpyAsync(dData, p->surfp->lock.pBits, size, cudaMemcpyHostToDevice, 0) );
	} else {
		CUDACALL( cudaMemcpy2D(dData, pitch, (BYTE*)p->surfp->lock.pBits + (p->xpos*Bpp), p->surfp->lock.Pitch, config.sResX*Bpp, config.sResY-1, cudaMemcpyHostToDevice) );
	}	

	// cudaThreadSynchronize is a busy loop D:
	if(!event)
		cudaThreadSynchronize();
	else {
		while(cudaEventQuery(event) == cudaErrorNotReady)
			Sleep(1);
	}
	*/
	return 0;
}

#endif