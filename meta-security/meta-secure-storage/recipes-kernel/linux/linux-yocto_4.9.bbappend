FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

SRC_URI_append += " \
                  file://ecryptfs.cfg \
                  "
