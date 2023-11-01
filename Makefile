
quash: quash.c
		gcc -g quash.c -lpthread -lm -o quash

program1: program1.c
		gcc -g program1.c -lpthread -lm -o program1
program2: program2.c
		gcc -g program2.c -lpthread -lm -o program2
program3: program3.c
		gcc -g program3.c -lpthread -lm -o program3


test: quash
	./quash

clean:
	rm -rf quash