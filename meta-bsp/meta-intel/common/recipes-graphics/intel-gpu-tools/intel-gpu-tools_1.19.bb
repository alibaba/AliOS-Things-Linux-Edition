require ${COREBASE}/meta/recipes-graphics/xorg-app/xorg-app-common.inc

SUMMARY = "Intel GPU tools"
DESCRIPTION = "Variety of small tools for testing intel graphics."

LIC_FILES_CHKSUM = "file://COPYING;md5=e4b3dd344780e0400593b21b115a6947"

LICENSE_append = " & ISC"

inherit autotools gtk-doc

DEPENDS += "libdrm libpciaccess cairo udev glib-2.0 libxv libx11 libxext libxrandr procps"
RDEPENDS_${PN} += "bash"
RDEPENDS_${PN}-tests += "bash"

PACKAGE_BEFORE_PN = "${PN}-benchmarks ${PN}-tests"

SRC_URI[md5sum] = "4fdfa56acca3b046fc61fb12686656f3"
SRC_URI[sha256sum] = "a49c8ae7c0431c1f33cd9492f83ec4e06e98a48429178e4bd22d9a1a87e3b0f1"

PACKAGECONFIG ??= ""
PACKAGECONFIG[libunwind] = "--with-libunwind,--without-libunwind,libunwind,libunwind"

EXTRA_OECONF = "--disable-nouveau --disable-shader-debugger"
COMPATIBLE_HOST = "(x86_64.*|i.86.*)-linux"
COMPATIBLE_HOST_libc-musl_class-target = "null"

gputools_sysroot_preprocess() {
	rm -f ${SYSROOT_DESTDIR}${libdir}/pkgconfig/intel-gen4asm.pc
}
SYSROOT_PREPROCESS_FUNCS += "gputools_sysroot_preprocess"

FILES_${PN} += "${libdir}/intel_aubdump.so"
FILES_${PN}-benchmarks += "${libexecdir}/intel-gpu-tools/benchmarks"
FILES_${PN}-tests += "\
		${libexecdir}/intel-gpu-tools/*\
		${datadir}/intel-gpu-tools/1080p-right.png\
		${datadir}/intel-gpu-tools/1080p-left.png\
		${datadir}/intel-gpu-tools/pass.png\
		${datadir}/intel-gpu-tools/test-list.txt"
