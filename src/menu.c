#include "bumprace.h"
#include <math.h>

void clear_screen()
{
  SDL_Rect fillrect;

  fillrect.x=0;
  fillrect.y=0;
  fillrect.w=800;
  fillrect.h=600;
  SDL_FillRect(Screen,&fillrect,0);
}

void clear_bb()
{
  SDL_Rect fillrect;

  fillrect.x=0;
  fillrect.y=0;
  fillrect.w=800;
  fillrect.h=600;
  SDL_FillRect(backbuffer,&fillrect,0);
}

//************************  HELP MENU  ***********************

void help()  //prints the help & credits screeen
{
  int i;
 
  clear_screen();
  for (i=0;i<=5;i++)
  {
    Blit(25,160+i*70,fore[i]);
  }
  Blit(300,-30,racer_pic[4]);
  PutString(Screen, 80, 175, "Normal wall");
  PutString(Screen, 80, 245, "Deadly wall");
  PutString(Screen, 80, 315, "Stopper");
  PutString(Screen, 80, 385, "Your goal is to arrive here");
  PutString(Screen, 80, 455, "Teleporter");  
  PutString(Screen, 80, 525, "Time bonus");  

  PutString(Screen, 350, 165, "Code:");
  PutString(Screen, 350, 180, "Karl Bartel (main code) <karl42@gmail.com>");
  PutString(Screen, 350, 195, "Lion Kimbro (particles) <SnowLion@sprynet.com>");
  PutString(Screen, 350, 220, "Graphics:");
  PutString(Screen, 350, 235, "Karl Bartel");
  PutString(Screen, 500, 220, "Background:");
  PutString(Screen, 500, 235, "Sweetie187");

  PutString(Screen, 350, 265, "Packaging:");
  PutString(Screen, 350, 280, "Christian T. Steigies (debian)"),

  PutString(Screen, 350, 305, "Music:");
  PutString(Screen, 350, 320, "Alexandr Zhelanov");
  PutString(Screen, 350, 335, "(soundcloud.com/alexandr-zhelanov)");

  PutString(Screen, 350, 365, "Sound Effects:");
  PutString(Screen, 350, 380, "Retimer, NenadSimic, Little Robot Sound Factory");  
  PutString(Screen, 350, 405, "Levels:");  
  PutString(Screen, 350, 420, "Karl Bartel");  
  PutString(Screen, 350, 435, "Stephan <emailme@enthralling.com>");  

  PutString(Screen, 350, 465, "Libraries used:");  
  PutString(Screen, 350, 480, "SDL by Sam Lantinga");  
  PutString(Screen, 350, 495, "SFont by Karl Bartel");  
  PutString(Screen, 350, 510, "modified MikMod (for SDL)");  

  PutString(Screen, 40, 580, "You can find the BumpRace web page at: http://www.linux-games.com");
  SDL_UpdateRect(Screen,0,0,0,0);
  SDL_WaitEvent(&event);
  SDL_WaitEvent(&event);
  clear_screen();
  BlitMenu();
  Blit(100,0,selectp_pic[pl]);
  SDL_UpdateRect(Screen,0,0,0,0);
}

// ********************************  Main Menu  *****************

void PrepareMenu()
{
  SDL_EventState(SDL_KEYUP, SDL_ENABLE);
  SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
  clear_bb();
  BlitToBB(0,0,title_pic);
  FadeScreen(7);
}

SDL_Rect DrawMode(int num, int y, SDL_Rect *rect) //for Menu only
{
  if (y<600)
  {    
    if (num<3) Blit(50+num*250,y,mode_select_pic[num]);
    else Blit(300+(num-3)*250,y,mode_select_pic[num]);
  }else
  {
    blitrect.x=1;
    blitrect.y=1;
    blitrect.w=1;
    blitrect.h=1;
  }    
  return blitrect;
}

void FadeSelectorOut(int y, int mode)
{
  SDL_Rect dstrect;

  now=SDL_GetTicks();
  for (i=0;i<=255;i+=SDL_GetTicks()-now)
  {
    now=SDL_GetTicks();
    dstrect.x=40+mode*250;dstrect.y=y-10;dstrect.w=220;dstrect.h=10;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=40+mode*250;dstrect.y=y+100;dstrect.w=220;dstrect.h=10;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=40+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=250+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
    SDL_FillRect(Screen,&dstrect,0);
    SDL_SetAlpha(selector_pic,(SDL_SRCALPHA),255-i);
    Blit (40+250*mode,y-10,selector_pic);
    Update();
  }
  dstrect.x=40+mode*250;dstrect.y=y-10;dstrect.w=220;dstrect.h=10;
  SDL_FillRect(Screen,&dstrect,0);
  dstrect.x=40+mode*250;dstrect.y=y+100;dstrect.w=220;dstrect.h=10;
  SDL_FillRect(Screen,&dstrect,0);
  dstrect.x=40+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
  SDL_FillRect(Screen,&dstrect,0);
  dstrect.x=250+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
  SDL_FillRect(Screen,&dstrect,0);
  SDL_UpdateRect(Screen,40+mode*250,y-10,220,120);
}

void FadeSelectorIn(int y, int mode)
{
  SDL_Rect dstrect;

  now=SDL_GetTicks();
  if (mode==3) {y+=120;mode=1;}
  for (i=255;i>=0;i-=SDL_GetTicks()-now)
  {
    now=SDL_GetTicks();
    dstrect.x=40+mode*250;dstrect.y=y-10;dstrect.w=220;dstrect.h=10;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=40+mode*250;dstrect.y=y+100;dstrect.w=220;dstrect.h=10;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=40+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
    SDL_FillRect(Screen,&dstrect,0);
    dstrect.x=250+mode*250;dstrect.y=y;dstrect.w=10;dstrect.h=100;
    SDL_FillRect(Screen,&dstrect,0);
    SDL_SetAlpha(selector_pic,(SDL_SRCALPHA),255-i);
    Blit (40+250*mode,y-10,selector_pic);
    Update();
  }
}

void Menu()
{
  int y=599,update;
  float startTime;
  SDL_Rect oldrect[4];

//  blit(0,0,title_pic);
  for (i=0;i<4;i++)
  {
    oldrect[i].x=0;oldrect[i].y=0;oldrect[i].w=1;oldrect[i].h=1;
  }
  startTime = now = SDL_GetTicks();
  while (startTime + 1000 > now)
  {
    for (i=0;i<4;i++) {
      SDL_FillRect(Screen,&oldrect[i],0);
      AddThisRect(oldrect[i]);
    }
    oldrect[0]=DrawMode(0, y, oldrect);
    oldrect[1]=DrawMode(1, y, oldrect);
    oldrect[2]=DrawMode(2, y, oldrect);
    oldrect[3]=DrawMode(3, y+120, oldrect);
    now = SDL_GetTicks();
    y = 599 - (int)( sin((now - startTime) * M_PI / 2000) * 310);
    Update();
  }
  mode=0;update=1;
  SDL_PollEvent(&event);
  keys = SDL_GetKeyState(NULL);
  while ( keys[SDLK_RETURN]!=SDL_PRESSED)
  {
    if (update) FadeSelectorIn(y,mode);
    SDL_PollEvent(&event);
    keys = SDL_GetKeyState(NULL);
    update=0;
    if (( keys[SDLK_RIGHT] == SDL_PRESSED )&&(mode<2))
      {FadeSelectorOut(y,mode);mode++;update=1;}
    if (( keys[SDLK_LEFT] == SDL_PRESSED )&&(mode>0)&&(mode<3))
      {FadeSelectorOut(y,mode);mode--;update=1;}
    if (( keys[SDLK_DOWN] == SDL_PRESSED )&&(mode<3))
      {FadeSelectorOut(y,mode);mode=3;update=1;}
    if (( keys[SDLK_UP] == SDL_PRESSED )&&(mode==3))
      {FadeSelectorOut(y+120,1);mode=1;update=1;}
    SDL_WaitEvent(&event);
  }
  if (mode==0) playernum=1; else playernum=2;
}


// **************************  Racer Selection

void load_racer() //loads the racer data
{
  if (user[pl].racernum==0) {
    for (i=0;i<=17;i++)
    {
      sprintf(text,"racerc%d.gif",i+1);
      user[pl].racer[i]=LoadImage(text,1);
    }
  }else
  if (user[pl].racernum==1) {      
    for (i=0;i<=17;i++)
    {
      sprintf(text,"racerd%d.gif",i+1);
      user[pl].racer[i]=LoadImage(text,1);
    }
  }else
  if (user[pl].racernum==2) {      
    for (i=0;i<=17;i++)
    {
      sprintf(text,"racere%d.gif",i+1);
      user[pl].racer[i]=LoadImage(text,1);
    }
  }else
  {
    for (i=0;i<=17;i++)
    {
      sprintf(text,"racerf%d.gif",i+1);
      user[pl].racer[i]=LoadImage(text,1);
    }
  }  
  user[pl].slowdown=0;
  user[pl].turbo_possible=0;
  user[pl].sticky_possible=0;
  user[pl].extra_time=0;
  if (user[pl].racernum==0) {user[pl].slowdown=0.00012;}
  if (user[pl].racernum==1) {user[pl].extra_time=15;}
  if (user[pl].racernum==2) {user[pl].turbo_possible=1; user[pl].slowdown=0.00004;}
  if (user[pl].racernum==3) {user[pl].sticky_possible=1; user[pl].slowdown=0.00003;}
}

void SelectRacer()  //menu for racer selection
{  
  SDL_Rect fillrect;
  
  fillrect.x=50;
  fillrect.y=140;
  fillrect.w=700;
  fillrect.h=430;
  SDL_PollEvent(&event);
  keys = SDL_GetKeyState(NULL);
  while (keys[SDLK_RETURN]==SDL_PRESSED)  
  {
    SDL_PollEvent(&event);
    keys = SDL_GetKeyState(NULL);
  }
  while (keys[SDLK_RETURN]!=SDL_PRESSED)  
  {    
    SDL_PollEvent(&event);
    keys = SDL_GetKeyState(NULL);
    while (!SDL_WaitEvent(&event))
    {
      SDL_PollEvent(&event);
      keys = SDL_GetKeyState(NULL);
    }
    if (( keys[SDLK_RIGHT] == SDL_PRESSED )&&(user[pl].racernum<5))
      {user[pl].racernum++;}
    if (( keys[SDLK_LEFT] == SDL_PRESSED  )&&(user[pl].racernum>0))
      {user[pl].racernum--;}
    if (( keys[SDLK_DOWN] == SDL_PRESSED  )&&(user[pl].racernum<3))
      {user[pl].racernum+=3;}
    if (( keys[SDLK_UP] == SDL_PRESSED    )&&(user[pl].racernum>2))
      {user[pl].racernum-=3;}
      if (user[pl].racernum>5) user[pl].racernum=5;
    BlitMenu();
    SDL_UpdateRects(Screen,1,&fillrect);
  }
}
