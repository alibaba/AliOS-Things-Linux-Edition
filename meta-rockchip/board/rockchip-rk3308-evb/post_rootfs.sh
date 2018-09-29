#!/bin/sh -x

ROOTFS_DIR=$1

echo Installing to ${ROOTFS_DIR}...

[ -d fs-overlay ] && cp -rp fs-overlay/* ${ROOTFS_DIR}
