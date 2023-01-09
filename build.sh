#!/usr/bin/env bash
set +x
make clean
make 
if [ -e bin/libpdstream.so ]; then
  cp bin/libpd.so bin/libpdstream.so /home/opyate/Documents/gdextension-puredata/games/gdnative-godot35-audiostreampd/addons/audiostreampd/lib/linux
fi
