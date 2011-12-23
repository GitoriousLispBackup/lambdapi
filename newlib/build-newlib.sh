#!/bin/bash
# Oh how I hate bash.
if [ -L $0 ] ; then
    ME=$(readlink $0)
else
    ME=$0
fi
cd $(dirname $ME)

if [ ! -a newlib-1.19-0 ] 
  then
  if [ ! -f newlib-1.19.0.tar.gz ]
    then
    wget ftp://sources.redhat.com/pub/newlib/newlib-1.19.0.tar.gz
  fi
  tar -xzf newlib-1.19.0.tar.gz
fi

cd build
../newlib-1.19.0/configure --target=arm-none-eabi --disable-newlib-supplied-syscalls --prefix=`cd ../usr;pwd`
make && make install