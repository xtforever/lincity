
## Here is how i do it in ver 1.13, using these versions:
##   autoconf (GNU Autoconf) 2.59
##   automake (GNU automake) 1.9.4
##   ltmain.sh (GNU libtool) 1.5.2

aclocal-1.15 -I m4
libtoolize --force
automake-1.15 --add-missing
autoconf
autoheader
rm -f config.cache
./configure --prefix=$HOME/lincity

exit

## This is how i did it in ver 1.12
aclocal
automake --add-missing --foreign Makefile
autoconf
autoheader
rm -f config.cache
./configure --prefix=$HOME/lincity
