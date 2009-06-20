#include <SDL.h>
#define backbuffer BackBuffer

extern SDL_Surface *Screen,*BackBuffer,*fadebuffer;
extern SDL_Rect blitrect;
extern char DATAPATH[200];

// inits a lot of SDL stuff
void init_SDL();

// fades screen to BackBuffer
void FadeScreen(float speed);

// prints error and quits
void ComplainAndExit(void);

// Updates all blitted rects;
void Update();

// Puts a pixel (lock and unlock needed!)
extern int PutPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y, Uint32 Color);

// Add rect to update queue
void AddRect(int x1, int y1, int x2, int y2);
void AddThisRect(SDL_Rect blitrect);

// random number between a and b (inclusive)
int abrand(int a,int b);

// Draws a circle
int Circle (SDL_Surface *Surface, Sint32 X, Sint32 Y, Sint32 Rad, float Aspect, Uint32 Color);

// Draws the background (as a circle)
int UndoCircle (SDL_Surface *Surface, Sint32 X, Sint32 Y, Sint32 Rad, float Aspect);

// returns the flipped surface
SDL_Surface *FlippedSurface(SDL_Surface *Surface);

// returns the color value of a pixel
Uint32 GetPixel (SDL_Surface *Surface, Sint32 X, Sint32 Y);

void lock();
void unlock();
void fadeout();
SDL_Surface *LoadImage(char *datafile, int transparent);
void Blit(int Xpos,int Ypos,SDL_Surface *image);
void BlitToBB(int Xpos,int Ypos,SDL_Surface *image);   //blits to BackBuffer
void BlitPart(int Xpos,int Ypos,SDL_Surface *image, SDL_Rect srcrect);
void PutBackPixel(SDL_Surface *Surface, Sint32 x, Sint32 y);
