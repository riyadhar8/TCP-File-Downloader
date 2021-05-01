client:	client.c
		gcc -o client client.c -L/usr/lib -lssl -lcrypto

clean:
		rm -rf client