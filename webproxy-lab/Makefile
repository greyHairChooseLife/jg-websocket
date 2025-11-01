# Makefile for Proxy Lab
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your sp./Echo/ecific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c -o csapp.o

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o
	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
#########################################
echo: ./Echo/echo_client.o ./Echo/echo_server.o csapp.o
	$(CC) $(CFLAGS) ./Echo/echo_client.o  csapp.o -o ./Echo/echo_client $(LDFLAGS)
	$(CC) $(CFLAGS) ./Echo/echo_server.o  csapp.o -o ./Echo/echo_server $(LDFLAGS)

./Echo/echo_client.o: ./Echo/echo_client.c
	$(CC) $(CFLAGS) -c ./Echo/echo_client.c -o ./Echo/echo_client.o

./Echo/echo_server.o: ./Echo/echo_server.c
	$(CC) $(CFLAGS) -c ./Echo/echo_server.c -o ./Echo/echo_server.o

