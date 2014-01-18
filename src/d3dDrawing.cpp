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

// Drawing helpers for D3D)

#include <d3d9.h>
#include <d3dx9.h>

#include "helper.h"
#include "d3dDrawing.h"

struct LLVERTEX {
    D3DXVECTOR4 p;
    DWORD       color;
};
#define D3DFVF_LLVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

struct PPVERTEX {
    D3DXVECTOR3 p;
    DWORD       color;
    FLOAT       tu, tv;
};
//#define D3DFVF_PPVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_PPVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

d3dDrawing::d3dDrawing(IDirect3DDevice9Ex *device)
{
  ppEffect = NULL;
  fault = true;
  dev = device;
  D3DCALL( dev->CreateStateBlock(D3DSBT_ALL, &defaultState) );
  defaultState->Capture();
};

d3dDrawing::~d3dDrawing()
{
  SAFE_RELEASE_LAST(ppEffect);
  endDraw();
  defaultState->Release();
}

void d3dDrawing::beginDraw()
{
  // Store device state
  if(dev->CreateStateBlock(D3DSBT_ALL, &sblock) != D3D_OK) {
    fault = true;
    return;
  }

  sblock->Capture();

  // Set default device state
  defaultState->Apply();

  D3DCALL( dev->SetRenderState(D3DRS_LIGHTING, FALSE) );
  D3DCALL( dev->SetTexture(0, 0) );
  D3DCALL( dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) );
  D3DCALL( dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) );
	D3DCALL( dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) );
	D3DCALL( dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) );	
  D3DCALL( dev->SetRenderState(D3DRS_ZENABLE, FALSE) );

  dev->BeginScene();
  fault = false;
}

void d3dDrawing::endDraw()
{
  if(fault) return;

  dev->EndScene();

  // Restore device state
  sblock->Apply();
  sblock->Release();

  fault = true;
}

// Draw a box!
void d3dDrawing::drawBox(int x, int y, int width, int height, DWORD color)
{
  if(fault) return;

  float w = (float)x+width;
  float h = (float)y+height;

  LLVERTEX vdata[] = {
    D3DXVECTOR4(w,(float)y,0,0), color,
    D3DXVECTOR4(w,h,0,0), color,
    D3DXVECTOR4((float)x,(float)y,0,0), color,
    D3DXVECTOR4((float)x,h,0,0), color,
  };
  D3DCALL( dev->SetFVF(D3DFVF_LLVERTEX) );
  D3DCALL( dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vdata, sizeof(LLVERTEX)) );
}

// Draw a simple graph from array of floats (0-1)
void d3dDrawing::drawGraph(int x, int y, int width, int height, float *data, int numData)
{
  if(fault) return;  

  LLVERTEX *vdata = new LLVERTEX[numData];
  for(int i=0;i<numData;i++) {
    float xx = (float)x + ((float)width/(float)numData*(float)i);
    float yy = y+height - ((float)height*data[i]);
    LLVERTEX v = {D3DXVECTOR4(xx,yy,0,0), 0xffFFFFFF};
    memcpy(&vdata[i], &v, sizeof(LLVERTEX));
  }

  drawBox(x, y, width, height, 0xA0000000);

  D3DCALL( dev->SetFVF(D3DFVF_LLVERTEX) );
  D3DCALL( dev->DrawPrimitiveUP(D3DPT_LINESTRIP, numData-1, vdata, sizeof(LLVERTEX)) );

  for(int i=0;i<numData;i++)
    vdata[i].color = 0xFFFF0000;
  D3DCALL( dev->DrawPrimitiveUP(D3DPT_POINTLIST, numData, vdata, sizeof(LLVERTEX)) );

  delete[] vdata;
}

/*
int createGridMesh(PPVERTEX *data, int xx, int yy, int width, int height, int detail, int bufWidth, int bufHeight)
{
  int numTri = 0 ;
	float sX = width/(float)detail; // x step
	float sY = height/(float)detail; // y step

  // Half texel offsets
  float hx = 1.0f/bufWidth/2.0f;
  float hy = 1.0f/bufHeight/2.0f;

  float dx = (float)bufWidth;
  float dy = (float)bufHeight;

  PPVERTEX *d = data;
  for(float y=(float)yy;y<=height;y+=sY)
  {
    for(float x=(float)xx;x<=width;x+=sX)
    {      
      // Blah!
      float X = x+sX;
      float Y = y+sY;
      {PPVERTEX v = {D3DXVECTOR4(x,y,0,0.1f), 0xFFFFFFFF, (x/dx)+hx, (y/dy)+hy};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(x,Y,0,0.1f), 0xFFFFFFFF, (x/dx)+hx, (Y/dy)+hy};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(X,Y,0,0.1f), 0xFFFFFFFF, (X/dx)+hx, (Y/dy)+hy};*d++ = v;}

      {PPVERTEX v = {D3DXVECTOR4(X,Y,0,0.1f), 0xFFFFFFFF, (X/dx)+hx, (Y/dy)+hy};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(X,y,0,0.1f), 0xFFFFFFFF, (X/dx)+hx, (y/dy)+hy};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(x,y,0,0.1f), 0xFFFFFFFF, (x/dx)+hx, (y/dy)+hy};*d++ = v;}
      numTri += 2;
    }
  }
  return numTri;
}
*/

int createGridMesh(PPVERTEX *data, int detail)
{
  int numTri = 0;
	float sX = 1.0f/(float)detail; // x step
	float sY = 1.0f/(float)detail; // y step
  PPVERTEX *d = data;
  for(float y=0;y<=1.0f;y+=sY)
  {
    for(float x=0;x<=1.0f;x+=sX)
    {      
      // Blah!
      float X = x+sX;
      float Y = y+sY;
      {PPVERTEX v = {D3DXVECTOR4(x,y,0,0.1f), 0xFFFFFFFF, x, y};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(x,Y,0,0.1f), 0xFFFFFFFF, x, Y};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(X,Y,0,0.1f), 0xFFFFFFFF, X, Y};*d++ = v;}

      {PPVERTEX v = {D3DXVECTOR4(X,Y,0,0.1f), 0xFFFFFFFF, X, Y};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(X,y,0,0.1f), 0xFFFFFFFF, X, y};*d++ = v;}
      {PPVERTEX v = {D3DXVECTOR4(x,y,0,0.1f), 0xFFFFFFFF, x, y};*d++ = v;}
      numTri += 2;
    }
  }
  return numTri;
}

void d3dDrawing::drawTexturePP(int xx, int yy, int width, int height, IDirect3DTexture9* texture, const char *tech)
{
  if(fault) return;

  if(GetKeyState('P') < 0 && ppEffect)
  {
    dbg("Recompile Postprocess");
    while(ppEffect->Release()) {};
    ppEffect = NULL;
  }

  if(!ppEffect)
  { 
    // Create effect first (auto-destroyed when drawing is deleted)
    ID3DXBuffer *err;
    D3DCALL( D3DXCreateEffectFromFile(dev, "P:\\pp_test.fx", NULL, NULL, 0, NULL, &ppEffect, &err) );
    if(err)
    {
      dbg("d3dDrawing::drawTexturePP D3DXCreateEffectFromFile error:\n%s", err->GetBufferPointer());
    }
  }

  if(!ppEffect) return;

  float x = (float)xx;
  float y = (float)yy;
  float w = (float)x+width;
  float h = (float)y+height;

  /*D3DCALL( dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL) );
  D3DCALL( dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE) );*/

  D3DCALL( dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) );
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

  D3DSURFACE_DESC desc;
  texture->GetLevelDesc(0, &desc);

  /*
  const int detail = 30;
  PPVERTEX *data = new PPVERTEX[(detail+1)*(detail+1)*6];
  int numt = createGridMesh(data, x, y, width, height, detail, desc.Width, desc.Height);
  */
  const int detail = 30;
  PPVERTEX *data = new PPVERTEX[(detail+1)*(detail+1)*6];
  int numt = createGridMesh(data, detail);

  /*
  // Texture coordinate divisors
  D3DSURFACE_DESC desc;
  texture->GetLevelDesc(0, &desc);
  float dx = (float)desc.Width;
  float dy = (float)desc.Height;

  // Half texel offsets
  float hx = 1.0f/w/2.0f;
  float hy = 1.0f/h/2.0f;

  const float z = 0.0;
  PPVERTEX vdata[] = {
    D3DXVECTOR4(w,y,z,0.1f), 0xFFFFFFFF, (w/dx)+hx, (y/dy)+hy,
    D3DXVECTOR4(w,h,z,0.1f), 0xFFFFFFFF, (w/dx)+hx, (h/dy)+hy,
    D3DXVECTOR4(x,y,z,0.1f), 0xFFFFFFFF, (x/dx)+hx, (y/dy)+hy,
    D3DXVECTOR4(x,h,z,0.1f), 0xFFFFFFFF, (x/dx)+hx, (h/dy)+hy,
  };
*/
  
  D3DCALL( ppEffect->SetTechnique(tech) );

  // Set effect parameters
  D3DXVECTOR4 bufferResolution((float)desc.Width, (float)desc.Height, 0, 0);
  D3DXVECTOR4 ppArea(x, y, w, h);
  ppEffect->SetVector("bufferResolution", &bufferResolution);
  ppEffect->SetVector("ppArea", &ppArea);

  UINT passes = 0;
  D3DCALL( ppEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE)  );
  for(DWORD i=0;i<passes;i++)
  {
    ppEffect->BeginPass(i);

    D3DCALL( dev->SetFVF(D3DFVF_PPVERTEX) );
    D3DCALL( dev->SetTexture(0, texture) ); 
    //D3DCALL( dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vdata, sizeof(PPVERTEX)) );
    D3DCALL( dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, numt, data, sizeof(PPVERTEX)) );    
    D3DCALL( dev->SetTexture(0, NULL) ); 

    ppEffect->EndPass();
  }
  D3DCALL( ppEffect->End() );

  delete[] data;
}


void d3dDrawing::drawTextureLens(int x, int y, int width, int height, IDirect3DTexture9* texture)
{
  if(fault) return;

  static int lu = 0; // Last update time
  static float foo = 20;
  static float edge = foo;
  static bool lol = false;
  if(GetKeyState('O') < 0)
  {
    foo += 1;
    lu = GetTickCount();
  }
  if(GetKeyState('P') < 0)
  {
    foo -= 1;
    lu = GetTickCount();
  }
  if(GetKeyState('I') < 0)
  {
    lol = !lol;
    Sleep(500);
    dbg("%f", foo);
  }

  edge = edge>70?70:edge; // Limit edge value

  int numTris = createQuadLens(width, height, (int)foo, lol?(int)edge/2.0f:0);

  dev->SetTexture(0, texture);
  dev->SetStreamSource(0, lensBuf, 0, sizeof(PPVERTEX));

	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
  dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

  D3DCALL( dev->SetFVF(D3DFVF_PPVERTEX) );
  D3DCALL( dev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, numTris) );

  if(GetTickCount() - lu < 2000)
  {
    dev->SetTexture(0, NULL);
    dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    D3DCALL( dev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, numTris) );
  }

  dev->SetStreamSource(0, NULL, 0, sizeof(PPVERTEX));
  dev->SetTexture(0, NULL);

  SAFE_RELEASE_LAST(lensBuf);
  lensBuf = NULL;
}


// Lens corrected quad

/////////////////////////////
// WARNING: old code ahead //
/////////////////////////////

const double PI = 3.1415926535897932384626433832795;
static __forceinline double Deg2Rad(double x) {
	return x * PI/180.0;
};

int d3dDrawing::createQuadLens(int resX, int resY, int lensCorrection, int lensCorrectionEdge) {
	const int detail = 30;
	double sX = resX/(float)detail; // x step
	double sY = resY/(float)detail; // y step

	DWORD size = (detail+2)*(detail+1)*2;
/*
	*vp = lensBuf;
	static int lastAngle = -1;
	static int lastAngledge = -1;

	if(lastAngle == config.lensCorrection && lastAngledge == config.lensCorrectionEdge && lensBuf) {
		// Same data as before, just use existing buffer
		return size;
	}
	lastAngle = config.lensCorrection;
	lastAngledge = config.lensCorrectionEdge;

	if(lensBuf)
		while(lensBuf->Release());
  
	dbg("createQuadLens: Creating new buffer %d/%d", config.lensCorrection, config.lensCorrectionEdge);
*/

	if(lensBuf)
		while(lensBuf->Release());

	// Create new buffer
	int ret = dev->CreateVertexBuffer(size*sizeof(PPVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_PPVERTEX, D3DPOOL_DEFAULT, &lensBuf, NULL);
	if(D3D_OK != ret) {
		dbg("createQuadLens: CreateVertexBuffer failed!");
		lensBuf = NULL;
		return -1;
	}
	
	PPVERTEX* vd;
	if(lensBuf->Lock(0, 0, (void**)&vd, D3DLOCK_NOSYSLOCK) != D3D_OK) {
		dbg("createQuadLens: lensBuf->Lock failed!");
		return 0;
	}

	// 2D distance macro
	#define DISTANCE(x,y, xx, yy) sqrt(((x-xx)*(x-xx)) + ((y-yy)*(y-yy)))

	float aspect = (float)resX/(float)resY;
	float maxD = (float)DISTANCE(0,0,0.5,0.5/aspect);
	float centerX = 0.5f;
	float centerY = 0.5f;
	float amount = ((float)lensCorrection/360.0f);
	float amountE = ((float)lensCorrectionEdge/360.0f);

	int i = 0;
	for(double y=0;y<=resY;y+=sY) {
		for(double x=0;x<=resX+sX;x+=sX) {
			for(int a=0;a<2;a++) {
				float xx = (float) (x/resX);
				float yy = (float) (y/resY);
				yy += (1.0f/detail*a);

				if((DWORD)i>size)
					dbg("createQuadLens: %d>%d!", i, size);

				vd[i].p.x = (float) x;
				vd[i].p.y = (float) y + (float) (sY*a);
				vd[i].p.z = 0;
        //vd[i].p.w = 100;

				float dist = DISTANCE(xx, yy/aspect, centerX, centerY/aspect); // Distance to center
				dist = dist/maxD;
				dist = (float) (amountE*cos((dist/maxD)*180*PI/180)) + ((1-amountE)*dist); // Edge

				vd[i].color = 0xFFFFFFFF;

				vd[i].tu = (xx-0.5f) * ((dist*amount) + (1*(1-amount))) + 0.5f;
				vd[i].tv = (yy-0.5f) * ((dist*amount) + (1*(1-amount))) + 0.5f;

				// Half texel offset
				vd[i].tu += (0.5f/resX);
				vd[i].tv += (0.5f/resY);

				i++;
			}
		}
	}

	lensBuf->Unlock();

	return size;
}