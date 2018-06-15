require recipes-graphics/xorg-driver/xorg-driver-video.inc

SUMMARY = "X.Org X server -- ASpeed Technologies graphics driver"

DESCRIPTION = "ast is an Xorg driver for ASpeed Technologies video cards"

LIC_FILES_CHKSUM = "file://COPYING;md5=0b8c242f0218eea5caa949b7910a774b"

DEPENDS += "libpciaccess"

SRC_URI[md5sum] = "4f85febe48d51e53624550a96fc9e9ee"
SRC_URI[sha256sum] = "1edbbc55d47d3fd71dec99b15c2483e22738c642623a0fb86ef4a81a9067a2de"
