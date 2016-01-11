#include "inpaint.h"

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
	if(trous[i][j])
	{
		return 1;
	}

	return 0;	
}

int bordsTrous(PIXEL** trous, PIXEL** bords, int nl, int nc)
{
	int i, j, dx, dy, n;
	n=0;
	
	printf("Calcul du calque des bords de trous...");
	for(i=0; i<nl; i++)
	{
		for(j=0; j<nc; j++)
		{
			// le pixel (i,j) n'est pas un bord de trou ...
			bords[i][j] = 0;
			
			// ... sauf si c'est un trou ...
			if(estTrou(trous, i, j))
			{
				// ... dont un voisin n'est pas un trou
				for(dx=-1; dx<=1; dx++)
				{
					for(dy=-1; dy<=1; dy++)
					{
						// on évite le cas (dx,dy)==(0,0)
						if((dx || dy)
						&&(i+dx<nl)&&(i+dx>=0)
						&&(j+dy<nc)&&(j+dy>=0)
						&&!estTrou(trous, i+dx, j+dy))
						{
							bords[i][j] = 255;
						}
					}
				}
			}
			
			if(bords[i][j]) n++;
		}
	}
	printf("fait\n");

	return n;
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
			if((it+i<nl)&&(it+i>=0)
			&&(jt+j<nc)&&(jt+j>=0)
			&&!estTrou(trous, it+i, jt+j)
			&&(is+i<nl)&&(is+i>=0)
			&&(js+j<nc)&&(js+j>=0)
			&&!estTrou(trous, is+i, js+j))
			{
				d+=pow(im[it+i][jt+j]-im[is+i][js+j],2);
				cv++;
			}
		}
	}

	if(!cv)
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
			champ[i][j].x=i;
			champ[i][j].y=j;

			if(estTrou(trous, i, j))
			{
				champ[i][j].d=DMAX;
			}
			
			else
			{
				champ[i][j].d=0;
			}
		}
	}
}

void trouvemeilleurMP(PPV** champ, PIXEL**  im, PIXEL** trous, int  is, int  js, int taillepatch, int nl, int nc)
{
	int dx, dy, n;
	PPV p;
	
	// étape 1 : H calculs de distances sur patch aléatoires -> on garde le patch le plus ressemblant
	srand(time(NULL));

	for(n=0; n<H; n++)
	{
		// tirage de coordonnées aléatoires
		p.x = rand()%nl;
		p.y = rand()%nc;

		p.d=distance(im, trous, is, js, p.x, p.y, taillepatch, nl, nc);

		if(champ[is][js].d >= p.d)
		{
			champ[is][js] = p;
		}
	}

	// étape 2 : on regarde les patchs voisins de ceux qui correspondent aux voisins du patch qui nous intéresse
	for(dx=-1; dx<=1; dx++)
	{
		for(dy=-1; dy<=1; dy++)
		{
			// on évite le cas (dx,dy)==(0,0)
			if((dx || dy)
			&&(is+dy<nl)&&(is+dy>=0) 
			&&(js+dx<nc)&&(js+dx>=0)
			&&((champ[is+dy][js+dx].x)-dy<nl)&&((champ[is+dy][js+dx].x)-dy>=0)
			&&((champ[is+dy][js+dx].y)-dx<nc)&&((champ[is+dy][js+dx].y)-dx>=0))
			{
				p.x = (champ[is+dy][js+dx].x)-dy;
				p.y = (champ[is+dy][js+dx].y)-dx;

				p.d = distance(im, trous, is, js, p.x, p.y, taillepatch, nl, nc);

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
				// si la "distance" n'est pas optimale (optimale équivaut à nulle) alors on essaye de trouver mieux
				if(champ[i][j].d)
				{
					trouvemeilleurMP(champ, im, trous, i,  j, taillepatch, nl, nc);
				}
			}
		}
	}
}

PIXEL** inpaint(PIXEL** im, PIXEL** trous, int taillepatch, int nbiter, int nl, int nc)
{
	int i, j, n=0;
	PPV** champ = alloue_champMP(nl, nc);
	PIXEL** result = alloue_image(nl, nc);
	PIXEL** bords = alloue_image(nl, nc);
	
	printf("Initialisation du tableau MP...");
	initialiseMP(champ, trous, nl, nc);
	printf("fait\n");

	// on remplit l'image résultat avec les pixels de l'originale sauf sur les trous
	printf("Copie des pixels corrects...");
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
	printf("fait\n");

	

	printf("Réparation...\n");
	while(bordsTrous(trous, bords, nl, nc))
	{
		n++;
		printf("passe n°%d...\n", n);

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
	
	printf("Image réparée !\n");
	return result;
}

int main(int argc, char** argv) 
{ 
	int nc, nl, nctr, nltr;
	PIXEL** im;
	PIXEL** trous;
	PIXEL** result;

	// Lecture des fichiers images, en niveau de gris, sur 8 bits
	printf("Lecture...");
	im = lectureimage(argv[1], &nl, &nc);
	trous = lectureimage(argv[2], &nltr, &nctr);
	printf("fait : im : (%d,%d), trous : (%d,%d)\n", nl, nc, nltr, nctr);

	result = inpaint(im, trous, 1, 100, nl, nc);
	// On sauve l’image dans un fichier nommé resultat.pgm 
	ecritureimage(argv[3], result, nl, nc);
}
