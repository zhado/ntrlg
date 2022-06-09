# ntrlg
Terminal time tracker

# Dependencies

* gcc/clang 

* ncurses

Should run on any OS that supports ncurses and libc. tested on linux, android(termux), macOS.

# Building
Run:
```console
./build.sh build
```

# Usage

use arrow keys to move forwards and backwards in time. `Up`/`Down` move 'cursor' by zoom granularity, while `Left` and `Right` advance cursor by 24 hour increments.

`l` add a new log at current time

`a` add a new log which starts at last logs end and is still active.

`d` drag start/end/body of an entry

`z` zoom in

`x` zoom out

`D` delete an entry

`s` save

`q` quit

`v` switch to view mode

`w` switch to week mode.

`g` switch to tag editing mode.

# log editing mode

While in log mode, enter name and tags for current activity/log. Separate tags with commas. Spaces are not allowed. If available, use arrow keys
to select autocomplete result and press Enter.

# tag mode

you will see a list of tags which are being tracked.

press `a` to add a new tag to tracked list.
While adding it is also possible to specify background color for a tag. For example, `walking(22)` will be green. 
Numbers for colors can be seen [here](https://www.ditig.com/256-colors-cheat-sheet).

`d` to drag tag up or down.

`Delete` to delete.

# storage

all data is stored in a local file in extremely simple format and can be easily edited. This data file named "data"
needs to be placed in the same folder as the executable. This probably will be changed in future versions.

# todo
* clean up messy code.
* optimize week view.
* switch to meson
* add better graphs and visualization 
* add compression support for data file..

