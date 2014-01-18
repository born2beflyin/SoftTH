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

#ifndef _INPUTHANDLER_FILE_H_
#define _INPUTHANDLER_FILE_H_

#define MAX_KEYS  256

#define MOUSE_EVENTS_ALREADY_MAPPED 0x10000000  // If set in WM_* wParam, message is already remapped and should not be touched in msg hook

#define MOUSE_ALL_WM_EVENTS\
		WM_LBUTTONDBLCLK:\
		case WM_LBUTTONDOWN:\
		case WM_LBUTTONUP:\
		case WM_MBUTTONDBLCLK:\
		case WM_MBUTTONDOWN:\
		case WM_MBUTTONUP:\
		case WM_MOUSEMOVE:\
		case WM_NCLBUTTONDBLCLK:\
		case WM_NCLBUTTONDOWN:\
		case WM_NCLBUTTONUP:\
		case WM_NCMBUTTONDBLCLK:\
		case WM_NCMBUTTONDOWN:\
		case WM_NCMBUTTONUP:\
		case WM_NCMOUSEMOVE:\
		case WM_NCRBUTTONDBLCLK:\
		case WM_NCRBUTTONDOWN:\
		case WM_NCRBUTTONUP:\
		case WM_RBUTTONDBLCLK:\
		case WM_RBUTTONDOWN:\
		case WM_RBUTTONUP:\
		case WM_MOUSEWHEEL

// Mapper functions
bool inputMapClientToVirtual(HWND win, POINT *in, POINT *out);
bool inputMapVirtualToDesktop(POINT *in, POINT *out);
bool inputMapIsDeviceWindow(HWND win);
char* getMouseEventName(UINT event);

class InputHandler {
public:
  InputHandler() {hWin = NULL;setHook();for(int i=0;i<MAX_KEYS;i++) keys[i]=false;};
  ~InputHandler() {releaseHook();};

  void hookRemoteThread(DWORD threadID);
  void newThread();
  void detachThread();

  void setHWND(HWND newHWND) {hWin = newHWND;};
  HWND getHWND() {return hWin;};

  void keyDown(int key) {if(key<MAX_KEYS) keys[key] = true;keysAsync[key] = true;};
  void keyUp(int key) {if(key<MAX_KEYS) keys[key] = false;};
  bool key(int key) {
    if(key == VK_APPLICATION) // Doesn't get reported in wm_keydown?!
      return GetKeyState(VK_APPLICATION)<0;
    if(key<MAX_KEYS) 
      return keys[key];
    else
      return false;
  };
  bool keyAsync(int key) {if(key<MAX_KEYS) if(keysAsync[key]) {keysAsync[key]=false;return true;} else {return false;};};
  void resetAsyncKeys() {for(int i=0;i<MAX_KEYS;i++) keysAsync[i]=false;};

private:

  bool keys[MAX_KEYS];
  bool keysAsync[MAX_KEYS];

  void setHook(DWORD tid=NULL);
  void releaseHook();

  volatile HWND hWin;  // Device window to do hooking for
};

extern InputHandler ihGlobal; // Global handler. init automatically on dll start

#endif