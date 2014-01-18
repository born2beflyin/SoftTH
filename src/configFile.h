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

#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <windows.h>

#define OUTMETHOD_AUTO          -1 // Auto-detect (done by config loader)
#define OUTMETHOD_LOCAL         0 // Use a shared surface directly
#define OUTMETHOD_NONLOCAL      1 // Use a shared surface as copy buffer
#define OUTMETHOD_NONLOCAL_24b  2 // Use a shared surface as copy buffer - Use pixel shader to pack/unpack RGB data to RGBA surface
#define OUTMETHOD_NONLOCAL_16b  3 // Use a shared surface as copy buffer - 16bit undithered buffer
#define OUTMETHOD_NONLOCAL_16bDither 4 // 16bit dithered buffer
#define OUTMETHOD_BLIT          5 // Use GetRenderTargetData + win32 blitting
#define OUTMETHOD_CUDA          6 // Use  CUDA copying

extern int nonlocalMethodDefault; // Default method for nonlocal autodetect

static bool copyMethodNonlocal(int x) {return (x == OUTMETHOD_NONLOCAL_16b || x == OUTMETHOD_NONLOCAL_24b || x == OUTMETHOD_NONLOCAL || x == OUTMETHOD_NONLOCAL_16bDither);};

char* createDefaultConfig(char *outFile); // Creates default config by detecting desktop monitors
bool createDefaultConfigUAC(char *outFile);

// Resolution (1234x456)
typedef struct RESOLUTION {
  int x, y;
  RESOLUTION() {x=y=0;};
} RESOLUTION;

#define MANF_UNKNOWN 0
#define MANF_NVIDIA  1
#define MANF_AMD     2

// Head (monitor)
typedef struct HEAD {
  int devID;    // Direct3D device ID
  RECT sourceRect;  // Source rectangle from newbb
  RECT destRect;    // Destrination rectangle in screen coordinates (or zero)
  RESOLUTION screenMode;
  RESOLUTION transportRes;
  bool noSync;
  int transportMethod;
  HWND hwnd;  // Output window (if created)

  int manufacturer; // MANF_NVIDIA etc.

  // FPS limiting
  DWORD rateLimit;
  bool skipPresent;
  bool skipPresentNext;
  DWORD lastUpdate;

  HEAD() {
    devID = 0;
    noSync = false;
    transportMethod = OUTMETHOD_AUTO;
    hwnd = NULL;
    manufacturer = MANF_UNKNOWN;
    rateLimit = 0;
    skipPresent = skipPresentNext = false;
    lastUpdate = 0;
  }
} HEAD;

class configFile
{
public:
  configFile();
  void load();
  int getNumAdditionalHeads() {return numHeads;};
  HEAD* getPrimaryHead() {return &primaryHead;};
  HEAD* getHead(int num) {return &additionalHeads[num];};

  struct {
    RESOLUTION renderResolution; // Wanted render resolution
    bool keepComposition; // true = Do not disable Vista desktop composition
    bool debugD3D;  // true = use debug direct3D library + hook debug output
    bool smoothing; // true = use D3D queries to sync secondary devices
    bool vsync; // true = use vsync
    bool tripleBuffer; // true = use two backbuffers
    char dllPathD3D9[256];  // Overridden path to direct3D DLL, or blank string for auto
    char dllPathDXGI[256];  // Overridden path to DXGI DLL, or blank string for auto
    char dllPathD3D11[256];  // Overridden path to D3D11 DLL, or blank string for auto    
    char screenshotFormat[4];
    bool zClear;  // Use z-clear trick to skip drawing of non-visible screen areas
  } main;

  struct {
    bool forceResolution; // true = Force resolution to renderResolution
    int antialiasing;
    int processAffinity;
    bool FOVForceHorizontal;
    bool FOVForceVertical;
  } overrides;

  struct {
    bool compatibleIB; // Use dynamic indxebuffers only
    bool compatibleTextures; // Use dynamic textures only
    bool compatibleVB; // Use compatible vertexbuffers
    bool enableVBQuirk; // Enable vertex buffer quirk mode
  } debug;

private:
  char cfgPath[256];

  void loadConfigFile();
  bool getResolution(char *ctg, char *name, RESOLUTION* tgt);
  bool getRect(char *ctg, char *name, RECT *r, bool relative=false);
  bool getHead(int num, HEAD *head);

  int numHeads; // Number of additional heads
  HEAD primaryHead; // Primary head (always on primary device, not transport res)
  HEAD *additionalHeads; 
};

#endif
