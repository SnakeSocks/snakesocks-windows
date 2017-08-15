#!/bin/bash

#echo "******* fake_gcc> I'm called as $* at $(pwd), dollar0 is $0"
##cp -r . /c/bash_home/rescued_dir
#c:\\MinGW\\bin\\gcc.exe $* > /c/bash_home/out.log 2>&1
#res=$?
#echo "******* Done. "
##cp -r . /c/bash_home/rescued_dir_done
#exit $res


c:\\MinGW\\bin\\gcc.exe $*
exit $?