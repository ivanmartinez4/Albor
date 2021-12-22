#### About trailing spaces

You can remove trailing spaces across your entire project by using:

`egrep -rl ' $' --include *.c --include *.h --include  *.s --include  *.inc --include  *.txt *  | xargs sed -i 's/\s\+$//g'`
