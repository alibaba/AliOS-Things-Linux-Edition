SUMMARY = "Update Intel CPU microcode"

DESCRIPTION = "iucode_tool is a program to manipulate Intel i686 and X86-64\
 processor microcode update collections, and to use the kernel facilities to\
 update the microcode on Intel system processors.  It can load microcode data\
 files in text and binary format, sort, list and filter the microcode updates\
 contained in these files, write selected microcode updates to a new file in\
 binary format, or upload them to the kernel. \
 It operates on microcode data downloaded directly from Intel:\
 http://feeds.downloadcenter.intel.com/rss/?p=2371\
"
HOMEPAGE = "https://gitlab.com/iucode-tool/"
BUGTRACKER = "https://bugs.debian.org/cgi-bin/pkgreport.cgi?ordering=normal;archive=0;src=iucode-tool;repeatmerged=0"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe \
                    file://iucode_tool.c;beginline=1;endline=15;md5=5d8e3639c3b6a80e7d5e0e073933da16"

DEPENDS_append_libc-musl = " argp-standalone"

SRC_URI = "https://gitlab.com/iucode-tool/releases/raw/master/iucode-tool_${PV}.tar.xz"
SRC_URI_append_libc-musl = " file://0001-Makefile.am-Add-arg-parse-library-for-MUSL-support.patch"

SRC_URI[md5sum] = "c6f131a0b69443f5498782a2335973fa"
SRC_URI[sha256sum] = "01f1c02ba6935e0ac8440fb594c2ef57ce4437fcbce539e3ef329f55a6fd71ab"

inherit autotools

BBCLASSEXTEND = "native"

COMPATIBLE_HOST = "(i.86|x86_64).*-linux"

UPSTREAM_CHECK_URI = "https://gitlab.com/iucode-tool/releases"
