#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL_phelma.h>

#define DMAX 3000
#define H 100

typedef unsigned char PIXEL;

struct ppv
{
	int x;
	int y;
	int d;
};

typedef struct ppv PPV;

PIXEL** alloue_image(int nl, int nc)
{
	int i;
	PIXEL** p= calloc(nl,sizeof(*p));

	if (p==NULL) 
	{
		return NULL;
	}
	else 
	{
		*p = calloc(nl*nc,sizeof(**p));

		if (*p ==NULL) 
		{ 
			free(p); 
			return NULL; 
		}

		else
		{
			for(i=1; i<nl; i++)
			{
				p[i]=p[i-1]+nc;
			}
		}
	}

	return p;
}

PPV** alloue_champMP(int nl, int nc)
{
	int i;
	PPV** p= calloc(nl,sizeof(*p));

	if (p==NULL) 
	{
		return NULL;
	}
	else 
	{
		*p = calloc(nl*nc,sizeof(**p));

		if (*p ==NULL) 
		{ 
			free(p); 
			return NULL; 
		}

		else
		{
			for(i=1; i<nl; i++)
			{
				p[i]=p[i-1]+nc;
			}
		}
	}

	return p;
}

int estTrou(PIXEL** trous, int i, int j)
{
	if(trous[i][j]==255)
	{
		return 1;
	}

	else
	{
		return 0;
	}	
}

int bordsTrous(PIXEL** trous, PIXEL** bords, int nl, int nc)
{
	int i, j, dx, dy;

	for(i=0; i<nl; i++)
	{
		for(j=0; j<nc; j++)
		{
			if(estTrou(trous, i, j))
			{
				bords[i][j] = 0;

				for(dx=-1; dx<=1; dx++)
				{
					for(dy=-1; dy<=1; dy++)
					{
						// on évite le cas (dx,dy)==(0,0)
						if((dx || dy)&&(i+dx<nl)&&(i+dx>0)&&(j+dy<nc)&&(j+dy>0)&&!estTrou(trous, i+dx, j+dy))
						{
							bords[i][j] = 255;
						}
					}
				}
			}
		}
	}
}

double distance(PIXEL** im, PIXEL** trous, int is, int js, int it, int jt, int taillepatch, int nl, int nc)
{
	int i,j,cv=0;
	double d=0;

	if(estTrou(trous, it, jt))
	{
		return DMAX;
	}

	for(i=-taillepatch; i<=taillepatch; i++)
	{
		for(j=-taillepatch; j<=taillepatch; j++)
		{
			if(!estTrou(trous, it+i, jt+j) && !estTrou(trous, is+i, js+j))
			{
				cv++;
				d+=pow((im[it+i][jt+j]-im[is+i][js+j]),2);
			}
		}
	}

	if(cv<2)
	{
		return DMAX;
	}

	d=sqrt(d/cv);

	return d;
}

void initialiseMP (PPV** champ, PIXEL** trous, int nl, int nc)
{
	int i,j;

	for(i=0; i<nl; i++)
	{
		for(j=0; j<nc; j++)
		{
			if(estTrou(trous, i, j))
			{
				champ[i][j].x=i;
				champ[i][j].y=j;
				champ[i][j].d=DMAX;
			}
			
			else
			{
				champ[i][j].x=i;
				champ[i][j].y=j;
				champ[i][j].d=0;
			}
		}
	}
}

void trouvemeilleurMP(PPV** champ, PIXEL**  im, PIXEL** trous, int  is, int  js, int taillepatch, int nl, int nc)
{
	int dx, dy, n;
	PPV p;
	
	// étape 1
	srand(time(NULL));

	for(n=0;n<H;n++)
	{
		// tirage de coordonnées aléatoires
		p.x= rand()%nl;
		p.y= rand()%nc;

		p.d=distance(im, trous, is, js, p.x, p.y, taillepatch, nl, nc);

		if(champ[is][js].d >= p.d)
		{
			champ[is][js] = p;
		}
	}

	// étape 2

	for(dx=-1; dx<=1; dx++)
	{
		for(dy=-1; dy<=1; dy++)
		{
			// on évite le cas (dx,dy)==(0,0)
			
			if(dx || dy)
			{
				p.x = champ[is+dx][js+dy].x-dx;
				p.y = champ[is+dx][js+dy].y-dy;

				p.d=distance(im, trous, is, js, p.x, p.y, taillepatch, nl, nc);

				if(champ[is][js].d >= p.d)
				{
					champ[is][js] = p;
				}	
			}
		}
	}	
}

void optimiseMP(PPV** champ, PIXEL** im, PIXEL** trous, int nbiter, int taillepatch, int nl, int nc)
{
	int i, j, n;

	for(n=0; n<nbiter; n++)
	{
		// on parcourt l'image
		for(i=0; i<nl; i++)
		{
			for(j=0; j<nc; j++)
			{
				// si la "distance" n'est pas optimale (nulle) alors on essaye de trouver mieux
				if(champ[i][j].d)
				{
					trouvemeilleurMP(champ, im, trous, i,  j, taillepatch, nl, nc);
				}
			}
		}
	}
}

unsigned char** inpaint(unsigned char** im, unsigned char** trous, int taillepatch, int nbiter, int nl, int nc)
{
	int i, j;
	PPV** champ = alloue_champMP(nl, nc);
	unsigned char** result = alloue_image(nl, nc);
	unsigned char** bords = alloue_image(nl, nc);

	initialiseMP(champ, trous, nl, nc);
	
	// on remplit l'image résultat avec les pixels de l'originale sauf sur les trous
	for(i=0; i<nl; i++)
	{
		for(j=0; j<nc; j++)
		{
			if(!estTrou(trous, i, j))
			{
				result[i][j] = im[i][j];
			}	
		}
	}

	while(bordsTrous(trous, bords, nl, nc))
	{
		optimiseMP(champ, im, trous, nbiter, taillepatch, nl, nc);

		// on remplace les pixels des bords de trous par la valeur de celui au centre de leur meilleur patch
		// on en profite pour mettre à jour l'image des trous
		for(i=0; i<nl; i++)
		{
			for(j=0; j<nc; j++)
			{
				if(bords[i][j])
				{
					result[i][j] = im[champ[i][j].x][champ[i][j].y];
					trous[i][j] = 0;
				}	
			}
		}
	}

	return result;
}

PIXEL** lectureimage(char* fic, int* pnl, int* pnc)
{
	FILE* f;
	int a, nb, i;
	char s[512];
	PIXEL** im;

	if ((f=fopen(fic,"r")) == NULL) return (NULL);

	fgets(s, 511,f);

	if (strcmp(s,"P5\n")) return(NULL);

	do
	{
		fgets(s, 511, f);
	} while (s[0] == '#');

	sscanf(s, "%d %d", pnc, pnl);

	do
	{
		fgets(s, 511, f);
	} while (s[0] == '#');

	sscanf(s, "%d", &a);
	if (a > 255) return(NULL);

	im = alloue_image(*pnl, *pnc);

	for(i=0; i<*pnl; i++)
	{
		fread(im[i], sizeof(PIXEL), *pnc, f);
	}

	if (fclose(f) != 0 ) perror("Une erreur s’est produite à la fermeture");

	return im;
}


int main(int a, char** b) 
{ 
	int* pnc;
	int* pnl;
	unsigned char** im = lectureimage("cow_img.pgm", pnl, pnc); // Le tableau image
	
	SDL_Surface* fenetre=NULL;
	/* Creation d'une fenetre, couleurs sur 32 bits */
	fenetre=newfenetregraphique(600,400);
	/* On place cette image dans la fenetre, en position 10 20
	Attention : la fenetre doit pouvoir contenir l’image */
	afficheim8SDL(fenetre,im,*pnc,*pnl,10,20);
	puts("Taper pour continuer"); getchar();

}
