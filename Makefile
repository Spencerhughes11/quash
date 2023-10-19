
quash: quash.c
		gcc -g quash.c -lpthread -lm -o quash

test: quash
	./quash

clean:
	rm -rf quash