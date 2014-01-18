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

#ifndef _SHCOPY_FILE_H_
#define _SHCOPY_FILE_H_

#include <d3d9.h>

#define SHCOPY_DITHER   "DITHER"

class shCopy
{
public:
  shCopy(IDirect3DDevice9 *device, char *shaderSrc);
  ~shCopy();
  bool surfCopyShader(IDirect3DTexture9 *src, IDirect3DSurface9 *dst);
  bool surfCopyShader(IDirect3DTexture9 *src, IDirect3DTexture9 *dst);

private:
  bool createTextureFromRawData(void *src, int width, int height, int bpp, D3DFORMAT fmt, IDirect3DTexture9 **tex);

  bool isDither;

  IDirect3DDevice9 *dev;
  LPDIRECT3DPIXELSHADER9 pshader;
  IDirect3DStateBlock9 *defaultState;
  IDirect3DStateBlock9 *sblock;

  IDirect3DTexture9 *texDither, *texDitherRamp;
};

#endif