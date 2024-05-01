
BIN_FILES  =  servidor

HEADER_FILE = comm.h libserver.h

CC = gcc

CPPFLAGS = -Wall

LDFLAGS =  -L./

LDLIBS = -lpthread -lrt -lclaves

all:  libclaves.so  servidor
.PHONY : all

libclaves.so: comm.o libserver.o
	$(CC) -shared -fPIC $(CPPFLAGS) $^ -o libclaves.so


servidor: servidor.o comm.o libserver.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@


%.o: %.c $(HEADER_FILE)
	$(CC) $(CPPFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(BIN_FILES) *.o libclaves.so


.SUFFIXES:
.PHONY : clean
