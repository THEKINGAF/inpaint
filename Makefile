#$@     Le nom de la cible
#$<     Le nom de la première dépendance
#$^     La liste des dépendances
#$*     Le nom du fichier sans suffixe

CFLAGS= 
LDFLAGS= -lm


SRC = image.c inpaint.c
OBJS = $(SRC:.c=.o)


inpaint : $(OBJS)
	gcc -o $@ $^ $(LDFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

