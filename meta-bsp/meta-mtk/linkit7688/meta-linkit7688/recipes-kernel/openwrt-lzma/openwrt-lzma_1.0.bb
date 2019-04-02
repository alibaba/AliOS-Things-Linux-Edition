DESCRIPTION = "Openwrt tool for pathcing an image with DTB file."
SECTION = "Openwrt tools."
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "http://downloads.openwrt.org/sources/lzma-4.65.tar.bz2"
SRC_URI[md5sum]="434e51a018b4c8ef377bf81520a53af0"

S = "${WORKDIR}/lzma-4.65/"

BBCLASSEXTEND="native nativesdk"

FILES_${PN}_class-native="${D}/${bindir}/*"

do_compile_class-native () {
    cd ${S}/C/LzmaUtil
    make -C ${S}/C/LzmaUtil -f makefile.gcc
    cd ${S}/CPP/7zip/Compress/LZMA_Alone
    make -C ${S}/CPP/7zip/Compress/LZMA_Alone -f makefile.gcc
}

do_install(){
    install -d ${D}/${bindir}/
    install -m 0755 ${S}/CPP/7zip/Compress/LZMA_Alone/lzma ${D}/${bindir}/
    mv ${D}/${bindir}/lzma ${D}/${bindir}/openwrt-lzma
}
