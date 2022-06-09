# ntrlg
Terminal time tracker

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

# log editing mode

While in log mode, enter name and tags for current activity/log. Separate tags with commas. Spaces are not allowed. If available, use arrow keys
to select autocomplete result and press Enter.

# tag mode

press `g` in view mode to enter tag mode.

you will see a list of tags which are being tracked.

press `a` to add a new tag to tracked list.
While adding it is also possible to specify background color for a tag. For example, `walking(22)` will be green. 
Numbers for colors can be seen [here](https://www.ditig.com/256-colors-cheat-sheet).

`d` to drag tag up or down.

`Delete` to delete.

use `v` to switch to view mode


