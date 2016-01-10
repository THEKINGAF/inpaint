#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char PIXEL;

PIXEL** alloue_image(int nl, int nc);
PIXEL intensitemax(PIXEL** im, int nl, int nc);
PIXEL** lectureimage(char* fichier, int* pnl,int* pnc);
int ecritureimage(char* fichier, PIXEL** im, int nl,int nc);
