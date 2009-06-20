#include "bumprace.h"
#include "SDL_image.h"
#include <stdlib.h>
#include <string.h>

#define PATHNUM 8
#define BPP 0
#define RECTS_NUM 40000
#ifndef DATADIR
#define DATADIR "BumpRace.app/Contents/Resources/data"
#endif
char DATAPATH[200]=DATADIR;
const char PATH[PATHNUM][200]={DATADIR,".","data","/usr/local/share/bumprace"
,"/usr/lib/bumprace","../data","/usr/share/bumprace", DATADIR};
SDL_Surface *Screen,*BackBuffer,*fadebuffer;
SDL_Rect blitrect,blitrects[RECTS_NUM];
int blitrects_num=0;
#ifdef DATADIR
void ComplainAndExit(void)
{
        fprintf(stderr, "Problem: %s\n", SDL_GetError());
        exit(1);
}
#endif
int abrand(int a,int b)  //random number between a and b (inclusive)
{
  return(a+(rand() % (b-a+1)));
}

int (*_PutPixel)(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color);

int fast_putpixel1(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
  if (X < 0 || X > Surface->w || Y < 0 || Y > Surface->h) 
    return -1;

  *((Uint8 *)Surface->pixels + Y * Surface->pitch + X) = Color;

  return 0;
}

int fast_putpixel2(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
  if (X < 0 || X > Surface->w || Y < 0 || Y > Surface->h) 
    return -1;

 *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X) = Color;

  return 0;
}

int fast_putpixel3(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
  Uint8 *pix;
  int shift;

  if (X < 0 || X > Surface->w || Y < 0 || Y > Surface->h) 
    return -1;

  /* Gack - slow, but endian correct */
  pix = (Uint8 *)Surface->pixels + Y * Surface->pitch + X*3;
  shift = Surface->format->Rshift;
  *(pix+shift/8) = Color>>shift;
  shift = Surface->format->Gshift;
  *(pix+shift/8) = Color>>shift;
  shift = Surface->format->Bshift;
  *(pix+shift/8) = Color>>shift;

  return 0;
}

int fast_putpixel4(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
  if (X < 0 || X > Surface->w || Y < 0 || Y > Surface->h) 
    return -1;

  *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X) = Color;

  return 0;
}

void init_SDL()  // sets the video mode
{
  int bpp=BPP;
  const SDL_VideoInfo *info;

  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) < 0 ) {ComplainAndExit();}
  info = SDL_GetVideoInfo();
  if (info->vfmt->BitsPerPixel==8) bpp=16;
  atexit(SDL_Quit);
// Set the video mode (800x600 at 16-bit depth)  
  if (fullscreen)  
    Screen = SDL_SetVideoMode(800, 600, BPP, SDL_FULLSCREEN);
  else 
    { Screen = SDL_SetVideoMode(800, 600, BPP, 0); }
  if ( Screen == NULL ) {ComplainAndExit();}
// create BackBuffer
  BackBuffer = SDL_AllocSurface(Screen->flags,
                               800,
                               600,
                               Screen->format->BitsPerPixel,
                               Screen->format->Rmask,
                               Screen->format->Gmask,
                               Screen->format->Bmask, 0);
  if (BackBuffer == NULL)
  printf("ERROR: Couldn't create BackBuffer: %s\n", SDL_GetError());
  fadebuffer = SDL_AllocSurface(Screen->flags,
                               800,
                               600,
                               Screen->format->BitsPerPixel,
                               Screen->format->Rmask,
                               Screen->format->Gmask,
                               Screen->format->Bmask, 0);
  if (fadebuffer == NULL)
  printf("ERROR: Couldn't create fadebuffer: %s\n", SDL_GetError());
// Figure out what putpixel routine to use
   switch (Screen->format->BytesPerPixel)
   {
    case 1:
      _PutPixel = fast_putpixel1;
      break;
    case 2:
      _PutPixel = fast_putpixel2;
      break;
    case 3:
      _PutPixel = fast_putpixel3;
      break;
    case 4:
      _PutPixel = fast_putpixel4;
      break;
   }
}

void lock()
{
        if ( SDL_MUSTLOCK(Screen) ) {
                if ( SDL_LockSurface(Screen) < 0 )
		return;        }                                                                       
}

void unlock()
{
        if ( SDL_MUSTLOCK(Screen) ) {                                           
	                SDL_UnlockSurface(Screen); }                                                                       
}

// Performs Callback at each line point. This came straight from the
// asphyxia vga trainers
int DoLine (SDL_Surface *Surface, Sint32 X1, Sint32 Y1, Sint32 X2, Sint32 Y2, Uint32 Color, int Callback (SDL_Surface *Surf, Sint32 X, Sint32 Y, Uint32 Color))
{ 
  Sint32 dx, dy, sdx, sdy, x, y, px, py; 
 
  dx = X2 - X1; 
  dy = Y2 - Y1; 
 
  sdx = (dx < 0) ? -1 : 1; 
  sdy = (dy < 0) ? -1 : 1; 
 
  dx = sdx * dx + 1; 
  dy = sdy * dy + 1; 
 
  x = y = 0; 
 
  px = X1; 
  py = Y1; 
 
  if (dx >= dy) 
    { 
      for (x = 0; x < dx; x++) 
	{ 
          Callback(Surface, px, py, Color);
	 
	  y += dy; 
	  if (y >= dx) 
	    { 
	      y -= dx; 
	      py += sdy; 
	    } 
	  px += sdx; 
	} 
    } 
  else 
    { 
      for (y = 0; y < dy; y++) 
	{ 
 
          Callback(Surface, px, py, Color);
 
	  x += dx; 
	  if (x >= dy) 
	    { 
	      x -= dy; 
	      px += sdx; 
	    } 
	  py += sdy; 
	} 
    } 
   return 0;
}

void fadeout()
{
	int y;
	SDL_Rect rect;
	Uint32 time;

	rect.x = 0;
	rect.w = 800;
	rect.h = 1;
	for (y=0; y<600; y+=2) {
		time = SDL_GetTicks();
		rect.y = y;
		SDL_FillRect(Screen, &rect, SDL_MapRGB(Screen->format, 0,0,0));
		SDL_UpdateRects(Screen, 1, &rect);
		if (time == SDL_GetTicks())
			SDL_Delay(1);
	}
	for (y=599; y>0; y-=2) {
		rect.y = y;
		SDL_FillRect(Screen, &rect, SDL_MapRGB(Screen->format, 0,0,0));
		SDL_UpdateRects(Screen, 1, &rect);
	}
	
	/*
  int x,y;
  
  if (dofadeout==0) return;
  
  for (x=0;x<800;x++)
  {
    lock();
    for (y=0;y<300;y++)
    {
      PutPixel(Screen,x,y*2,SDL_MapRGB(Screen->format,0,0,0));
    }
    unlock();
    Update();
  }
  for (x=799;x>=0;x--)
  {
    lock();
    for (y=299;y>0;y--)
    {
      PutPixel(Screen,x,y*2+1,SDL_MapRGB(Screen->format,0,0,0));
    }
    unlock();
    Update();
  }*/
}

void Update()
{
  SDL_UpdateRects(Screen,blitrects_num,blitrects);
  blitrects_num=0;
}

void AddRect(int x1, int y1, int x2, int y2)
{
  int temp;

   /* Make sure X1 is before X2 */
   if (x2 < x1){
      temp = x2;
      x2 = x1;
      x1 = temp;
   }
   /* Make sure Y1 is before Y2 */
   if (y2 < y1){
      temp = y2;
      y2 = y1;
      y1 = temp;
   }
  blitrect.x = x1;
  blitrect.y = y1;
  blitrect.w = x2-x1+1;
  blitrect.h = y2-y1+1;
  if (x1<0) printf("x is too small in function AddRect!  -  %d\n",x1);else
  if (y1<0) printf("y is too small in function AddRect!  -  %d\n",y1);else
  if (x2>=800) printf("x is too big in function AddRect!  -  %d\n",x2);else
  if (y2>=600) printf("y is too big in function AddRect!  -  %d\n",y2);else {
      blitrects[blitrects_num]=blitrect;
      if (++blitrects_num>=RECTS_NUM-2)
      {printf("Too many blits!\n");blitrects_num--;Update();}
  }
}

void AddThisRect(SDL_Rect blitrect)
{
    blitrects[blitrects_num]=blitrect;
    if (++blitrects_num>=RECTS_NUM-2)
        {printf("Too many blits!\n");blitrects_num--;Update();}
}

void Blit(int Xpos,int Ypos,SDL_Surface *image)  //blits one GIF or BMP from the memory to the screen
{
  blitrect.x = Xpos;
  blitrect.y = Ypos;
  blitrect.w = image->w;
  blitrect.h = image->h;

  if (Xpos<-image->w) printf("WRONG BLIT: Xpos is too small! - %d\n",Xpos); else
  if (Xpos>=800) printf("WRONG BLIT: Xpos is too big! - %d\n",Xpos); else
  if (Ypos<-image->h) printf("WRONG BLIT: Ypos is too small! - %d\n",Ypos); else
  if (Ypos>=600) printf("WRONG BLIT: Ypos is too big! - %d\n",Ypos); else
  if ( SDL_BlitSurface(image, NULL, Screen, &blitrect) < 0 ) 
  {
    SDL_FreeSurface(image);
    ComplainAndExit();
  }
  blitrects[blitrects_num]=blitrect;
  blitrects_num++;
}

SDL_Surface *LoadImage(char *datafile, int transparent)   // reads one png into the memory
{
  SDL_Surface *pic,*pic2;
  char filename[200];
  int i=0;

  sprintf(filename,"%s/gfx/%s",DATAPATH,datafile);
  pic=IMG_Load(filename);
  while ( pic == NULL ) {
    strcpy(DATAPATH,PATH[i]);
    sprintf(filename,"%s/gfx/%s",DATAPATH,datafile);
    pic=IMG_Load(filename);
    i++;
    
    if (i>=PATHNUM)
    {
      fprintf(stderr,"Couldn't load %s: %s\n", filename, SDL_GetError());
      exit(2);
    }
  }
  if (transparent==3) return(pic);
  if (transparent==1)
    SDL_SetColorKey(pic,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(pic->format,0xFF,0xFF,0xFF));
  if (transparent==2)
    SDL_SetColorKey(pic,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(pic->format,0x00,0x00,0x00));
  pic2 = SDL_DisplayFormat(pic);
  SDL_FreeSurface(pic);
//  blit(0,0,pic2);
//  SDL_UpdateRect(screen,0,0,0,0);
  return (pic2);
}

void BlitToBB(int Xpos,int Ypos,SDL_Surface *image)  //blits one GIF or BMP from the memory to the screen
{
  blitrect.x = Xpos;
  blitrect.y = Ypos;
  blitrect.w = image->w;
  blitrect.h = image->h;
  if ( SDL_BlitSurface(image, NULL, BackBuffer, &blitrect) < 0 )
  {
    SDL_FreeSurface(image);
    ComplainAndExit();
  }
}

void BlitPart(int Xpos,int Ypos,SDL_Surface *image, SDL_Rect srcrect)
{
  blitrect.x = srcrect.x;											
  blitrect.y = srcrect.y;											
  blitrect.w = srcrect.w;											
  blitrect.h = srcrect.h;
  if ( SDL_BlitSurface(image, &srcrect , Screen, &blitrect) < 0 )
  {
    SDL_FreeSurface(image);
    ComplainAndExit();
  }
  blitrects[blitrects_num]=blitrect;
  blitrects_num++;
}

void FadeScreen(float speed)
{
  Sint32 now,i;

  SDL_BlitSurface(Screen,NULL,fadebuffer,NULL);
  now=SDL_GetTicks();
  for (i=255*speed;i>=0;i-=SDL_GetTicks()-now)
  {
    now=SDL_GetTicks();
    SDL_BlitSurface(fadebuffer,&blitrect,Screen,&blitrect);
    SDL_SetAlpha(BackBuffer,SDL_SRCALPHA,255-(int)(i/speed));
    Blit(0,0,BackBuffer);
    Update();
  }
  SDL_SetAlpha(BackBuffer,SDL_SRCALPHA,255);
  Blit(0,0,BackBuffer);
  Update();
}

int PutPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
    if (X<0) printf("X < 0 in function PutPixel! - %d\n",X); else
    if (X>=800) printf("X >= 800 in function PutPixel! - %d\n",X); else
    if (Y<0) printf("Y < 0 in function PutPixel! - %d\n",Y); else
    if (Y>=600) printf("Y >= 600 in function PutPixel! - %d\n",Y); else
    {
        _PutPixel(Surface,X,Y,Color);
        AddRect(X,Y,X,Y);
    }
    return 0;
}

int PutPixelC(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color)
{
    if (X<0) printf("X < 0 in function PutPixelC! - %d\n",X); else
    if (X>=800) printf("X >= 800 in function PutPixelC! - %d\n",X); else
    if (Y<0) printf("Y < 0 in function PutPixelC! - %d\n",Y); else
    if (Y>=600) printf("Y >= 600 in function PutPixelC! - %d\n",Y); else
    {
        _PutPixel(Surface,X,Y,Color);
        AddRect(X,Y,X,Y);
    }
    return 0;
}

void PutBackPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{
    SDL_Rect rect;
    
    rect.w=1;
    rect.h=1;
    rect.x=X;
    rect.y=Y;
    SDL_BlitSurface(BackBuffer, &rect, Surface, &rect);
    AddThisRect(rect);
}
