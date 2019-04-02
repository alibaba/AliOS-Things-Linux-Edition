#!/bin/sh

. /etc/default/rcS

# mount the encrypted directories
case "$1" in
  start)
  secstore-manager start
  ;;
  *)
  ;;
esac
