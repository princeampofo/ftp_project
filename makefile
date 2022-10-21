output : server/ftp_server client/ftp_client

ftp_server: server/ftp_server.c parse.o
	gcc ftp_server.c parse.o -o ftp_server

ftp_client : client/ftp_client.c parse.o
	gcc ftp_client.c parse.o -o ftp_client

parse.o: parse.c parse.h
	gcc -c parse.c 

clean:
	rm server/ftp_server client/ftp_client