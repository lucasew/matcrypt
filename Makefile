CFLAGS=-Wall -g

compile: main.o
	$(CC) *.o -o matcrypt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o matcrypt
