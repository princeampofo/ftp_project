output : ftp_server ftp_client

ftp_server: ftp_server.c parse.o
	gcc ftp_server.c parse.o -o ftp_server

ftp_client : ftp_client.c parse.o
	gcc ftp_client.c parse.o -o ftp_client

parse.o: parse.c parse.h
	gcc -c parse.c 

clean:
	rm ftp_server ftp_client