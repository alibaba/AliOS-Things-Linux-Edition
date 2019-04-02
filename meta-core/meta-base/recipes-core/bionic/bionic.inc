SUMMARY = "Bionic is Android's C library, math library, and dynamic linker."
HOMEPAGE = "https://android.googlesource.com/platform/bionic/"
SECTION = "libs"

# NOTICE: When using bionic as the standard C library, you should put meta-core/meta-base
# before meta-yp/meta in BBLAYERS to override bbclasses
# For now, bionic supports alios-tiny on qemux86-64, qemuarm64 and raspberrypi3-64
COMPATIBLE_MACHINE = "qemux86-64|qemuarm64|raspberrypi3-64"

ANDROID_MIRROR = "aosp.tuna.tsinghua.edu.cn"
CORE_REPO = "${ANDROID_MIRROR}/platform/system/core"
BIONIC_REPO = "${ANDROID_MIRROR}/platform/bionic"
JEMALLOC_REPO = "${ANDROID_MIRROR}/platform/external/jemalloc"
ZLIB_REPO = "${ANDROID_MIRROR}/platform/external/zlib"

# matches with nougat-release
SRCREV_core = "a325982045057d3011c1b3db247cd53300a14f7d"
SRCREV_bionic = "7cdb481dfe626524c93e4b6a1aae4621b1beedbc"
SRCREV_jemalloc = "5a21ffb0e3a9f7b6ad47dd16793fd0fca76e2c56"
SRCREV_zlib = "cec8538e6162907d587c5229b81fc6a025cc1236"

SRC_URI = " \
    git://${CORE_REPO};name=core;protocol=https;nobranch=1;destsuffix=git/system/core; \
    git://${BIONIC_REPO};name=bionic;protocol=https;nobranch=1;destsuffix=git/bionic; \
    git://${JEMALLOC_REPO};name=jemalloc;protocol=https;nobranch=1;destsuffix=git/external/jemalloc; \
    git://${ZLIB_REPO};name=zlib;protocol=https;nobranch=1;destsuffix=git/external/zlib; \
    file://0001-Fix-libbase-and-add-makefile.patch;patchdir=system/core \
    file://0001-Add-makefile-for-jemalloc.patch;patchdir=external/jemalloc \
    file://0001-Update-kernel-headers.patch;patchdir=bionic \
    file://0002-Essential-patch-to-fix-bionic.patch;patchdir=bionic \
    file://0003-Add-makefiles-for-bionic.patch;patchdir=bionic \
    file://Makefile;subdir=git \
"

LICENSE = "Apache-2.0 & Zlib & BSD"
LIC_FILES_CHKSUM = " \
    file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10 \
    file://${COMMON_LICENSE_DIR}/Zlib;md5=87f239f408daca8a157858e192597633 \
    file://${COMMON_LICENSE_DIR}/BSD;md5=3775480a712fc46a69647678acb234cb \
"

INHIBIT_DEFAULT_DEPS = "1"

# currently we only support gcc-6.3.0
EXTRA_OEMAKE += " \
    'CC=${CC}' 'CXX=${CXX}' 'LD=${LD}' 'AS=${AS}' 'AR=${AR}' 'STRIP=${STRIP}' \
    'OBJDUMP=${OBJDUMP}' 'DESTDIR=${D}' 'TARGET_PREFIX=${TARGET_PREFIX}' 'TARGET_ARCH=${TARGET_ARCH}' \
    'GCC_INCLUDE=${STAGING_DIR_NATIVE}${libdir_native}/${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS}.${STAGINGCC}/gcc/${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS}/6.3.0' \
    'GCC_LIB=${STAGING_DIR_TARGET}${libdir}/${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS}/6.3.0' \
    'SYSROOT=${WORKDIR}/recipe-sysroot' 'TARGET_NAME=${TARGET_ARCH}${TARGET_VENDOR}-${TARGET_OS}' \
"

S = "${WORKDIR}/git"
B = "${WORKDIR}/git"