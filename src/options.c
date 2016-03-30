#include "bumprace.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

char Player[10][200];
int Hiscore[10],i;

FILE *OpenOptionsFile( char *mode )
{
    char filename[200];
    FILE *file;
    
    if ((char *)getenv("HOME") != NULL)
		snprintf( filename, sizeof(filename), "%s/.bumprace", getenv("HOME"));
    else
		sprintf( filename, "bumprace.dat");

    file = fopen(filename, mode);
    
    if (file==NULL) {
	fprintf(stderr, "\nWarning: I could not open the options file ");
	if (strcmp(mode, "r") == 0)
	    fprintf(stderr, "for read:");
	else if (strcmp(mode, "w") == 0)
	    fprintf(stderr, "for write:");

	fprintf(stderr, "\n%s\n"
	    "The error that occurred was:\n"
	    "%s\n\n", filename, strerror(errno));
    }
    
    return file;
}

void ReadOptions()
{
    FILE *file;
        
    file=OpenOptionsFile("r");
    
    if (file==NULL) {
	file=OpenOptionsFile("w");
	if (file==NULL) exit(1);
	
	fprintf(file, "# BumpRace Options File\n\n");
	fprintf(file, "Sam\nLion\nBill\nJohn\nAndreas\nAaron\nMichael\nPhoenix\nPierre\nRobert\n");
	for (i=0;i<10;i++)
	    fprintf(file, "%d\n", 4000-i*400);
	
	fclose(file);
	file=OpenOptionsFile("r");
    }

    fscanf(file, "%*s%*s%*s%*s\n");
    for (i=0;i<10;i++)
	fscanf(file, "%199[^\n]\n",Player[i]);
    for (i=0;i<10;i++)
	fscanf(file, "%d\n",&Hiscore[i]);
//    for (i=0;i<10;i++)
//	printf("%s %d\n",Player[i],Hiscore[i]);
    
    fclose(file);
}

void FinalScore()
{
    char Name[200]="",text[200];
	int a;
    FILE *file;

    if (Score>Hiscore[9]) {
    Blit(0,0,back);
    XCenteredString(Screen, 200, "< YOUR FINAL SCORE >");
    sprintf(text,"%d", Score);
    XCenteredString(Screen, 250, text);
    XCenteredString(Screen, 300, "Enter Your Name:");
    Update();
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SFont_Input(Screen, 350, 350, 200, Name);
    if (strcmp(Name,"")==0) strcpy(Name,"Anonymous Coward");
    for (i=0;i<10;i++) {
	if (Score>Hiscore[i]) {
	    // Move other players down on the list
	    for (a=9;a>=i;a--) {
		strcpy(Player[a],Player[a-1]);
		Hiscore[a]=Hiscore[a-1];		
	    }
	    // add the player
	    strcpy(Player[i],Name);
	    Hiscore[i]=Score;
	    break;
	}
    }
//    for (i=0;i<10;i++)
//	printf("%s %d\n",Player[i],Hiscore[i]);
    
    // write everything into the options file	    
    file=OpenOptionsFile("w");
    if (file==NULL) exit(1);
    
    fprintf(file, "# BumpRace Options File\n\n");
    for (i=0;i<10;i++)
	fprintf(file, "%s\n", Player[i]);
    for (i=0;i<10;i++)
	fprintf(file, "%d\n", Hiscore[i]);

    fclose(file);

} else {

    if (Score<0) Score=0;
    Blit(0,0,back);
    XCenteredString(Screen, 200, "< YOUR FINAL SCORE >");
    sprintf(text,"%d", Score);
    XCenteredString(Screen, 250, text);
    XCenteredString(Screen, 300, "You didn't qualify for the highscore list...");
    Update();
    SDL_EventState(SDL_KEYUP, SDL_ENABLE);
    SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);    
    SDL_WaitEvent(&event);
    SDL_WaitEvent(&event);
}}

void ShowHiscore()
{
    SDL_Event WaitEvent;
    char text[200];
    int i;
    
    Blit(0,0,back);
    XCenteredString(Screen, 100, "< BUMPRACE HIGHSCORES >");
    for (i=0;i<10;i++) {
	sprintf(text,"%s", Player[i]);
	PutString(Screen,200,150+i*30,text);
	sprintf(text,"%d", Hiscore[i]);
	PutString(Screen,600-SFont_TextWidth(text),150+i*30,text);
    }
    Update();
    while ((WaitEvent.type!=SDL_KEYDOWN)&&(WaitEvent.type!=SDL_MOUSEBUTTONDOWN)) {
	SDL_PollEvent(&WaitEvent);
	SDL_WaitEvent(&WaitEvent);
    }
}
