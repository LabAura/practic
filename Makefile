libAura.a: Aura.o
		gcc -c Aura.c
		ar cr libAura.a Aura.o
		sudo cp libAura.a /usr/lib
		sudo cp Aura.h /usr/include		
