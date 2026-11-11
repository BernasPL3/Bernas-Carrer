#pragma once
#include <gccore.h>
#include <malloc.h>
#include <cstring>
#include <cctype>
#include <string>

namespace ui {
struct Color{u8 r,g,b,a=255;};
static constexpr Color bg{242,244,235},ink{26,47,36},muted{111,128,106},green{31,65,45},lime{215,237,133},white{255,255,255},line{217,225,208},red{176,71,62};
class Renderer{
 GXRModeObj* mode;void* buffers[2];void* fifo;int current=0;
 static const u8* glyph(char c){
  static const char* chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,:;!?+-/=()%#_<>[]'";
  static const u8 data[][7]={
   {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
   {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
   {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
   {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
   {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
   {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
   {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
   {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
   {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},
   {14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
   {0,0,0,0,0,0,0},{0,0,0,0,0,12,12},{0,0,0,0,12,4,8},{0,12,12,0,12,12,0},
   {0,12,12,0,12,4,8},{4,4,4,4,4,0,4},{14,17,1,2,4,0,4},{0,4,4,31,4,4,0},
   {0,0,0,31,0,0,0},{1,2,2,4,8,8,16},{0,0,31,0,31,0,0},{2,4,8,8,8,4,2},
   {8,4,2,2,2,4,8},{25,25,2,4,8,19,19},{10,10,31,10,31,10,10},{0,0,0,0,0,0,31},
   {2,4,8,16,8,4,2},{8,4,2,1,2,4,8},{14,8,8,8,8,8,14},{14,2,2,2,2,2,14},{4,4,8,0,0,0,0}
  };
  c=char(std::toupper(static_cast<unsigned char>(c)));const char*p=std::strchr(chars,c);return data[p?int(p-chars):42];
 }
 static void vertex(float x,float y,Color c){GX_Position2f32(x,y);GX_Color4u8(c.r,c.g,c.b,c.a);}
public:
 Renderer(){
  VIDEO_Init();mode=VIDEO_GetPreferredMode(nullptr);
  buffers[0]=MEM_K0_TO_K1(SYS_AllocateFramebuffer(mode));buffers[1]=MEM_K0_TO_K1(SYS_AllocateFramebuffer(mode));
  VIDEO_Configure(mode);VIDEO_SetNextFramebuffer(buffers[0]);VIDEO_SetBlack(FALSE);VIDEO_Flush();VIDEO_WaitVSync();if(mode->viTVMode&VI_NON_INTERLACE)VIDEO_WaitVSync();
  fifo=memalign(32,256*1024);std::memset(fifo,0,256*1024);GX_Init(fifo,256*1024);
  GX_SetCopyClear({bg.r,bg.g,bg.b,255},0x00ffffff);GX_SetViewport(0,0,mode->fbWidth,mode->efbHeight,0,1);
  GX_SetScissor(0,0,mode->fbWidth,mode->efbHeight);float scale=GX_GetYScaleFactor(mode->efbHeight,mode->xfbHeight);u32 height=GX_SetDispCopyYScale(scale);
  GX_SetDispCopySrc(0,0,mode->fbWidth,mode->efbHeight);GX_SetDispCopyDst(mode->fbWidth,height);GX_SetCopyFilter(mode->aa,mode->sample_pattern,GX_TRUE,mode->vfilter);
  GX_SetFieldMode(mode->field_rendering,((mode->viHeight==2*mode->xfbHeight)?GX_ENABLE:GX_DISABLE));GX_SetCullMode(GX_CULL_NONE);GX_SetDispCopyGamma(GX_GM_1_0);
  Mtx44 projection;guOrtho(projection,0,480,0,640,0,1);GX_LoadProjectionMtx(projection,GX_ORTHOGRAPHIC);
  Mtx model;guMtxIdentity(model);GX_LoadPosMtxImm(model,GX_PNMTX0);GX_SetCurrentMtx(GX_PNMTX0);
  GX_ClearVtxDesc();GX_SetVtxDesc(GX_VA_POS,GX_DIRECT);GX_SetVtxDesc(GX_VA_CLR0,GX_DIRECT);
  GX_SetVtxAttrFmt(GX_VTXFMT0,GX_VA_POS,GX_POS_XY,GX_F32,0);GX_SetVtxAttrFmt(GX_VTXFMT0,GX_VA_CLR0,GX_CLR_RGBA,GX_RGBA8,0);
  GX_SetNumChans(1);GX_SetChanCtrl(GX_COLOR0A0,GX_DISABLE,GX_SRC_REG,GX_SRC_VTX,GX_LIGHTNULL,GX_DF_NONE,GX_AF_NONE);
  GX_SetNumTexGens(0);GX_SetTevOrder(GX_TEVSTAGE0,GX_TEXCOORDNULL,GX_TEXMAP_NULL,GX_COLOR0A0);GX_SetTevOp(GX_TEVSTAGE0,GX_PASSCLR);
  GX_SetZMode(GX_FALSE,GX_ALWAYS,GX_FALSE);GX_SetColorUpdate(GX_TRUE);GX_SetBlendMode(GX_BM_BLEND,GX_BL_SRCALPHA,GX_BL_INVSRCALPHA,GX_LO_CLEAR);
 }
 void rect(float x,float y,float w,float h,Color c){GX_Begin(GX_QUADS,GX_VTXFMT0,4);vertex(x,y,c);vertex(x+w,y,c);vertex(x+w,y+h,c);vertex(x,y+h,c);GX_End();}
 void outline(float x,float y,float w,float h,Color c){rect(x,y,w,1,c);rect(x,y+h-1,w,1,c);rect(x,y,1,h,c);rect(x+w-1,y,1,h,c);}
 void text(float x,float y,const std::string& value,Color color=ink,float size=1.5f,int maxChars=90){
  int count=0,n=std::min(int(value.size()),maxChars);for(int k=0;k<n;k++){const u8*g=glyph(value[k]);for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(g[row]&(1<<(4-col)))count++;}
  if(!count)return;GX_Begin(GX_QUADS,GX_VTXFMT0,u16(count*4));
  for(int k=0;k<n;k++){const u8*g=glyph(value[k]);for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(g[row]&(1<<(4-col))){float px=x+(k*6+col)*size,py=y+row*size;vertex(px,py,color);vertex(px+size,py,color);vertex(px+size,py+size,color);vertex(px,py+size,color);}}
  GX_End();
 }
 void finish(){GX_DrawDone();GX_CopyDisp(buffers[current],GX_TRUE);GX_Flush();VIDEO_SetNextFramebuffer(buffers[current]);VIDEO_Flush();VIDEO_WaitVSync();current^=1;}
};
}
