#include <math.h>
#include <time.h>
#include "image.h"

#define DMAX 1.7976931348623158e+308
#define H 300

struct ppv
{
	int x;
	int y;
	double d;
};

typedef struct ppv PPV;

PPV** alloue_champMP(int nl, int nc);
int estTrou(PIXEL** trous, int i, int j);
int bordsTrous(PIXEL** trous, PIXEL** bords, int nl, int nc);
double distance(PIXEL** im, PIXEL** trous, int is, int js, int it, int jt, int taillepatch, int nl, int nc);
void initialiseMP (PPV** champ, PIXEL** trous, int nl, int nc);
void trouvemeilleurMP(PPV** champ, PIXEL**  im, PIXEL** trous, int  is, int  js, int taillepatch, int nl, int nc);
void optimiseMP(PPV** champ, PIXEL** im, PIXEL** trous, int nbiter, int taillepatch, int nl, int nc);
PIXEL** inpaint(PIXEL** im, PIXEL** trous, int taillepatch, int nbiter, int nl, int nc);
