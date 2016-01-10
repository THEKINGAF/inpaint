#include "image.h"

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

PIXEL** lectureimage(char* fichier, int* pnl, int* pnc)
{
	FILE* f;
	int max, i;
	char s[512];
	PIXEL** im;

	if ((f=fopen(fichier,"r")) == NULL) return (NULL);

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

	sscanf(s, "%d", &max);
	if (max > 255) return(NULL);

	im = alloue_image(*pnl, *pnc);

	for(i=0; i<*pnl; i++)
	{	
		//printf("%d", i);
		fread(im[i], sizeof(PIXEL), *pnc, f);
	}

	if (fclose(f) != 0 ) perror("Une erreur s’est produite à la fermeture");

	return im;
}

PIXEL intensitemax(PIXEL** im, int nl, int nc)
{
	int i,j;
	PIXEL max=im[0][0];

	for(i=0; i<nl; i++)
	{
		for(j=0; j<nc; j++)
		{
			if(im[i][j]>max) max=im[i][j];
			if(max==255) return max;
		}
	}
	
	return max;
}

int ecritureimage(char* fichier, PIXEL** im, int nl, int nc)
{
	FILE* f;
	int i;

	if ((f=fopen(fichier,"w")) == NULL) return (1);

	fprintf(f, "%s%d %d%s%d%s", "P5\n#\n", nc, nl, "\n#\n", intensitemax(im, nl, nc), "\n");

	for(i=0; i<nl; i++)
	{
		fwrite(im[i], sizeof(PIXEL), nc, f);
	}

	if (fclose(f) != 0 ) perror("Une erreur s’est produite à la fermeture");
}
