SRC = ./src/pdstream.c \
	./src/instance.c
	
OUT = bin
	
INCLUDES = -I./godot-headers \
	-I./libpd/libpd_wrapper \
	-I./libpd/pure-data/src
	
LIBS = -L./libpd/libs -lpd

LIBPD_DIR = libpd
	
CFLAGS = $(INCLUDES)

ifeq ($(shell uname),Darwin)
	SOLIB_PREFIX = lib
	SOLIB_EXT = dylib
else
	SOLIB_PREFIX =
	SOLIB_EXT = dll
	LDFLAGS += -Wl,--export-all-symbols \
		-static-libgcc
endif

LDFLAGS += -shared\
	$(LIBS)
	
PDSTREAM = $(OUT)/$(SOLIB_PREFIX)pdstream.$(SOLIB_EXT)

LIBPD_FLAGS = MULTI=true
LIBPD = $(LIBPD_DIR)/libs/$(SOLIB_PREFIX)pd.$(SOLIB_EXT)

ifeq ($(OS),Windows_NT)
	LIBPD_FLAGS += ADDITIONAL_CFLAGS='-DPD_LONGINTTYPE="long long"'
endif

$(PDSTREAM): ${SRC:.c=.o} $(LIBPD)
	$(CC) -o $(PDSTREAM) $^ $(LDFLAGS)

clean:
	rm ${SRC:.c=.o}
	rm $(PDSTREAM)

$(LIBPD): Makefile.patch
ifeq ($(OS),Windows_NT)
	patch -u $(LIBPD_DIR)/Makefile $<
endif
	$(MAKE) -C $(LIBPD_DIR) $(LIBPD_FLAGS)
	cp $(LIBPD) $(OUT)/$(SOLIB_PREFIX)pd.$(SOLIB_EXT)


