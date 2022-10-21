output : ftp_server ftp_client

ftp_server: ftp_server.c 
	gcc ftp_server.c -o ftp_server

ftp_client : ftp_client.c 
	gcc ftp_client.c -o ftp_client

clean:
	rm ftp_server ftp_client