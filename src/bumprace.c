/*  BumpRace: an easy-to-learn arcade game for up to two players
    Copyright (C) 2006 Karl Bartel

    This program is free software; you can redistribute it and/or modify        
    it under the terms of the GNU General Public License as published by        
    the Free Software Foundation; either version 2 of the License, or           
    (at your option) any later version.                                         
                                                                                
    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of              
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
    GNU General Public License for more details.                
                                                                               
    You should have received a copy of the GNU General Public License           
    along with this program; if not, write to the Free Software                 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA   
                                                                                
    Karl Bartel
    Cecilienstr. 14                                                    
    12307 Berlin                                                  
    GERMANY
    karl42@gmail.com                                                      
*/                                                                            
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL_timer.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef SOUND
#include "SDL_mixer.h"
#endif
#include "levels.h"
#include "levels2.h"
#include "particles.h"
#include "menu.h"
#include "bumprace.h"
#include "font.h"
#include "options.h"

#ifndef VERSION
#define VERSION "1.5.3"
#endif
#define MAX_PLAYER_NUM 2
#define FOREGROUND_TILE_NUM 8
#define MAX_SHOTNUM 50
#define BPP 0
#define RR 3                 //this is a value of unprecision for the racer collision

    //the players
       player user[MAX_PLAYER_NUM]; int pl=0,playernum=2;
    //general setting
       int final=1,fullscreen=1,pageflip=0,sound=1,precision=10,fps=0,particle=1,mode,dofadeout=1,levelset=0;
       const int NUMBER_OF_LEVELS[2]={21,9};
    //level settings
       float gravity=0,turbo=5,laser_switch;
       int startx,starty;
    //sounds
#ifdef SOUND
       char modname[20]="lizard.mod";
       Mix_Chunk *explode_sound,*winlevel_sound,*teleport_sound;     
       Mix_Music *music;
#endif              
     //images
       SDL_Surface *timeout_pic, *explosion_pic, *title_pic,
                   *back, *fore[16], *pic_completed,*pic_crashed,
                   *racer_pic[6], *cannon_pic[21], *shot_pic[5],
                   *mode_select_pic[4], *selector_pic, *selectp_pic[2],
		   *Font,*laser_pic[9];
     //event handling  
       SDL_Event event;  Uint8 *keys;
     //game states
       int endgame=0,dontshow=0,levelnum=0,quit=0,already=0,laser_state;
     //level vars
       char finished[28]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // is level number X finished (1 or 0)
       int levels_completed=0,Stage;
     //assorted
       float game_speed=100,average_speed,time_since_start,accel_speed,turn_speed;
       int frame_count=0,count_start=0,Score=0; 
       float frames_per_second,PARTICLES_OF_EXHAUST=150;
       int stop=0,stop2=0,infox,infoy,shotnum=0,cannonnum;
       Sint32 now,lifetime=800000,last_lifetime,NextParticle;
       char text[200];
       int y,x,time_bonus;
       shot_type shot[MAX_SHOTNUM];
       cannon_type cannon;


void ComplainAndExit(void);

void free_memory()
{
#ifdef SOUND
  if (sound)
  {
    Mix_FreeMusic(music);
    Mix_FreeChunk(explode_sound);
    Mix_FreeChunk(winlevel_sound);
    Mix_FreeChunk(teleport_sound);
    Mix_CloseAudio();
  }
#endif  
}

void timing()
{
#ifdef SOUND
  if ((!Mix_PlayingMusic())&&(sound)) {Mix_PlayMusic(music,1);}
#endif
  time_since_start+=game_speed;
  average_speed=time_since_start/frame_count;
  //printf("gamespeed: %f    average: %f \n",game_speed,average_speed);
  lifetime-=game_speed;
  if ( keys[SDLK_ESCAPE] == SDL_PRESSED ) 
    {quit=1;endgame=1;}  
  frame_count++;
  frames_per_second=frame_count*1000/((SDL_GetTicks()-count_start)+1);
  game_speed=(SDL_GetTicks()-now)*7;
  while (game_speed<140) {
      SDL_Delay(1);
      game_speed=(SDL_GetTicks()-now)*7;
  }
 // printf("x:%f  y:%f  delay:%d   fps%f\n",user[pl].realx,user[pl].realy,(SDL_GetTicks()-now),frames_per_second);
  now=SDL_GetTicks();
}

void corner_collision(  Uint8 xblock, Uint8 yblock)
{
  float hyp,alpha,xold,yold;
  xold=user[pl].xspeed;
  yold=user[pl].yspeed;
  hyp=hypot(user[pl].realx+15-xblock*40,user[pl].realy+15-yblock*40);
  if (hyp<15-RR)
  {
    alpha=atan(-(user[pl].realx+15-xblock*40)/(user[pl].realy+15-yblock*40));
    user[pl].xspeed=2*(xold*cos(alpha)+yold*sin(alpha))*cos(alpha)-xold;
    user[pl].yspeed=2*(xold*cos(alpha)+yold*sin(alpha))*sin(alpha)-yold;
    already=1;
  }
}

void ResetLevels() {
	for (i=0;i<=NUMBER_OF_LEVELS[levelset];i++) {
		int x,y;
		
		finished[i]=0;
		for (x=0; x<20; x++) {
			for (y=0; y<15; y++) {
				if (levelset == 0)
					map[i][y][x] = origMap[i][y][x];
				else
					map[i][y][x] = origMap2[i][y][x];
			}
		}
	}
}

/**
 * If players p1 and p2 collide, moves them out of the collision and makes them bounce.
 */
void player_collision(int p1, int p2) {
	const float radius = 12; /**< assumed radius of ships */

	// calculate distance between ships
	float dist_x = user[p1].realx - user[p2].realx;
	float dist_y = user[p1].realy - user[p2].realy;
	float dist = sqrt((dist_x * dist_x) + (dist_y * dist_y));

	// abort if ships do not touch/intersect
	if (dist > (radius + radius)) return;

	// calculate penetration depth
	float depth = (radius + radius) - dist;

	// calculate unity vector of hit direction
	float hit_x = dist_x / sqrt((dist_x * dist_x) + (dist_y * dist_y));
	float hit_y = dist_y / sqrt((dist_x * dist_x) + (dist_y * dist_y));

	// calculate fraction of speeds in direction of hit vector
	float scalar1 = (user[p1].xspeed * hit_x) + (user[p1].yspeed * hit_y);
	float scalar2 = (user[p2].xspeed * hit_x) + (user[p2].yspeed * hit_y);

	// abort if nobody seems to have caused the collision
	if (fabsf(scalar1) + fabsf(scalar2) == 0) return;

	// calculate "contribution" of each ship to the collision as a fraction of 1, judging by its speed
	// TODO: this penalizes a ship moving *out* of the collision, too
	float contrib1 = fabsf(scalar1) / (fabsf(scalar1) + fabsf(scalar2));
	float contrib2 = 1-contrib1;

	// move ships just out of collision with total distance based on "contribution" figure
	user[p1].realx += hit_x * (depth * contrib1);
	user[p1].realy += hit_y * (depth * contrib1);
	user[p2].realx -= hit_x * (depth * contrib2);
	user[p2].realy -= hit_y * (depth * contrib2);

	// calculate changes in speed
	float delta1_x = scalar1 * hit_x;
	float delta1_y = scalar1 * hit_y;
	float delta2_x = scalar2 * hit_x;
	float delta2_y = scalar2 * hit_y;

	// modify speeds
	user[p1].xspeed -= delta1_x;
	user[p1].yspeed -= delta1_y;
	user[p1].xspeed += delta2_x;
	user[p1].yspeed += delta2_y;
	user[p2].xspeed += delta1_x;
	user[p2].yspeed += delta1_y;
	user[p2].xspeed -= delta2_x;
	user[p2].yspeed -= delta2_y;
}

int extra_pressed(int pl) {
  return (
  	(keys[user[pl].extra] == SDL_PRESSED) ||
	(playernum == 1 && (
		// if single player, any ctrl-key will do
		(keys[user[0].extra] == SDL_PRESSED) ||
		(keys[user[1].extra] == SDL_PRESSED)
	))
  );
}

void kollision()  //calcualtes all collisions
{
  Uint8 x,y,x2,y2,sticky;
  SDL_Rect reblit_back;

  if (extra_pressed(pl) && user[pl].sticky_possible) {
		  sticky = 1;
  } else {
		  sticky = 0;
  }

  // handle collisions with other players
  // TODO: does this belong here?
  int pl2;
  for (pl2 = pl+1; pl2 < playernum; pl2++) {
    player_collision(pl, pl2);
  }

  if (already) {already=0;} else
  for (y=0;y<=14;y++) {
    for (x=0;x<=19;x++) {
//wall
      if (((map[levelnum][y][x]==1)||(map[levelnum][y][x]>10)||(map[levelnum][y][x]==8))&&((user[pl].realx+30>x*40)&&(user[pl].realx<x*40+38)&&(user[pl].realy+30>y*40)&&(user[pl].realy<y*40+38))&&(!stop))
      { 
		  // edges
		  if ((user[pl].realy+15<y*40)&&(user[pl].realy+30-RR>y*40)&&(!already)&&
			  (user[pl].realx+15<(x+1)*40)&&(user[pl].realx+15>x*40)&&(map[levelnum][y-1][x]!=1))
		  {
			  if (sticky) {user[pl].yspeed*=0.1;user[pl].xspeed*=0.1;} else if (user[pl].yspeed>0) {user[pl].yspeed=-user[pl].yspeed;already=1;}
		  }
		  if ((user[pl].realx+15>(x+1)*40)&&(user[pl].realx+RR<(x+1)*40)&&(!already)&&
			  (user[pl].realy+15<(y+1)*40)&&(user[pl].realy+15>y*40)&&(map[levelnum][y][x+1]!=1))	    
		  {
			  if (sticky) {user[pl].yspeed*=0.1;user[pl].xspeed*=0.1;} else if (user[pl].xspeed<0) {user[pl].xspeed=-user[pl].xspeed;already=1;}
		  }
		  if ((user[pl].realy+15>(y+1)*40)&&(user[pl].realy+RR<(y+1)*40)&&(!already)&&
			  (user[pl].realx+15<(x+1)*40)&&(user[pl].realx+15>x*40)&&(map[levelnum][y+1][x]!=1))
		  {
			  if (sticky) {user[pl].yspeed*=0.1;user[pl].xspeed*=0.1;} else if (user[pl].yspeed<0) {user[pl].yspeed=-user[pl].yspeed;already=1;}
		  }
		  if ((user[pl].realx+15<x*40)&&(user[pl].realx+30-RR>x*40)&&(!already)&&
			  (user[pl].realy+15<(y+1)*40)&&(user[pl].realy+15>y*40)&&(map[levelnum][y][x-1]!=1))	    
		  {
			  if (sticky) {user[pl].yspeed*=0.1;user[pl].xspeed*=0.1;} else if (user[pl].xspeed>0) {user[pl].xspeed=-user[pl].xspeed;already=1;}
		  }
		  // corners
		  if ((user[pl].realx+15>(x+1)*40)&&(user[pl].realy+15<y*40)&&(!already)&&(map[levelnum][y][x+1]!=1)&&(map[levelnum][y-1][x]!=1))
			  /*if ((user[pl].xspeed<0)&&(user[pl].yspeed>0))*/ corner_collision(x+1,y);
		  if ((user[pl].realx+15<x*40)&&(user[pl].realy+15<y*40)&&(!already)&&(map[levelnum][y][x-1]!=1)&&(map[levelnum][y-1][x]!=1))
			  /*if ((user[pl].xspeed>0)&&(user[pl].yspeed>0))*/ corner_collision(x,y);
		  if ((user[pl].realx+15>(x+1)*40)&&(user[pl].realy+15>(y+1)*40)&&(!already)&&(map[levelnum][y][x+1]!=1)&&(map[levelnum][y+1][x]!=1))
			  /*if ((user[pl].xspeed<0)&&(user[pl].yspeed<0))*/ corner_collision(x+1,y+1);
		  if ((user[pl].realx+15<x*40)&&(user[pl].realy+15>(y+1)*40)&&(!already)&&(map[levelnum][y][x-1]!=1)&&(map[levelnum][y+1][x]!=1))
			  /*if ((user[pl].xspeed>0)&&(user[pl].yspeed<0))*/ corner_collision(x,y+1);
      }
      if (stop) {stop--;}
      if (stop2) {stop2--;}
//lightning
      if ((map[levelnum][y][x]==2)&&((user[pl].realx+24>x*40)&&(user[pl].realx<x*40+32)&&(user[pl].realy+24>y*40)&&(user[pl].realy<y*40+32)))
      { 
	user[pl].crashed=1;
      }
//stopper
      if ((map[levelnum][y][x]==3)&&((user[pl].realx+24>x*40)&&(user[pl].realx<x*40+32)&&(user[pl].realy+24>y*40)&&(user[pl].realy<y*40+32))&&(!stop2))
      { 
        user[pl].xspeed=0;
	user[pl].yspeed=0;
	stop2=30000;
      }
//finish
      if ((map[levelnum][y][x]==4)&&((user[pl].realx+14>x*40)&&(user[pl].realx<x*40+22)&&(user[pl].realy+14>y*40)&&(user[pl].realy<y*40+22))&&(!stop2))
      { 
        user[pl].completed=1;
      }
//teleporter
      if ((map[levelnum][y][x]==5)&&((user[pl].realx+10>x*40)&&(user[pl].realx<x*40+18)&&(user[pl].realy+10>y*40)&&(user[pl].realy<y*40+18)))
      { 
        for (y2=0;y2<=14;y2++) {
          for (x2=0;x2<=19;x2++) {
            if (((x2!=x)||(y2!=y))&&(map[levelnum][y2][x2]==5)&&(user[pl].teleported==0))
	    {user[pl].realx=x2*40+2;user[pl].realy=y2*40+2;user[pl].teleported=30000000;	
#ifdef SOUND
            if (sound) {Mix_PlayChannel(1,teleport_sound,0);}
#endif
	  }}
        }
      }
      if (user[pl].teleported>0) {user[pl].teleported-=game_speed;}
      if (user[pl].teleported<0) {user[pl].teleported=0;}
//time bonus
      if ((map[levelnum][y][x]==6)&&((user[pl].realx+10>x*40)&&(user[pl].realx<x*40+18)&&(user[pl].realy+10>y*40)&&(user[pl].realy<y*40+18)))
      { 
        map[levelnum][y][x]=0;lifetime+=160000;
	reblit_back.x=x*40+20-fore[5]->w/2;
        reblit_back.y=y*40+20-fore[5]->h/2;
        reblit_back.w=fore[5]->w;
        reblit_back.h=fore[5]->h; 
        BlitPart(0,0,back,reblit_back);
	SDL_BlitSurface( back, &reblit_back, backbuffer, &reblit_back );
        if (!fullscreen) {SDL_UpdateRects(Screen,1,&reblit_back);}    
      }
//laser
      if ((map[levelnum][y][x]==7)&&((user[pl].realx+24>x*40)&&(user[pl].realx<x*40+32)&&(user[pl].realy+24>y*40)&&(user[pl].realy<y*40+32)))
      { 
	if (laser_state) user[pl].crashed=1;
      }
    }
  } 
}

/**
 * Add forward thrust to active player's ship.
 * Set turbo_factor to 1 for normal acceleration
 */
void thrust(float turbo_factor)
{
    float deltaspeedx, deltaspeedy;
	
	deltaspeedy = -cos(user[pl].turn*0.01745)*turbo_factor*(game_speed/precision);      // 0.01745=(2*PI)/360 thats RAD to DEG
	deltaspeedx =  sin(user[pl].turn*0.01745)*turbo_factor*(game_speed/precision);
    user[pl].yspeed+=deltaspeedy;
    user[pl].xspeed+=deltaspeedx;
    NextParticle-=game_speed;
	 
	float spread_percent;
	switch( user[pl].racernum ) {
		case 0: // SlowPoke
			spread_percent = 100;
			break;
		case 1: // Normal
			spread_percent = 50;
			break;
		case 2: // Speedy
			spread_percent = 10;
			break;
		case 3: // Spinny
			spread_percent = 120;
			break;
		default: // OOPS!
			spread_percent = 0;
	}
	if (turbo_factor > 1) spread_percent = 100;
	
	while (NextParticle<0) {
			NewParticles(user[pl].turn, user[pl].realx, user[pl].realy, spread_percent);
			NextParticle+=350;
	}
}

//void thrust()
//{
//  float deltaspeedx, deltaspeedy;
//  
//  deltaspeedy = -cos(user[pl].turn*0.01745)*(game_speed/precision);      // 0.01745=(2*PI)/360 thats RAD to DEG
//  deltaspeedx =  sin(user[pl].turn*0.01745)*(game_speed/precision);
//  user[pl].yspeed+=deltaspeedy;
//  user[pl].xspeed+=deltaspeedx;
//  NextParticle-=game_speed;
//  while (NextParticle<0) {
//      NewParticles(user[pl].turn, user[pl].realx, user[pl].realy);
//      NextParticle+=350;
//  }
//}

void HandleRacer() //reads keys and sets new coordinates
{
  float alpha,hyp;
  for(i=0;i<precision;i++)     // multiple times for a more precise collision
  {
    turn_speed=0.05*(game_speed/precision);
    accel_speed=400000/(game_speed/precision);

    SDL_PollEvent(&event);
    keys = SDL_GetKeyState(NULL);
    if ( keys[user[pl].up] == SDL_PRESSED ) {
      thrust(1);
    }
    if ( keys[user[pl].right] == SDL_PRESSED ) {
      user[pl].turn+=turn_speed;
    }
    if ( keys[user[pl].left] == SDL_PRESSED ) {
      user[pl].turn-=turn_speed;
    }
    if (extra_pressed(pl) && user[pl].turbo_possible) {
      thrust(turbo);
    }
    user[pl].realx=user[pl].realx+user[pl].xspeed/accel_speed;
    user[pl].realy=user[pl].realy+user[pl].yspeed/accel_speed;
    if (user[pl].realx<-6) {user[pl].realx=-6;user[pl].xspeed=-user[pl].xspeed;}
    if (user[pl].realx>774) {user[pl].realx=774;user[pl].xspeed=-user[pl].xspeed;}
    if (user[pl].realy<-6) {user[pl].realy=-6;user[pl].yspeed=-user[pl].yspeed;}
    if (user[pl].realy>574) {user[pl].realy=574;user[pl].yspeed=-user[pl].yspeed;}
    kollision();
  }
//gravity
  user[pl].yspeed+=(gravity)*game_speed;
//slowdown
  if (user[pl].slowdown>0)
  {
    if (user[pl].xspeed!=0) {alpha=atan(user[pl].yspeed/user[pl].xspeed);} else alpha=1;
    hyp=hypot(user[pl].xspeed,user[pl].yspeed);
    if ((user[pl].xspeed<0))
    {  
      if (!gravity) {user[pl].yspeed+=sin(alpha)*hyp*user[pl].slowdown*game_speed;}
      {user[pl].xspeed+=cos(alpha)*hyp*user[pl].slowdown*game_speed;}
    }else
    {  
      if (!gravity) {user[pl].yspeed-=sin(alpha)*hyp*user[pl].slowdown*game_speed;}
      {user[pl].xspeed-=cos(alpha)*hyp*user[pl].slowdown*game_speed;}
    }
  }
}

void DontReadKeys()
{
  SDL_EventState(SDL_KEYUP, SDL_IGNORE);                                  
  SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);                                  
}

void UndrawLaser(int update)
{
  SDL_Rect src_rect;
 
  Uint8 x,y;  
  for (y=0;y<=14;y++) {
    for (x=0;x<=19;x++) {
      if (map[levelnum][y][x]==7){ 
        src_rect.x=x*40;
        src_rect.y=y*40;
        src_rect.w=40;
        src_rect.h=40;
        BlitPart(x*40,y*40,backbuffer,src_rect);
//        SDL_BlitSurface(back , &src_rect , backbuffer, &src_rect);
	if (update) SDL_UpdateRect(Screen,x*40,y*40,40,40);
      }
    }
  } 
}

void DrawLaser()
{
  SDL_Rect src_rect;
 
  Uint8 x,y;  
  for (y=0;y<=14;y++) {
    for (x=0;x<=19;x++) {
      if (map[levelnum][y][x]==7){ 
        src_rect.x=x*40;
        src_rect.y=y*40;
        src_rect.w=40;
        src_rect.h=40;
	Blit(x*40   ,y*40,laser_pic[abrand(0,8)]);
	Blit(x*40+15,y*40,laser_pic[abrand(0,8)]);
	Blit(x*40+30,y*40,laser_pic[abrand(0,8)]);
	SDL_UpdateRect(Screen,x*40,y*40,40,40); 
      }
    }
  } 
}

void draw_cannon()
{
  int pic=0;

  while (cannon.turn<0) cannon.turn+=360;
  cannon.turn+=90;               // bad fix -> should be changed!
  pic=((cannon.turn)*21/360);
  if (pic>20) pic-=21;           // bad fix -> should be changed!
  if (pic!=cannon.last_pic)
  {
    Blit(cannon.x*40,cannon.y*40,cannon_pic[pic]);
    BlitToBB(cannon.x*40,cannon.y*40,cannon_pic[pic]);
  }
  cannon.last_pic=pic;
  cannon.turn-=90;               // bad fix -> should be changed! 
  while (cannon.turn>360) cannon.turn-=360;
}

void checks_common()
{
//laser
  if ((laser_state==0)&&(div(SDL_GetTicks(),laser_switch)).rem<(int)(laser_switch/2))
    laser_state=1;
  if ((laser_state==1)&&(div(SDL_GetTicks(),laser_switch).rem>(int)(laser_switch/2))) {
    laser_state=0;
    UndrawLaser(1);
  }
//cannon
  if (cannonnum)
  {
    cannon.turn+=game_speed/(15+cannon.type*15);
    if (cannon.turn>360) cannon.turn-=360;
    if (cannon.turn<0) cannon.turn+=360;
    cannon.time_to_shot-=game_speed*1;
    if ((cannon.time_to_shot<0)&&(shotnum<MAX_SHOTNUM))
    {
      cannon.time_to_shot=cannon.shot_delay;
      shot[shotnum].xspeed=cos((cannon.turn+900*(1/(float)(15+cannon.type*15)))*0.01745)*0.01;
      shot[shotnum].yspeed=sin((cannon.turn+900*(1/(float)(15+cannon.type*15)))*0.01745)*0.01;   
      shot[shotnum].x=cannon.x*40+20-shot_pic[0]->w/2+1400*shot[shotnum].xspeed;
      shot[shotnum].y=cannon.y*40+20-shot_pic[0]->h/2+1400*shot[shotnum].yspeed;
      shotnum++;
      //printf("%d\n",(int)cannon.turn);
    }
  }
}

void DisplayMsg(SDL_Surface *img, float speed)
{
  Blit(400-img->w/2,250,img);
  now=SDL_GetTicks();
  for (i=255*speed;i>=0;i-=(float)(SDL_GetTicks()-now)/2)
  {
    timing();
    SDL_SetAlpha(backbuffer,(SDL_SRCALPHA),255);
    UndrawParticles();
    HandleParticles();
    DisplayParticles();
    Blit(400-img->w/2,250,img);
    SDL_SetAlpha(backbuffer,(SDL_SRCALPHA),(int)(i/speed));
    SDL_BlitSurface(backbuffer,&blitrect,Screen,&blitrect);
    Update();
  }
  BlitToBB(400-img->w/2,250,img);
  SDL_SetAlpha(backbuffer,SDL_SRCALPHA,255);
  while (NullParticle>0) {
    timing();
    UndrawParticles();
    HandleParticles();
    DisplayParticles();
    Update();
  }
  SDL_SetAlpha(backbuffer,SDL_SRCALPHA,255);
}

void DrawExplosion()
{
  int x,y;
 
  x=abrand(-14,+8);y=abrand(-14,+8);
  Blit(user[pl].realx+x,user[pl].realy+y,explosion_pic);
  BlitToBB(user[pl].realx+x,user[pl].realy+y,explosion_pic);
}

void checks_1p()
{
//level end
  if ( user[pl].completed )
  {  
#ifdef SOUND
    if (sound) {Mix_PlayChannel(-1,winlevel_sound,0);}    
#endif
    DontReadKeys();
    DisplayMsg(pic_completed,1);
    SDL_Delay(1000);
    endgame=1;
    lifetime+=(int)(150000*(1+(time_bonus)/100));
  }
//crash
  if ( user[pl].crashed )
  {
    DontReadKeys();
#ifdef SOUND    
    if (sound) {Mix_PlayChannel(-1,explode_sound,0);}    
#endif
    SDL_SetAlpha(explosion_pic,(SDL_SRCALPHA),255-80);
    DrawExplosion();
    DrawExplosion();
    DrawExplosion();
    DisplayMsg(pic_crashed,1);
    SDL_Delay(1000);
    endgame=1;
    fadeout();
  }
//timeout
  if (lifetime<0) 
  {
    DontReadKeys();
    DisplayMsg(timeout_pic,1);
    SDL_Delay(1000);
    endgame=1;
    quit=1;
    fadeout();
  }
}

void checks_2p()
{
  for (pl=0;pl<playernum;pl++)
  {
    if ((mode==1)&&(!endgame))
    {
      if ( user[pl].completed )
      {
#ifdef SOUND
        if (sound) {Mix_PlayChannel(-1,winlevel_sound,0);}    
#endif
        DontReadKeys();
        DisplayMsg(pic_completed,1);
	user[pl].points+=1;
        endgame=1;
      }
      if ( user[pl].crashed )
      {
#ifdef SOUND
        if (sound) {Mix_PlayChannel(-1,explode_sound,0);}    
#endif
        user[pl].realx=startx;user[pl].realy=starty;
        user[pl].xspeed=0;user[pl].yspeed=0;user[pl].crashed=0;
      }
    }
    if ((mode==2)&&(!endgame))
    {
      if ( user[pl].completed )
      {
#ifdef SOUND
	if (sound) {Mix_PlayChannel(-1,winlevel_sound,0);}    
#endif
        DontReadKeys();
        DisplayMsg(pic_completed,1);
        SDL_Delay(1000);
        endgame=1;
        lifetime+=(int)(150000*(1+(time_bonus)/100));
      }
      if (lifetime<0) 
      {
        DontReadKeys();
        DisplayMsg(timeout_pic,1);
        endgame=1;
	quit=1;
        fadeout();
      }
      if ( user[pl].crashed )
      {
#ifdef SOUND
        if (sound) {Mix_PlayChannel(-1,explode_sound,0);}    
#endif
        user[pl].realx=startx;user[pl].realy=starty;
        user[pl].xspeed=0;user[pl].yspeed=0;user[pl].crashed=0;
      }
    }
  }
}

void blit_lifetime()
{
  float x,y,xcol;
  SDL_Rect timerect;
  
  if (mode==1)
  {
    Blit(infox*40   ,infoy*40,fore[10]);
    Blit(infox*40+40,infoy*40,fore[11]);
    Blit(infox*40+80,infoy*40,fore[12]);
    PutString(Screen, infox*40+29, infoy*40+14, "P1   P2");
    sprintf(text,"%2d    %2d",user[0].points,user[1].points);
    PutString(Screen, infox*40+26, infoy*40+24, text );
    last_lifetime=user[0].points + user[1].points;
    timerect.x=infox*40+5;
    timerect.y=infoy*40+24;
    timerect.w=110;
    timerect.h=10;
    SDL_BlitSurface(Screen,&timerect,backbuffer,&timerect);
    if (!fullscreen) {SDL_UpdateRects(Screen,1,&timerect);}    
  }
  else
  {
    if ((int)(lifetime/10000)!=(int)(last_lifetime/10000))
    {
      Blit(infox*40   ,infoy*40,fore[10]);
      Blit(infox*40+40,infoy*40,fore[11]);
      Blit(infox*40+80,infoy*40,fore[12]);
      PutString(Screen, infox*40+17, infoy*40+10, "Time Left");
      last_lifetime=lifetime;
      lock();
      for (x=0;x<(lifetime/10000);x++) {
        if (x*3<255) xcol=x*3; else xcol=255;
        for (y=0;y<10;y++) {
          PutPixel(Screen,x+infox*40+60-(lifetime/20000),y+infoy*40+28,SDL_MapRGB(Screen->format,255-xcol,xcol,y));    
        }
      }
      unlock();
      timerect.x=infox*40+5;
      timerect.y=infoy*40;
      timerect.w=110;
      timerect.h=38;
      if (!fullscreen) {SDL_UpdateRects(Screen,1,&timerect);}    
      SDL_BlitSurface(Screen,&timerect,backbuffer,&timerect);
    }
  }    
}

SDL_Rect SetShotRect(int i)
{
  SDL_Rect rect;

  rect.x=shot[i].x;
  rect.y=shot[i].y;
  rect.w=shot_pic[0]->w;
  rect.h=shot_pic[0]->h;
  if (rect.w+rect.x>800) rect.w=800-rect.x;
  if (rect.h+rect.y>600) rect.h=600-rect.y;
  if (rect.x<0) {rect.w=rect.w+rect.x;rect.x=0;}
  if (rect.y<0) {rect.h=rect.h+rect.y;rect.y=0;}
  return (rect);
}

void UndrawShots()
{
  for (i=0;i<shotnum;i++)
  {
    shot[i].oldrect=SetShotRect(i);
    BlitPart(0,0,backbuffer,shot[i].oldrect);
  }
}

void HandleShots()
{
  int i2;
  for (i=0;i<shotnum;i++)
  {
    shot[i].anim+=game_speed*0.004;
    while ((int)shot[i].anim>4) shot[i].anim-=5;
    shot[i].x+=shot[i].xspeed*game_speed;
    shot[i].y+=shot[i].yspeed*game_speed;
    if ((shot[i].x<=-shot_pic[0]->w)||(shot[i].x>=800)
       ||(shot[i].y<=-shot_pic[0]->h)||(shot[i].y>=600))
       {
         SDL_UpdateRects( Screen, 1, &shot[i].oldrect );
         for (i2=i;i2<shotnum-1;i2++)
	   shot[i2]=shot[i2+1];
	 shotnum--;
       }
    for (pl=0;pl<playernum;pl++)
    if (hypot(shot[i].x+10-(user[pl].realx+15),shot[i].y+10-(user[pl].realy+15))<16-RR)
    {
      user[pl].crashed=1;
    }
  }
}

void DisplayShots()
{
  for (i=0;i<shotnum;i++)
    Blit(shot[i].x,shot[i].y,shot_pic[(int)shot[i].anim]);
}

void UndrawRacer()
{
  BlitPart(0,0,backbuffer,user[pl].oldrect);
}

void DisplayRacer()
{
  int pic=0,i;

  if (user[pl].turn<0) {user[pl].turn=user[pl].turn+360;}
  if (user[pl].turn>360) {user[pl].turn=user[pl].turn-360;}
  for (i=0;i<=17;i++)   //selects the correct picture
    if (user[pl].turn+10>i*20) {pic=i;}
  Blit(user[pl].realx,user[pl].realy,user[pl].racer[pic]);
  user[pl].oldrect=blitrect;
}

void HandleAndDraw_RacerParticlesAndShots()
{
  if (laser_state) UndrawLaser(0);
  UndrawShots();
  UndrawParticles();
  for (pl=0;pl<playernum;pl++)
    UndrawRacer();                      
  HandleShots();
  HandleParticles();
  for (pl=0;pl<playernum;pl++)
    HandleRacer();           
  draw_cannon();
  DisplayShots();
  if (laser_state) DrawLaser();
  DisplayParticles();
  for (pl=0;pl<playernum;pl++)
    DisplayRacer();
}

void blit_fore() //blits the leves foreground (used only one time per level)
{
  Uint8 x,y;
  for (y=0;y<=14;y++) {
    for (x=0;x<=19;x++) {
      if ((map[levelnum][y][x]!=0)&&(map[levelnum][y][x]!=7))
          Blit(x*40-fore[map[levelnum][y][x]-1]->w/2+20,
	       y*40-fore[map[levelnum][y][x]-1]->h/2+20,
	       fore[map[levelnum][y][x]-1]);
    }
  }
}

void LookForInfo() //looks for the info box in this level
{
  infox=-1;
  infoy=-1;
  cannon.x=-1;
  for (y=0;y<=14;y++) {
    for (x=0;x<=19;x++) {
      if ((map[levelnum][y][x]==11)&&(infoy==-1)) 
      { 
        infox=x;
        infoy=y;
      }
      if (map[levelnum][y][x]==8)
      { 
        cannon.y=y;
        cannon.x=x;
      }
    }
  } 
  if (cannon.x<0) cannonnum=0; else cannonnum=1;
}

void load_images()
{
  Uint8 i;  

  racer_pic[0]=LoadImage("racer_big.gif",0);
  racer_pic[1]=LoadImage("racerb_big.gif",0);
  racer_pic[2]=LoadImage("racere_big.gif",0);
  racer_pic[4]=LoadImage("racerd_big.gif",0);
  racer_pic[3]=LoadImage("racerf_big.gif",0);
  racer_pic[5]=LoadImage("highscore.png",0);
  timeout_pic=LoadImage("timeout.png",3);
  explosion_pic=LoadImage("explosion1.gif",1);
  pic_completed=LoadImage("completed.png",3);  
  pic_crashed=LoadImage("crashed.png",3);  
  back=LoadImage("back2.jpg",0);
  mode_select_pic[0]=LoadImage("1player.jpg",0);
  mode_select_pic[1]=LoadImage("2playercomp.jpg",0);
  mode_select_pic[2]=LoadImage("2playerteam.jpg",0);
  mode_select_pic[3]=LoadImage("quit.jpg",0);
  selectp_pic[0]=LoadImage("selectp1.jpg",0);
  selectp_pic[1]=LoadImage("selectp2.jpg",0);
  selector_pic=LoadImage("select.gif",1);
  for (i=0;i<=FOREGROUND_TILE_NUM-1;i++)
  {
    sprintf(text,"fore%d.png",i+1);
    fore[i]=LoadImage(text,3);
  }
  for (i=10;i<16;i++)
  {
    sprintf(text,"fore%d.gif",i+1);
    fore[i]=LoadImage(text,0);
  }
  for (i=0;i<21;i++)
  {
    sprintf(text,"cannon%d.jpg",i+1);
    cannon_pic[i]=LoadImage(text,0);
  }
  for (i=0;i<5;i++)
  {
    sprintf(text,"star%d.gif",i+1);
    shot_pic[i]=LoadImage(text,1);
  }
  for (i=0;i<9;i++)
  {
    sprintf(text,"laser%d.png",i+1);
    laser_pic[i]=LoadImage(text,3);
  }
}

void TextHelp(char *argv[])
{
    printf("\nBumpRace Version %s\n",VERSION);
    puts("The newest version can be obtained at http://www.linux-games.com/\n");
    printf("Usage: %s [options]\n",argv[0]);
    puts("  [-f | --fullscreen]         start in fullscreen mode (default)");
    puts("  [-w,| --windowed]           start in windowed mode");
    puts("  [-s,| --nosound]            start without sound");
    puts("  [-n,| --notfinal]           no title screen (nice for developers)");
    puts("  [-t,| --noparticles]        turns of particles. good for slow computers.");
    puts("  [-o,| --nofadeout]          no fadeout after crash (for slow computers)");
    puts("  [     --precision]        	sets the precision of the collisions (default=10)");
    puts("  [-h,| --help]               this text\n");
    
    exit(0);
}

void ReadCommandLine(char *argv[])
{
  int i;
  for ( i=1;argv[i];i++ ) 
  {
    if ((strcmp(argv[i],"--nosound")==0)||(strcmp(argv[i],"-s")==0)) {sound=0;} else
    if ((strcmp(argv[i],"--fullscreen")==0)||(strcmp(argv[i],"-f")==0)) {fullscreen=1;} else
    if ((strcmp(argv[i],"--windowed")==0)||(strcmp(argv[i],"-w")==0)) {fullscreen=0;} else
    if ((strcmp(argv[i],"--pageflip")==0)||(strcmp(argv[i],"-p")==0)) {pageflip=1;fullscreen=1;} else
    if ((strcmp(argv[i],"--notfinal")==0)||(strcmp(argv[i],"-n")==0)) {final=0;} else
    if ((strcmp(argv[i],"--noparticles")==0)||(strcmp(argv[i],"-t")==0)) {particle=0;} else
    if ((strcmp(argv[i],"--nofadeout")==0)||(strcmp(argv[i],"-o")==0)) {dofadeout=0;} else
    if ((strcmp(argv[i],"--precision")==0)&&(argv[i+1])) 
      {i++;precision=atoi(argv[i]);printf("Precision is set to %d\n",precision);} else
    if ((strcmp(argv[i],"--levelset")==0)&&(argv[i+1])) 
      {i++;levelset=atoi(argv[i]);printf("Levelset is set to %d\n",levelset);} else
    if ((strcmp(argv[i],"--help")==0)||(strcmp(argv[i],"-h")==0)) TextHelp(argv);
    else  {
	printf("Unknown parameter (-h for help): \"%s\" \n", argv[i]);
	TextHelp(argv);
    }
  }
}

void BlitMenu()  //blits menu for SelectRacer
{
  SDL_Rect fillrect;

  NullParticle = 0;

  for (i=0;i<=5;i++)
  {
    if (i==user[pl].racernum) {
      SDL_SetAlpha(racer_pic[i],(SDL_SRCALPHA),255);
    } else {
      SDL_SetAlpha(racer_pic[i],(SDL_SRCALPHA),75);
    }
  }
  fillrect.x=0;
  fillrect.y=160;
  fillrect.w=800;
  fillrect.h=440;
  SDL_FillRect(Screen,&fillrect,0);
  Blit(50,160,racer_pic[0]);
  Blit(300,160,racer_pic[1]);
  Blit(550,160,racer_pic[2]);
  Blit(50,370,racer_pic[3]);
  Blit(300,370,racer_pic[4]);
  Blit(550,370,racer_pic[5]);
  PutString(Screen, 100, 350, "Best steering");
  PutString(Screen, 330, 350, "15% extra time");
  PutString(Screen, 540, 350, "High velocity (ctrl key)");
  PutString(Screen,  40, 560, "Sticks to walls (ctrl key)");
}

void NextStage()
{
  Blit(0,0,back);
  XCenteredString(Screen, 100, "Congratulations!");
  XCenteredString(Screen, 130, "You have completed a stage!");
  XCenteredString(Screen, 200, "You gain 200 points for completing this stage");
  sprintf(text, "and %d points for the time that is left,", lifetime/3000);
  XCenteredString(Screen, 230, text);
  XCenteredString(Screen, 260, "but the game gets a bit harder.");
  sprintf(text, "You are in Stage %d now", ++Stage);
  XCenteredString(Screen, 360, text);
  sprintf(text, "Your current score is %d", Score+=200+lifetime/3000);
  XCenteredString(Screen, 390, text);
  levels_completed=0;
  lifetime=800000-(Stage-1)*70000;
  SDL_UpdateRect(Screen,0,0,0,0);
  SDL_EventState(SDL_KEYUP, SDL_ENABLE);
  SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
  SDL_WaitEvent(&event);
  SDL_WaitEvent(&event);
}

void nextlevel() //selects the next level at random
{
  int i;
  finished[levelnum]=1;
  quit=1;
  for (i=0;i<NUMBER_OF_LEVELS[levelset];i++) {
    if (finished[i]==0) {quit=0;}
  }
  if (!quit) {
    while (finished[levelnum]==1)
    {
      levelnum=abrand(0,NUMBER_OF_LEVELS[levelset]-1);
    }
    levels_completed++;
  } else {
    // reset all levels, if each level has been played one time
    ResetLevels();
    quit=0;
    levelnum=abrand(0,NUMBER_OF_LEVELS[levelset]-1);
  }
  if ((levels_completed==4)&&(mode!=1)) NextStage();
  sprintf(text,"BumpRace: Level #%d",levelnum);
  SDL_WM_SetCaption(text,"BumpRace");
  Score+=20;
}

void InitLevel()
{
  NullParticle = 0;   // No Particles
  endgame=0;
  game_speed=1;count_start=SDL_GetTicks();
  frame_count=0;now=SDL_GetTicks();time_since_start=0;cannon.turn=0;cannon.type=1;
  shotnum=0;last_lifetime=-1;startx=5;starty=5;
  cannon.shot_delay=11000-Stage*300-abrand(0,3000+Stage*1000); cannon.time_to_shot=1000;
  laser_switch=6000+abrand(-Stage*500,200);
  cannon.type=abrand(1,5);
  
  if (levelset == 0) {
  
  if (levelnum==12) {gravity=1;starty=560;} else {gravity=0;}
  if (levelnum==13) {starty=270;} 
  if (levelnum==14) {startx=370;starty=290;} 
  if (levelnum==15) {startx=750;starty=40;} 
  if (levelnum==16) {startx=370;starty=50;} 
  if (levelnum==17) {startx=370;starty=570;gravity=0.3;} 
 
  } else {
 
  if (levelnum==0) {startx=20;starty=570;gravity=1;}  else {gravity=0;}
  if (levelnum==1) {startx=20;starty=20;} 
  if (levelnum==2) {startx=20;starty=20;} 
  if (levelnum==3) {startx=750;starty=470;gravity=0.5;} 
  if (levelnum==4) {startx=5;starty=5;} 
  if (levelnum==5) {startx=10;starty=10;gravity=0.3;} 
  if (levelnum==6) {startx=10;starty=20;gravity=1.5;} 
  if (levelnum==7) {startx=20;starty=570;gravity=1.5;}
  if (levelnum==8) {startx=20;starty=570;gravity=0;}
	
  }
  	     
  for (pl=0;pl<playernum;pl++)
  {
    user[pl].realx=startx+10*pl;user[pl].realy=starty;user[pl].xspeed=0;user[pl].yspeed=0;
    user[pl].crashed=0;user[pl].completed=0;user[pl].teleported=0;
    HandleRacer();
  }
  for (i=0;i<=1;i++)              
  {
    Blit(0,0,back);
    blit_fore();
    LookForInfo();
    last_lifetime=0;
    blit_lifetime();
    SDL_BlitSurface(Screen, NULL, backbuffer, NULL);
    SDL_UpdateRect(Screen,0,0,0,0);
  } 
  SDL_EventState(SDL_KEYUP, SDL_ENABLE);                                  
  SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);                                  
//  printf(" successfully **\n");
}

void StartText()
{
  SDL_Rect rect;
  
  sprintf(text,"BumpRace Version %s (C) 2007 Karl Bartel", VERSION);
  PutString(Screen,170, 380, text);
  PutString(Screen,170, 402, "BumpRace comes with ABSOLUTELY NO WARRANTY");
  PutString(Screen,170, 424, "This is free software, and you are welcome to");
  PutString(Screen,170, 446, "redistribute it under certain conditions;");
  PutString(Screen,170, 468, "see COPYING for details.");
  SDL_UpdateRect(Screen,50,380,700,90);
  rect.x=50;
  rect.y=380;
  rect.w=700;
  rect.h=90;
  SDL_FillRect(Screen,&rect,0);
}

void ScoreText()
{
    Uint32 x;

    x=260;
    PutString(Screen, 400-SFont_TextWidth("---- COMPETITION SCORE ----")/2, 100, "---- COMPETITION SCORE ----");
    PutString(Screen, x, 200, "Player 1:");
    PutString(Screen, x, 215, "Player 2:");
    sprintf(text, "%d levels completed", user[0].points);
    PutString(Screen, x+280-SFont_TextWidth(text), 200, text);
    sprintf(text,"%d levels completed",user[1].points);
    PutString(Screen, x+280-SFont_TextWidth(text), 215, text);
    if (user[0].points>user[1].points) sprintf(text,"Player 1 has won!");
	else if (user[0].points<user[1].points) sprintf(text,"Player 2 has won!");
    	    else sprintf(text,"Draw Game!");
    PutString(Screen, 400-SFont_TextWidth(text)/2, 240, text);
}

void score()
{
  if (lifetime<0) lifetime=0;
  if (mode==1) {
    Blit(0,0,back);
    ScoreText();
    SDL_UpdateRect(Screen,0,0,0,0); 
    SDL_EventState(SDL_KEYUP, SDL_ENABLE);
    SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
    SDL_WaitEvent(&event);
    SDL_WaitEvent(&event);
  } else {
    Score+=lifetime/3000;
    FinalScore();
    ShowHiscore();
  }
}

#ifdef SOUND

void InitSound()
{
  if ( Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) < 0 ) 
  {
    fprintf(stderr,"Warning: Couldn't set 44100 Hz 16-bit audio\n- Reason: %s\n",SDL_GetError());
    fprintf(stderr,"\t**\nSOUND TURNED OFF\n\t**\n");
    sound=0;
  }
}

void LoadSound()
{
	if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {ComplainAndExit();}
	if (sound) {
		sprintf(text,"%s/sound/Flying_me_softly.ogg",DATAPATH);
		music = Mix_LoadMUS(text);
		if (music==NULL) {
			printf("COULD NOT LOAD MUSIC\n");
		}
		sprintf(text,"%s/sound/%s",DATAPATH,"explosion1.ogg");
		explode_sound = Mix_LoadWAV_RW(SDL_RWFromFile(text, "rb"), 1);
		sprintf(text,"%s/sound/%s",DATAPATH,"Picked_Coin_Echo_2.ogg");
		winlevel_sound = Mix_LoadWAV_RW(SDL_RWFromFile(text, "rb"), 1);
		sprintf(text,"%s/sound/%s",DATAPATH,"Menu_Select_01.ogg");
		teleport_sound = Mix_LoadWAV_RW(SDL_RWFromFile(text, "rb"), 1);
	}
}
#endif

int main(int argc, char *argv[])
{
//intialisation
  ReadCommandLine(argv);
#ifdef SOUND
  if (sound) {InitSound();}
#endif
  init_SDL();
  SDL_ShowCursor(0);
  SDL_WM_SetCaption("BumpRace","BumpRace");
  printf("** Video mode set **\n");
  srand( (unsigned) time(NULL) );    
  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);                                  
  fullscreen=0;  //activate update_rects
  SDL_EnableUNICODE(1);
  user[0].up=SDLK_UP;user[0].down=SDLK_DOWN;user[0].left=SDLK_LEFT;user[0].right=SDLK_RIGHT;user[0].extra=SDLK_RCTRL;
  user[1].up=SDLK_w;user[1].down=SDLK_s;user[1].left=SDLK_a;user[1].right=SDLK_d;user[1].extra=SDLK_LCTRL;
  ReadOptions();
  ReadOptions();
//show title screen
  title_pic=LoadImage("title.gif",0);
  Blit(0,0,title_pic);
  SDL_UpdateRect(Screen,0,0,0,0);
  sprintf(text,"%s/font.scl",DATAPATH);
  Font=LoadImage("font.png",3);
  InitFont(Font);
  if (final) StartText(i);
//load data
  printf("** Loading Data **\n");
#ifdef SOUND
  if (sound) {
	  LoadSound();
	  if (!Mix_PlayingMusic()) {Mix_PlayMusic(music,1);}
  }
#endif
  load_images();
  printf("** Main data loaded **\n");
  if (final) {
    SDL_Delay(3000);
    SDL_UpdateRect(Screen,50,380,700,90);
  }
  // select game mode
  Menu();
  while (mode!=3)
  {
    ResetLevels();
    levelnum=abrand(0,NUMBER_OF_LEVELS[levelset]-1);
    sprintf(text,"BumpRace: Level #%d",levelnum);
    SDL_WM_SetCaption(text,"BumpRace");
    for (pl=0;pl<playernum;pl++) {user[pl].racernum=0;user[pl].points=0;}
    lifetime=800000;Score=0;levels_completed=0;Stage=1;
    for (i=0;i<=NUMBER_OF_LEVELS[levelset];i++)
	finished[i]=0;
    // select racer
    if (final) 
    {
      clear_screen();
      for (pl=0;pl<playernum;pl++) while (1)
      {
        BlitMenu();
        Blit(100,0,selectp_pic[pl]);
        SDL_UpdateRect(Screen,0,0,0,0);
        SDL_PollEvent(&event);
        SelectRacer();
        if (user[pl].racernum==4) help(); 
	else if (user[pl].racernum==5) {ShowHiscore();SDL_WaitEvent(&event);}
        else break;
      }
    }
    printf("** Racer selected **\n");
    for (pl=0;pl<playernum;pl++) load_racer();  
    printf("** Racer data loaded **\n");
    // set racer abilities
    time_bonus=user[0].extra_time+user[1].extra_time;
    if (mode!=0) time_bonus-=20;
    lifetime+=lifetime*time_bonus/100;
    // start level
    while (!quit)
    {
      InitLevel();
      checks_common();
      // game loop
      while (!endgame)
      {
        HandleAndDraw_RacerParticlesAndShots();
        pl=0;
        if (playernum==1) checks_1p();
        if (playernum==2) checks_2p();
        checks_common();
        timing();
        blit_lifetime();
		Update();
      }
      //end of game loop
      if ((user[0].completed)||(user[playernum-1].completed))
      {
        nextlevel();
      }     
    }
    if (final) {
      score();
    }  
    quit=0;
    PrepareMenu();
    SDL_EventState(SDL_KEYUP, SDL_ENABLE);                                  
    SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);                                  
    Menu();
  }
//user wants to quit
#ifdef SOUND
  Mix_FadeOutMusic(1000);
#endif
  free_memory();
  printf("Awaiting SDL_Quit()\n");
  SDL_Quit();
  printf("SDL_Quit() finished.\n");
  
  return 0;
}
