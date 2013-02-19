BIN=bin
CC=gcc
CFLAGS=
LDFLAGS=
EXECUTABLES=part1a_client part1a_server part1c_client part1c_server part3_sender part3_receiver
HOST=localhost

$(EXECUTABLES):
	$(CC) $@.c -o $(BIN)/$@

part1a: part1a_client part1a_server
	./$(BIN)/part1a_server 34567 &
	./$(BIN)/part1a_client $(HOST) 34567 large.jpg

part1c: part1c_client part1c_server
	./$(BIN)/part1c_server 34567 &
	./$(BIN)/part1c_client $(HOST) 34568 large.jpg

part3: part3_receiver part3_sender
	./$(BIN)/part3_receiver 34567 &
	./$(BIN)/part3_sender $(HOST) 34567
