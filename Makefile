CFLAGS=-Wall -g
LDFLAGS=-lm

compile: main.o
	$(CC) *.o -o matcrypt $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o matcrypt
