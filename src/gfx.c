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
SDL_Texture *sdlTexture;
SDL_Renderer *sdlRenderer;
SDL_Rect blitrect,blitrects[RECTS_NUM];
int blitrects_num=0;

void ComplainAndExit(void)
{
        fprintf(stderr, "Problem: %s\n", SDL_GetError());
        exit(1);
}

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

void init_SDL(char *window_name)  // sets the video mode
{
  Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI;
  int w=800, h=600;

  if (fullscreen)  
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  SDL_Window *sdl_window;
  if (SDL_CreateWindowAndRenderer(w, h, flags, &sdl_window, &sdlRenderer))
    ComplainAndExit();
  SDL_SetWindowTitle(sdl_window, window_name);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(sdlRenderer, w, h);

  Screen = SDL_CreateRGBSurface(0, w, h, 32,
      0x00FF0000,
      0x0000FF00,
      0x000000FF,
      0xFF000000);
  BackBuffer = SDL_CreateRGBSurface(0, w, h, 32,
      0x00FF0000,
      0x0000FF00,
      0x000000FF,
      0xFF000000);
  fadebuffer = SDL_CreateRGBSurface(0, w, h, 32,
      0x00FF0000,
      0x0000FF00,
      0x000000FF,
      0xFF000000);
  sdlTexture = SDL_CreateTexture(sdlRenderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      w, h);
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
		Update();
		if (time == SDL_GetTicks())
			SDL_Delay(1);
	}
	for (y=599; y>0; y-=2) {
		rect.y = y;
		SDL_FillRect(Screen, &rect, SDL_MapRGB(Screen->format, 0,0,0));
		Update();
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
  SDL_UpdateTexture(sdlTexture, NULL, Screen->pixels, Screen->pitch);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
  SDL_RenderPresent(sdlRenderer);
}

void AddRect(int x1, int y1, int x2, int y2)
{
  // noop after SDL2 migration
}

void AddThisRect(SDL_Rect blitrect)
{
  // noop after SDL2 migration
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
	return; // SDL2-MIGRATION
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
	return; // SDL2-MIGRATION
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
	return; // SDL2-MIGRATION
    SDL_Rect rect;
    
    rect.w=1;
    rect.h=1;
    rect.x=X;
    rect.y=Y;
    SDL_BlitSurface(BackBuffer, &rect, Surface, &rect);
    AddThisRect(rect);
}
