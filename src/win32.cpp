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

// Win32 crud (window creation)
// warning: ugly code follows

#include "win32.h"
#include "helper.h"
#include <stdio.h>
#include "Windowsx.h"
//#include "configFile.h"
#include "inputHandler.h"

#include "overlay_interface.h"

#include "d3dSoftTH.h"

HWND createWindow(int x, int y, int w, int h, HWND rootWindow, DWORD flags, bool show);

// Thread that creates window and handles its messages
void __cdecl windowHandler(void* a) {
  WINDOWPARAMS *wp = (WINDOWPARAMS*) a;
	HWND appWindow = createWindow(wp->x, wp->y, wp->width, wp->height, wp->parent, wp->flags, wp->show);  
  wp->hWnd = appWindow;
	MSG msg;
	while(GetMessage(&msg, appWindow, 0, 0)) {    
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} 
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
      case WM_CLOSE:
        dbg("SecondaryWindow:WM_CLOSE");
        DestroyWindow(hwnd);
      break;
      case WM_DESTROY:
        dbg("SecondaryWindow:WM_DESTROY");
        PostQuitMessage(0);
      break;

      case WM_MOUSEACTIVATE:
        dbg_input("SecondaryWindow:WM_MOUSEACTIVATE");
        return MA_NOACTIVATE;
        //return MA_NOACTIVATEANDEAT;
        break;

      case WM_ACTIVATE: {
        dbg_input("SecondaryWindow: WM_ACTIVATE");
        return 0;
        break;
      }

      case WM_MOVE:
        //dbg("WM_MOVE!");
        break;

      case MOUSE_ALL_WM_EVENTS: {
        if(!SoftTHActive)
          break;

        int x = GET_X_LPARAM(lParam); 
        int y = GET_Y_LPARAM(lParam);

        POINT op = {x, y};
        POINT vp = {0, 0};
        if(inputMapClientToVirtual(hwnd, &op, &vp)) {
          dbg_input("SecondaryWindow: %s: %dx%d -> %dx%d (wparam: 0x%08X)", getMouseEventName(msg), x, y, vp.x, vp.y, wParam);
          LRESULT ret = PostMessage(GetParent(hwnd), msg, wParam + MOUSE_EVENTS_ALREADY_MAPPED, MAKELPARAM(vp.x, vp.y));
  /*
          // Send click to overlay
          if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
          {
            OVERLAY_CLICK_BLOCK p;
            p.overlayVersion = OVERLAY_VERSION;
            p.x = vp.x;
            p.y = vp.y;
            p.up = msg==WM_LBUTTONUP;
            p.appWindow = GetParent(hwnd);
            overlayDoClick(&p);
          }
*/
          return 1;
        }
        break;
      }

      case WM_WINDOWPOSCHANGING: {
        WINDOWPOS *wp = (WINDOWPOS*) lParam;

        bool moving = !(wp->flags & SWP_NOMOVE);
        bool resizing = !(wp->flags & SWP_NOSIZE);
        if(moving)
          wp->flags += SWP_NOMOVE;
        if(resizing)
          wp->flags += SWP_NOSIZE;
        break;
      }

      default:
        dbg_input("SecondaryWindow: msg 0x%08X", msg);
        return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

HWND createWindow(int x, int y, int w, int h, HWND rootWindow, DWORD flags, bool show) {
	HWND hWnd;

	// Register new window class
	static char className[256] = {0x00};
	if(strlen(className) < 2)
		sprintf(className, "SoftTH_%d", GetTickCount());
	static WNDCLASS    wc;
	static HINSTANCE hInstance = NULL;
  if(!hInstance) {
		hInstance = GetModuleHandle(NULL);
		//wc.style         = CS_OWNDC;
		wc.style         = CS_BYTEALIGNCLIENT;
		wc.lpfnWndProc   = (WNDPROC) WindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = NULL;
		wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = className;

		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:  "
			   "Cannot register window class.", "Error", MB_OK);
			return NULL;
		}
  }

  hWnd = CreateWindowEx(flags, className, 
                             "SoftTH",
                             /*WS_OVERLAPPEDWINDOW*/ WS_POPUP,
                             x, y, w, h, rootWindow, NULL, hInstance, NULL);
  if(hWnd == NULL) {
    dbg("CreateWindowEx failed!");
    return NULL;
  }
/*
  if(show)
  {
    ShowWindow(hWnd, SW_SHOWNA);
    UpdateWindow(hWnd);
  }
*/
	return hWnd;
}

// Copy image surface from memory pointer
void bitsToWin(HDC hdc, int srcWidth, int srcHeight, int tgtWidth, int tgtHeight, void *pBits, int bpp) {
	// Bitmap junk...
	BITMAPINFO *bi = (BITMAPINFO*) new unsigned char[sizeof(BITMAPINFO) + 2 * sizeof(int)];
	BITMAPINFOHEADER ih;
	ZeroMemory(&ih, sizeof(ih));
	ZeroMemory(bi, sizeof(BITMAPINFO) + 2 * sizeof(int));
	ih.biSize = sizeof(ih);
	ih.biWidth = srcWidth;
	ih.biHeight = -(signed int) srcHeight;
	ih.biPlanes = 1;

	if(bpp==16) { // YUV mode never should get here so can be ignored
		// 16bit colours
		ih.biClrUsed = 0;
		ih.biClrImportant = 0;
		ih.biBitCount = bpp;
		ih.biCompression = BI_BITFIELDS;
		ih.biSizeImage = srcWidth * srcHeight * 2;

		// RGB 565 bitmask
    bi->bmiColors[0].rgbGreen = 0xf8;
    bi->bmiColors[1].rgbGreen = 0x07;
    bi->bmiColors[1].rgbBlue  = 0xe0;
    bi->bmiColors[2].rgbBlue  = 0x1f;
	} else {
		// 32bit colour
		ih.biBitCount = bpp;
		ih.biCompression = BI_RGB;
		ih.biSizeImage =  0;
	}
	bi->bmiHeader = ih;

  int rr = StretchDIBits(hdc, 0, 0, tgtWidth, tgtHeight, 0, 0, srcWidth, srcHeight, pBits, bi, DIB_RGB_COLORS, SRCCOPY);
  if(!rr)
	  dbg("StretchDIBits failed! Error 0x%X", GetLastError());

	delete[] bi;
}