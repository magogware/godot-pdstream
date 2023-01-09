SRC = ./src/pdstream.c \
	./src/instance.c
	
OUT = bin
	
INCLUDES = -I./godot-headers \
	-I./libpd/libpd_wrapper \
	-I./libpd/pure-data/src
	
LIBS = -L./libpd/libs -lpd

CFLAGS = $(INCLUDES)

ifeq ($(shell uname),Darwin)
	SOLIB_PREFIX = lib
	SOLIB_EXT = dylib
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    SOLIB_PREFIX =
    SOLIB_EXT = dll
    LDFLAGS += -Wl,--export-all-symbols \
      -static-libgcc
  else # assume Linux
    SOLIB_PREFIX = lib
    SOLIB_EXT = so
  endif
endif

LDFLAGS += -shared\
	$(LIBS)

ifeq ($(shell uname),Linux)
  CFLAGS += -fPIC -g
  LDFLAGS += '-Wl,-rpath,addons/audiostreampd/lib/linux'
endif

PDSTREAM = $(OUT)/$(SOLIB_PREFIX)pdstream.$(SOLIB_EXT)

LIBPD_DIR = libpd
LIBPD_FLAGS = MULTI=true
LIBPD = $(LIBPD_DIR)/libs/$(SOLIB_PREFIX)pd.$(SOLIB_EXT)
LIBPD_LOCAL = $(OUT)/$(SOLIB_PREFIX)pd.$(SOLIB_EXT)

ifeq ($(OS),Windows_NT)
	LIBPD_FLAGS += ADDITIONAL_CFLAGS='-DPD_LONGINTTYPE="long long"'
endif

$(PDSTREAM): ${SRC:.c=.o} $(LIBPD_LOCAL)
	$(CC) -o $@ $(CFLAGS) ${SRC:.c=.o} $(LDFLAGS)

$(LIBPD_LOCAL): Makefile.patch $(OUT)
ifeq ($(OS),Windows_NT)
	patch -u $(LIBPD_DIR)/Makefile $<
endif
	$(MAKE) -C $(LIBPD_DIR) $(LIBPD_FLAGS)
	cp $(LIBPD) $@

$(OUT):
	mkdir $@

clean:
	rm ${SRC:.c=.o}
	rm $(PDSTREAM)
