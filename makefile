output : server/ftp_server client/ftp_client

ftp_server: ftp_server.c 
	gcc server/ftp_server.c -o server/ftp_server

ftp_client : ftp_client.c 
	gcc client/ftp_client.c -o client/ftp_client

clean:
	rm server/ftp_server client/ftp_client