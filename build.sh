#!/bin/sh
cd $(dirname $0)
links='-lncurses -lstdc++ -pthread'
flags="-Wno-write-strings -fno-omit-frame-pointer"

if [ "$1" = "buildrun" ]; then
	if [ "$(echo $ANDROID_DATA)" != "" ]; then
		echo "on android"
		sed -i '/my_port 1901/a ip 192.168.50.103' serv_conf 
		gcc ./src/main.cpp $links $flags -D ANDROID -o out && ./out $2
	else
		gcc ./src/main.cpp $links $flags -o out && ./out $2
	fi
elif [ "$1" = "buildrunpipe" ]; then
	gcc ./src/main.cpp $links $flags -o out && ./out $2 2> ./errpipe
elif [ "$1" = "debrun" ]; then
	gcc ./src/main.cpp $links $flags -ggdb -O0 -o deb_out && gdb ./deb_out $2
elif [ "$1" = "build" ]; then
	gcc ./src/main.cpp $links $flags -o out $2
elif [ "$1" = "debbuild" ]; then
	gcc ./src/main.cpp $links $flags -ggdb -O0 -o deb_out $2
elif [ "$1" = "buildrunfast" ]; then
	gcc ./src/main.cpp $links $flags -O3 -o out && ./out $2
else 
	echo "unknown option"
	exit 1
fi
