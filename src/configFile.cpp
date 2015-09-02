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

#include "configFile.h"
#include <stdio.h>
#include <windows.h>
#include "helper.h"
#include "version.h"

#define NOT_SET "_NOTSET_"

extern HINSTANCE hSelf; // in main.cpp

extern int nonlocalMethodDefault = OUTMETHOD_NONLOCAL_16bDither;

configFile::configFile()
{
  numHeads = 0;
}

void configFile::load()
{
  char *modName;

  #ifdef DXGI
  modName = "dxgi.dll";
  #else
  modName = "d3d9.dll";
  #endif // DXGI

	// Get path from module handle
  int len = GetModuleFileName(GetModuleHandle(modName), cfgPath, 512);
	for(int i=len;i>0;i--)
		if(cfgPath[i] == '\\' || cfgPath[i] == '/') {
			cfgPath[i] = 0x00;
			break;
		}
	strcat(cfgPath, "\\config.SoftTHconfig");

  if(!fileExists(cfgPath)) {
    char foo[256];
    sprintf(foo, "Config file <%s> not found.\n\nCreate default configuration?", cfgPath);
    int r = MessageBox(NULL, foo, SOFTTH_VERSION, MB_YESNO|MB_ICONEXCLAMATION);
    if(r == IDYES) {
      // Create config by detecting desktop monitor layout
      MessageBox(NULL, createDefaultConfig(cfgPath), SOFTTH_VERSION, MB_OK|MB_ICONINFORMATION);
      //createDefaultConfigUAC(cfgPath);
    } else {
      exit(0);
    }
  }

  dbg("Config path: <%s>", cfgPath);
	loadConfigFile();
}

bool configFile::getResolution(char *ctg, char *name, RESOLUTION* tgt)
{
  char tmp[128];
  GetPrivateProfileString(ctg, name, NOT_SET, (LPSTR) &tmp, 128, cfgPath);
  if(strcmp(tmp, NOT_SET) && strstr(tmp,"x"))
  {
    sscanf(tmp, "%dx%d", &tgt->x, &tgt->y);
    dbg("config: %s.%s: %dx%d", ctg, name, tgt->x, tgt->y);
    return true;
  }
  tgt->x = tgt->y = 0;
  dbg("config: %s.%s: not set", ctg, name);
  return false;
}

bool configFile::getRect(char *ctg, char *name, RECT *tgt, bool relative)
{
  char tmp[256];
  GetPrivateProfileString(ctg, name, NOT_SET, (LPSTR) &tmp, 256, cfgPath);
  if(strcmp(tmp, NOT_SET) && strstr(tmp,","))
  {
    sscanf(tmp, "%d,%d,%d,%d", &tgt->left, &tgt->top, &tgt->right, &tgt->bottom);
    if(relative) {
      // Convert width/height to right/left
      tgt->right  += tgt->left;
      tgt->bottom += tgt->top;
    }
    dbg("config: %s.%s: %d,%d,%d,%d", ctg, name, tgt->left, tgt->top, tgt->right, tgt->bottom);
    return true;
  }
  tgt->left = tgt->top = tgt->right = tgt->bottom = 0;
  dbg("config: %s.%s: not set", ctg, name);
  return false;
}

bool configFile::getHead(int num, HEAD *head)
{
  char name[24];
  if(num)
    sprintf(name, "head_%d", num);
  else
    strcpy(name, "head_primary");

  // Common settings
  getResolution(name, "screenMode", &head->screenMode);
  getRect(name, "sourceRect", &head->sourceRect, true);
  getRect(name, "destRect", &head->destRect, true);

  if(num)
  {
    // Non-primary head settings
    head->devID = GetPrivateProfileInt(name, "devID", head->devID, cfgPath);
    head->noSync = GetPrivateProfileInt(name, "noSync", 0, cfgPath)!=0;

    int limit = GetPrivateProfileInt(name, "fpsLimit", 0, cfgPath);
    if(limit)
    {
      head->rateLimit = (DWORD) (1000.0f/(float)limit);
    }
    else
    {
      head->rateLimit = 0;
    }

    getResolution(name, "transportResolution", &head->transportRes);

    //head->transportMethod = GetPrivateProfileInt(name, "transportMethod", head->transportMethod, cfgPath);
    char temp[24];
    GetPrivateProfileString(name, "transportMethod", "auto", (LPSTR) &temp, 16, cfgPath);
    if(!_stricmp(temp, "auto")) head->transportMethod = OUTMETHOD_AUTO;
    else if(!_stricmp(temp, "cuda")) head->transportMethod = OUTMETHOD_CUDA;
    else if(!_stricmp(temp, "blit")) head->transportMethod = OUTMETHOD_BLIT;
    else if(!_stricmp(temp, "local")) head->transportMethod = OUTMETHOD_LOCAL;
    else if(!_stricmp(temp, "nonlocal")) head->transportMethod = OUTMETHOD_NONLOCAL;
    else if(!_stricmp(temp, "nonlocal32")) head->transportMethod = OUTMETHOD_NONLOCAL;
    else if(!_stricmp(temp, "nonlocal24")) head->transportMethod = OUTMETHOD_NONLOCAL_24b;
    else if(!_stricmp(temp, "nonlocal16")) head->transportMethod = OUTMETHOD_NONLOCAL_16b;
    else if(!_stricmp(temp, "nonlocal16d")) head->transportMethod = OUTMETHOD_NONLOCAL_16bDither;
    else ShowMessage("Invalid setting for transportMethod: <%s>", temp), exit(0);
  }
  return true;
}

void configFile::loadConfigFile()
{
  getResolution("main", "renderResolution", &main.renderResolution);
  main.keepComposition = GetPrivateProfileInt("main", "keepComposition", 0, cfgPath)!=0;
  main.debugD3D = GetPrivateProfileInt("main", "debugD3D", 0, cfgPath)!=0;
  main.vsync = GetPrivateProfileInt("main", "vsync", 0, cfgPath)!=0;
  main.tripleBuffer = GetPrivateProfileInt("main", "tripleBuffer", 0, cfgPath)!=0;
  main.smoothing = GetPrivateProfileInt("main", "smoothing", 1, cfgPath)!=0;
  main.zClear = GetPrivateProfileInt("main", "zClear", 1, cfgPath)!=0;

  char temp[256];
  GetPrivateProfileString("main", "dllPathD3D9", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathD3D9, temp);
  else
    strcpy(main.dllPathD3D9, "");

  GetPrivateProfileString("main", "dllPathDXGI", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathDXGI, temp);
  else
    strcpy(main.dllPathDXGI, "");

  GetPrivateProfileString("main", "dllPathD3D10", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathD3D10, temp);
  else
    strcpy(main.dllPathD3D10, "");

  GetPrivateProfileString("main", "dllPathD3D10_1", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathD3D10_1, temp);
  else
    strcpy(main.dllPathD3D10_1, "");

  GetPrivateProfileString("main", "dllPathD3D11", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathD3D11, temp);
  else
    strcpy(main.dllPathD3D11, "");

  GetPrivateProfileString("main", "dllPathD3D12", "auto", (LPSTR) &temp, 256, cfgPath);
  if(_stricmp(temp, "auto") && strlen(temp) > 2)
    strcpy(main.dllPathD3D12, temp);
  else
    strcpy(main.dllPathD3D12, "");

  GetPrivateProfileString("main", "nonlocalFormat", "RGB16D", (LPSTR) &temp, 256, cfgPath);
  if(!_stricmp(temp, "RGB16D")) nonlocalMethodDefault = OUTMETHOD_NONLOCAL_16bDither;
  else if(!_stricmp(temp, "RGB16")) nonlocalMethodDefault = OUTMETHOD_NONLOCAL_16b;
  else if(!_stricmp(temp, "RGB32")) nonlocalMethodDefault = OUTMETHOD_NONLOCAL;
  else dbg("WARNING: Unrecognized nonlocal format: <%s>, defaulting to RGB16D", temp);

  GetPrivateProfileString("main", "screenshotFormat", "jpg", (LPSTR) &main.screenshotFormat, 4, cfgPath);

  overrides.forceResolution = GetPrivateProfileInt("overrides", "forceResolution", 0, cfgPath)!=0;
  overrides.antialiasing = GetPrivateProfileInt("overrides", "antialiasing", 0, cfgPath);
  overrides.processAffinity = GetPrivateProfileInt("overrides", "processAffinity", 0, cfgPath);
  overrides.FOVForceHorizontal = GetPrivateProfileInt("overrides", "FOVForceHorizontal", 0, cfgPath)!=0;
  overrides.FOVForceVertical = GetPrivateProfileInt("overrides", "FOVForceVertical", 0, cfgPath)!=0;

  debug.compatibleIB = GetPrivateProfileInt("debug", "compatibleIB", 0, cfgPath)!=0;
  debug.compatibleVB = GetPrivateProfileInt("debug", "compatibleVB", 0, cfgPath)!=0;
  debug.compatibleTextures = GetPrivateProfileInt("debug", "compatibleTextures", 0, cfgPath)!=0;
  debug.enableVBQuirk = GetPrivateProfileInt("debug", "enableVBQuirk", 0, cfgPath)!=0;

  char names[256];
  GetPrivateProfileSectionNames(names, 256, cfgPath);

  // Count heads in config
  DWORD r;
  int heads = 0;
  do {
    char foo[16], hd[24];
    sprintf(hd, "head_%d", heads+1);
    r = GetPrivateProfileSection(hd, foo, 16, cfgPath);
    if(r)
      heads++;
  } while(r);
  dbg("config: Additional heads: %d", heads);
  numHeads = heads;

  // Read primary head config
  getHead(0, &primaryHead);

  // Read secondary head config
  additionalHeads = new HEAD[numHeads];
  for(int i=0;i<numHeads;i++)
    getHead(i+1, &additionalHeads[i]);
}

// Automatic config creation //

typedef struct {
  int devID;
  RECT r;
  char info[512];
} DISPLAY;

const static char cfgHeader[] = {"\
[main]\n\
renderResolution=%dx%d\n\
nonlocalFormat=RGB16D\n\
keepComposition=0\n\
smoothing=1\n\
debugD3D=0\n\
zClear=1\n\
vsync=0\n\
tripleBuffer=0\n\
screenshotFormat=jpg\n\
dllPathD3D9=auto\n\
dllPathDXGI=auto\n\
dllPathD3D10=auto\n\
dllPathD3D10_1=auto\n\
dllPathD3D11=auto\n\
dllPathD3D12=auto\n\
\n\
[overrides]\n\
forceResolution=0\n\
antialiasing=0\n\
processAffinity=0\n\
FOVForceHorizontal=0\n\
FOVForceVertical=0\n\
\n\
[debug]\n\
compatibleIB=0\n\
compatibleTex=0\n\
compatibleVB=0\n\
enableVBQuirk=0\n\
"};

const static char cfgHeadMain[] = {"\n\
[head_primary]\n\
sourceRect=%d,%d,%d,%d\n\
screenMode=%dx%d\n\
"};

const static char cfgHead[] = {"\n\
[head_%d]\n\
; %s\n\
devID=%d\n\
sourceRect=%d,%d,%d,%d\n\
transportResolution=%dx%d\n\
transportMethod=auto\n\
noSync=0\n\
fpsLimit=0\n\
"};

extern "C" __declspec(dllexport) void _cdecl doCreateDefaultConfig(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
  MessageBox(NULL, createDefaultConfig(lpszCmdLine), SOFTTH_VERSION, MB_OK|MB_ICONINFORMATION);
  //createDefaultConfig(lpszCmdLine);
}

bool createDefaultConfigUAC(char *outFile)
{
  return true;
}

char* createDefaultConfig(char *outFile)
{
  dbg("createDefaultConfig... %s", outFile?outFile:"NULL!");
  FILE *f = fopen(outFile, "rb");
  if(f) {
    fclose(f);
    return "Config already exists!";
  }

  DISPLAY *displays;

  char path[256];
  sprintf(path, "%s\\system32\\d3d9.dll", getenv("SystemRoot"));
  HINSTANCE hLib = LoadLibrary(path);
  if(!hLib)
    return "Cannot access Direct3D 9!";

  IDirect3D9* (WINAPI*d3dCreate)(UINT SDKVersion) = NULL;
  d3dCreate = (IDirect3D9*(__stdcall *)(UINT)) GetProcAddress(hLib, "Direct3DCreate9");

  IDirect3D9 *d3d = d3dCreate(D3D_SDK_VERSION);
  if(!d3d)
    dbg("createDefaultConfig: Direct3DCreate9 failed!");
  int numa = d3d->GetAdapterCount();
  dbg("createDefaultConfig: %d heads", numa);
  displays = new DISPLAY[numa];

  int minX = 0, minY = 0;     // Top left corner of desktop
  int width = 0, height = 0;  // Desktop width & height

  for(int i=0;i<numa;i++) {
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HMONITOR mon = d3d->GetAdapterMonitor(i);
    GetMonitorInfo(mon, &mi);

    DISPLAY *d = &displays[i];
    d->devID = i;
    d->r = mi.rcMonitor;

    if(d->r.left < minX) minX = d->r.left;
    if(d->r.top  < minY) minY = d->r.top;
    if(d->r.right > width)   width = d->r.right;
    if(d->r.bottom > height) height = d->r.bottom;

    D3DADAPTER_IDENTIFIER9 ai;
    d3d->GetAdapterIdentifier(i, 0, &ai);
    sprintf(d->info, "Autodetected as %s at %s", ai.DeviceName, ai.Description);
  }
  width  -= minX;
  height -= minY;

  for(int i=0;i<numa;i++) {
    DISPLAY *d = &displays[i];
    d->r.left -= minX;
    d->r.top -= minY;
    d->r.right -= minX;
    d->r.bottom -= minY;
  }

  FILE *o = NULL;
  do {
    o = fopen(outFile, "wb");
    if(!o) {
      char foo[256];
      sprintf(foo, "Cannot create: %s\nVerify write permission (Error %d)", outFile, GetLastError());
      if(MessageBox(NULL, foo, SOFTTH_VERSION, MB_RETRYCANCEL|MB_ICONSTOP) == IDCANCEL)
        return "Config creation cancelled";
    }
  } while(!o);
  fprintf(o, cfgHeader, width, height);
  fprintf(o, cfgHeadMain, displays[0].r.left, displays[0].r.top, displays[0].r.right-displays[0].r.left, displays[0].r.bottom-displays[0].r.top, displays[0].r.right-displays[0].r.left, displays[0].r.bottom-displays[0].r.top);
  for(int i=1;i<numa;i++) {
    fprintf(o, cfgHead, i, displays[i].info, displays[i].devID,
            displays[i].r.left, displays[i].r.top, displays[i].r.right-displays[i].r.left, displays[i].r.bottom-displays[i].r.top,
            displays[i].r.right-displays[i].r.left, displays[i].r.bottom-displays[i].r.top);
  }
  fclose(o);

  delete displays;

  static char out[256];
  sprintf(out, "Created config %s:\nrender resolution: %dx%d with %d heads", outFile, width, height, numa);
  d3d->Release();
  return out;
}
