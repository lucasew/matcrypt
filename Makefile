CFLAGS=-Wall -g
LDFLAGS=-lm -O0

compile: main.o
	$(CC) *.o -o matcrypt $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o matcrypt
