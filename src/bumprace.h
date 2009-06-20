#ifndef _BUMPRACE_H_
#define _BUMPRACE_H_

#include <SDL.h>
#include "gfx.h"
#include "font.h"

void BlitMenu();

#define SDL_SetAlpha(A,B,C) SDL_SetAlpha(A,0,0);SDL_SetAlpha(A,B,C)

typedef struct{
  float       slowdown,turbo_possible,sticky_possible,extra_time;
  float       realx,realy,xspeed,yspeed,turn,teleported;
  int         crashed,completed,racernum,points;
  SDL_Rect    oldrect;
  SDLKey      up,down,left,right,extra;
  SDL_Surface *racer[18];
}player;

typedef struct
{
  int shot_delay;      // delay between two shots
  float time_to_shot;  // time to the next shot
  float turn;          // current degree of rotation
  int x,y;             // coordinates
  int last_pic;        // last blitted picture
  int type;            // shooting method
} cannon_type;

extern cannon_type cannon;

typedef struct
{
  float x,y;            // coordinates
  float xspeed,yspeed;  // speed and direction
  float anim;           // # of shown picture
  SDL_Rect oldrect;     // the background has to be restored here
} shot_type;


#define MAX_PLAYER_NUM 2

extern player user[MAX_PLAYER_NUM];
extern SDL_Surface *screen, *back, *backbuffer, *fore[16], 
       *racer_pic[6], *selector_pic, *selectp_pic[2], *mode_select_pic[4],
       *title_pic;
extern float game_speed, gravity;
extern int particle,which_racer,pl,i,mode,playernum,Score,fullscreen,precision,dofadeout;
extern Uint8 *keys;
extern SDL_Event event;
extern struct BitFont font;
extern Sint32 now;
extern char text[200];
//extern char *DATAPATH[200];
extern void BlitMenu();  //blits menu for SelectRacer

#endif
