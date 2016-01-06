#$@     Le nom de la cible
#$<     Le nom de la première dépendance
#$^     La liste des dépendances
#$*     Le nom du fichier sans suffixe

DIRSDL=SDL

CFLAGS=-g -I$(DIRSDL)/include -I$(DIRSDL)/include/SDL
LDFLAGS=-L$(DIRSDL)/lib -lSDLmain -lSDL -lSDL_ttf -lSDL_image  -lSDL_sound -lSDL_phelma -lSDL_draw -lX11


SRC = inpaint.c
OBJS = $(SRC:.c=.o)


prog : $(OBJS)
	gcc -o $@ $^ $(LDFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)
        
clean :
	rm -f *.o test? *~

