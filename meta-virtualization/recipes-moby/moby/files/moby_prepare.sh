#!/bin/sh

list=$(egrep '^[^[:space:]]+[[:space:]]+0[[:space:]]' /proc/cgroups | awk '{print $1}')
for sub in $list
do
    mdir=/cgroup/$sub
    if [ -d $mdir ]; then
        if [ "`ls -A $mdir`" == "" ]; then
            mount -t cgroup -o rw,$sub $sub $mdir
        fi
    else
        mkdir -p $mdir > /dev/null 2>&1
        mount -t cgroup -o rw,$sub $sub $mdir
    fi
done
