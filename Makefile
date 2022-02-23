library : 
	gcc -I./godot-headers -I./libpd/libpd_wrapper -I./libpd/pure-data/src -c ./src/pdstream.c -o ./bin/pdstream.o
	gcc -I./libpd/libpd_wrapper -I./libpd/pure-data/src -c ./src/instance.c -o ./bin/instance.o
	gcc ./bin/pdstream.o ./bin/instance.o -o bin/pdstream.dll -shared -Wl,--export-all-symbols -static-libgcc -L./libpd/libs -lpd
test :
	gcc -I./godot-headers -I./libpd/libpd_wrapper -I./libpd/pure-data/src -I./wavfile -c ./src/pdtest_thread.c -o ./bin/pdtest_thread.o
	gcc -c ./wavfile/wavfile.c -o ./bin/wavfile.o
	gcc bin/pdtest_thread.o bin/wavfile.o -o ./pdtest_thread.exe -static-libgcc -L./libpd/libs -lpd

# -Wl,-Bstatic -lm -Wl,-Bstatic -lpthread