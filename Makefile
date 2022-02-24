SRC = ./src/pdstream.c \
	./src/instance.c
	
INCLUDES = -I./godot-headers \
	-I./libpd/libpd_wrapper \
	-I./libpd/pure-data/src
	
LIBS = -L./libpd/libs -lpd
	
CFLAGS = $(INCLUDES)

ifeq ($(shell uname),Darwin)
	SOLIB_EXT = dylib
else
	SOLIB_EXT = dll
	LDFLAGS += -Wl,--export-all-symbols \
		-static-libgcc
endif
	
LDFLAGS += -shared\
	$(LIBS)

ifeq ($(OS),Windows_NT)
	PDSTREAM = bin/pdstream.$(SOLIB_EXT)
else
	PDSTREAM = bin/libpdstream.$(SOLIB_EXT)
endif

$(PDSTREAM): ${SRC:.c=.o}
	$(CC) -o $(PDSTREAM) $^ $(LDFLAGS)
	
clean:
	rm ${SRC:.c=.o}
	rm $(PDSTREAM)

# -Wl,-Bstatic -lm -Wl,-Bstatic -lpthread

