CC=gcc
OBJ=forza_server.o
OBJ1=forza_client.o
CFLAGS=-Wall
BIN=/bin

all: compile
	@echo "\n"
	@echo	 "$	make install (per installare)"
	@echo	 "$	make clear (per pulire i file temporanei)"
compile: $(OBJ) $(OBJ1)
	$(CC) $(CFLAGS) -o forza_server $(OBJ)
	$(CC) $(CFLAGS) -o forza_client $(OBJ1)
	@echo File $^ creati
install:
	@echo \install..\N
	@sudo cp forza_server $(BIN)
	@sudo cp forza_client $(BIN) && echo Install OK

clean:
	@echo clean..
	@rm -f forza_server.o forza_server forza_client.o forza_client && echo "Clean OK"
