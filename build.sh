#!/bin/bash
cd $(dirname $0)
links='-lncurses -lstdc++ -pthread -lm'
flags="-Wall"
COMPILTER="clang"
OUT="ntrlg"

if [ "$1" = "buildrun" ]; then
	if [[ "$OSTYPE" == "linux-gnu"* ]]; then
		$COMPILTER ./src/main.c $links $flags -o $OUT && ./$OUT $2
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		$COMPILTER ./src/main.c $links $flags -D MAC_OS -o $OUT && ./$OUT $2
	elif [[ "$OSTYPE" == "linux-android" ]]; then
		echo "on android"
		sed -i '/my_port 1901/a ip 192.168.50.103' serv_conf 
		$COMPILTER ./src/main.c $links $flags -D ANDROID -o $OUT && ./$OUT $2
	else
		echo "OS not supported"
	fi

elif [ "$1" = "buildrunpipe" ]; then
	$COMPILTER ./src/main.c $links $flags -o $OUT && ./out $2 2> ./errpipe
elif [ "$1" = "debrun" ]; then
	$COMPILTER ./src/main.c $links $flags -ggdb3 -O0 -o deb_out && gdb ./deb_out $2
elif [ "$1" = "build" ]; then
	$COMPILTER ./src/main.c $links $flags -o $OUT $2
elif [ "$1" = "debbuild" ]; then
	$COMPILTER ./src/main.c $links $flags -ggdb3 -O0 -o deb_out $2
elif [ "$1" = "buildrunfast" ]; then
	if [ "$(echo $ANDROID_DATA)" != "" ]; then
		echo "on android"
		sed -i '/my_port 1901/a ip 192.168.50.103' serv_conf 
		$COMPILTER ./src/main.c $links $flags -O3 -D ANDROID -o $OUT && ./out $2
	else
		$COMPILTER ./src/main.c $links $flags -O3 -o $OUT && ./out $2
	fi
elif [ "$1" = "buildrunfastdeb" ]; then
	if [ "$(echo $ANDROID_DATA)" != "" ]; then
		echo "on android"
		sed -i '/my_port 1901/a ip 192.168.50.103' serv_conf 
		$COMPILTER ./src/main.c $links $flags -O3 -ggdb3 -D ANDROID -o deb_out && gdb ./deb_out $2
	else
		$COMPILTER ./src/main.c $links $flags -O3 -ggdb3 -o deb_out && gdb ./deb_out $2
	fi
else 
	echo "unknown option"
	exit 1
fi
