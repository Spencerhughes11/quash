
quash: quash.c
		gcc quash.c -o quash

test: quash
	./quash

clean:
	rm -rf quash
	rm *.txt

val: quash
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./quash