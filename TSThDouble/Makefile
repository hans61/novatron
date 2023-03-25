default: all

all: TST_Novatron_2h.gt1

TSTConOut.gt1:
	glcc -o $@ TSTConOut.c -map=32k
	
TST_Novatron_2h.gt1:
	glcc -o $@ TST_Novatron_2h.c -map=32k

clean: FORCE
	-rm *.gt1

FORCE:
