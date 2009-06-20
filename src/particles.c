#include "bumprace.h"
#include "math.h"
#include "stdlib.h"

#define PARTICLE_LIFETIME 13
#define PARTICLES_OF_EXHAUST 300
typedef struct
{
  float Lifetime;     // Lifetime of the current Particle
  float X;          // X coordinate
  float Y;          // Y coordinate
  float dX;         // delta X for each Particle frame
  float dY;         // delta Y for each Particle frame
  int R, G, B;      // Red, Green, and Blue for Particle
} Particle;

Particle particles[ 300 ];

int NullParticle=0;   // The NullParticle is the # of the last legal particle.
                    // Cannot reach PARTICLES_OF_EXHAUST!

#define PARTICLE_COLORS 5
int ParticleColorsR[] = { 255, 255, 200, 200, 80 };
int ParticleColorsG[] = { 255, 190, 90, 20,  0};
int ParticleColorsB[] = { 0, 0 ,   0,   50,   255 };

void UndrawParticles()
{
  int i;
  for( i = 0; i < NullParticle; i++ )
  {
    Particle* ThisParticle = &particles[ i ];
      if( (int)ThisParticle->X >= 0 )
        if( (int)ThisParticle->X < 800 )
          if( (int)ThisParticle->Y >= 0 )
            if( (int)ThisParticle->Y < 600 )
	      PutBackPixel( Screen, (int)ThisParticle->X, (int)ThisParticle->Y );
  }
}

void HandleParticles()
{
  int i;

  for( i = 0; i < NullParticle; i++ )
  {
    Particle* ThisParticle = &particles[ i ];

    ThisParticle->Lifetime-=game_speed/(float)1000;
    if( ThisParticle->Lifetime <= 0 )
    {
      // Delete this particle
      if( i != NullParticle -1 )
      {
        // We aren't the end particle, so we have to preserve the end.
        Particle* OtherParticle = &particles[ NullParticle - 1 ];
        *ThisParticle = *OtherParticle; // Default: Memberwise Assignment
      }
      NullParticle--;
      i--; // Do this cell once more...
    }
    else
    {
      int MorphColor;

      // Move this particle
      ThisParticle->X += ThisParticle->dX*game_speed;
      ThisParticle->Y += ThisParticle->dY*game_speed;
      ThisParticle->dY+= gravity/(float)2000;
      // printf("%f\t%f\n",ThisParticle->dX*game_speed,ThisParticle->dY*game_speed);

      MorphColor = 0;
      switch( (int)ThisParticle->Lifetime )
      {
        case 25:
        case 20: MorphColor = 1; break;
	case 8:
        case 2: MorphColor = -1; break;
      }
      ThisParticle->R += MorphColor;
      ThisParticle->G += MorphColor;
      ThisParticle->B += MorphColor;
      // Strangely enough, it looks better *WITHOUT* the logical constraints!
      //	    if (ThisParticle->R>255) ThisParticle->R=255;
      //	    if (ThisParticle->R<0) ThisParticle->R=0;
      //	    if (ThisParticle->G>255) ThisParticle->G=255;
      //	    if (ThisParticle->G<0) ThisParticle->G=0;
      //	    if (ThisParticle->B>255) ThisParticle->B=255;
      //	    if (ThisParticle->B<0) ThisParticle->B=0;
    }
  }
}

void DisplayParticles()
{
  int i;

  lock();
  for( i = 0; i < NullParticle; i++ ){
    Particle* ThisParticle = &particles[ i ];
    int X, Y;
    X = ThisParticle->X;
    Y = ThisParticle->Y;
    if( ThisParticle->X >= 0 ) 
      if( ThisParticle->X < 800 ) 
        if( ThisParticle->Y >= 0 ) 
          if( ThisParticle->Y < 600 ){
            PutPixel( Screen, ThisParticle->X, ThisParticle->Y,
	    SDL_MapRGB(Screen->format,ThisParticle->R, ThisParticle->G, ThisParticle->B) );
	  }
  } 
  unlock();
}

void NewParticles(float turn, int X, int Y, float spread_percent)
{
      if(( NullParticle < PARTICLES_OF_EXHAUST )&&(particle))
	{
	  int ColorIndex;
	  int Percent; // Damn. C is really really finicky about declarations! Urusai!

	  Particle* ThisParticle = &particles[ NullParticle ];
	  ThisParticle->Lifetime = PARTICLE_LIFETIME + abrand( -5, 5 );
	  ThisParticle->X = X + 15;
	  ThisParticle->Y = Y + 15;
	  
	  ColorIndex = abrand( 0, PARTICLE_COLORS-1 );
	  ThisParticle->R = ParticleColorsR[ ColorIndex ];
	  ThisParticle->G = ParticleColorsG[ ColorIndex ];
	  ThisParticle->B = ParticleColorsB[ ColorIndex ];
	  // Introduce Orthogonal (y, -x) Component
	Percent = abrand( -spread_percent/2, spread_percent/2 );
	turn+=190;
	turn+=Percent;
	if (turn>360) turn-=360;
	ThisParticle->dX =  sin(turn*0.01745)*(precision);
	ThisParticle->dY = -cos(turn*0.01745)*(precision);

	// scale particle speed
	// (because I'm having such difficulty finding a good scaling value...)
	ThisParticle->dX /= (float)2000;
	ThisParticle->dY /= (float)2000;

	NullParticle++;
     
    }
}
