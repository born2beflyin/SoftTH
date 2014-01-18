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

#ifndef _D3DDRAWING_FILE_H_
#define _D3DDRAWING_FILE_H_

#include <d3d9.h>
#include <d3dx9.h>

class d3dDrawing {
public:
  d3dDrawing(IDirect3DDevice9Ex *device);
  ~d3dDrawing();

  void beginDraw();
  void drawGraph(int x, int y, int width, int height, float *data, int numData);
  void endDraw();
  void drawBox(int x, int y, int width, int height, DWORD color);
  void drawTextureLens(int x, int y, int width, int height, IDirect3DTexture9* texture);
  void drawTexturePP(int x, int y, int width, int height, IDirect3DTexture9* texture, const char *tech);

private:
  bool fault;
  IDirect3DStateBlock9 *defaultState;
  IDirect3DStateBlock9 *sblock;
  IDirect3DDevice9Ex *dev;

  int createQuadLens(int resX, int resY, int lensCorrection, int lensCorrectionEdge);
  IDirect3DVertexBuffer9 *lensBuf;

  ID3DXEffect *ppEffect;
};

#endif
