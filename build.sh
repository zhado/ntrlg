#!/bin/sh
cd $(dirname $0)
links='-lncurses'

if [ "$1" = "buildrun" ]; then
	gcc ./src/main.cpp $links -o out && ./out $2
elif [ "$1" = "debrun" ]; then
	gcc ./src/main.cpp $links -ggdb -O0 -o deb_out && cgdb -args ./deb_out $2
elif [ "$1" = "build" ]; then
	gcc ./src/main.cpp $links -o out $2
elif [ "$1" = "debbuild" ]; then
	gcc ./src/main.cpp $links -ggdb -O0 -o deb_out $2
fi
