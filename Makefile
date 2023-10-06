
quash: quash.c
		gcc -g quash.c -lpthread -lm -o quash

clean:
	rm quash