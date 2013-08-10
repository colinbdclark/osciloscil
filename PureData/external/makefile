NAME=osciloscil~
CSYM=osciloscil_tilde

current: pd_darwin

pd_darwin: $(NAME).pd_darwin

.SUFFIXES:  .pd_darwin

DARWINCFLAGS = -DPD -O2 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.pd_darwin:
	$(CC) -arch i386  $(DARWINCFLAGS) -o osciloscil~.o -c osciloscil~.c
	$(CC) -arch i386  -bundle -undefined suppress -flat_namespace \
	    -o osciloscil~.pd_darwin osciloscil~.o 
	rm -f osciloscil.o

clean:
	rm osciloscil~.pd_darwin