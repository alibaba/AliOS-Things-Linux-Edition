SUMMARY = "A general purpose malloc(3) implementation that emphasizes fragmentation avoidance and scalable concurrency support."
HOMEPAGE = "https://github.com/jemalloc/jemalloc"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://COPYING;md5=5cff9c9987190d1c1ab7ba635c1d9f29"

PV="4.0.4"
SRC_URI = "https://github.com/jemalloc/jemalloc/releases/download/${PV}/jemalloc-${PV}.tar.bz2"
SRC_URI[md5sum] = "687c5cc53b9a7ab711ccd680351ff988"
SRC_URI[sha256sum] = "3fda8d8d7fcd041aa0bebbecd45c46b28873cf37bd36c56bf44961b36d0f42d0"

inherit autotools

FILES_${PN} = "/usr/lib/libjemalloc.so.2"

FILES_${PN}-dev = "/usr/lib/libjemalloc.a \
    /usr/lib/libjemalloc.so "

PROVIDES="${PACKAGES}"

do_configure (){
    oe_runconf
}

do_compile (){
    oe_runmake build_lib_shared
}

do_install() {
    oe_runmake DESTDIR=${D} install_lib_shared
    rm -fr ${D}/usr/src/debug/jemalloc
}

BBCLASSEXTEND = "lib_package"
