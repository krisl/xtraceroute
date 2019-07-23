#! /bin/sh
set -x

aclocal 
autoheader
#add --include-deps if you want to bootstrap with any other compiler than gcc
#automake --add-missing --copy --include-deps
automake --add-missing --force-missing --copy
autoconf
rm -f config.cache
