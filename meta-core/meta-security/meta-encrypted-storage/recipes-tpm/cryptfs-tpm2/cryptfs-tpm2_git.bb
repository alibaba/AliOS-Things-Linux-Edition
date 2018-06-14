SUMMARY = "A tool used to create, persist, evict a passphrase \
for full-disk-encryption with TPM 2.0"
DESCRIPTION = "\
This project provides with an implementation for \
creating, persisting and evicting a passphrase with TPM 2.0. \
The passphrase and its associated primary key are automatically \
created by RNG engine in TPM. In order to avoid saving the \
context file, the created passphrase and primary key are always \
persistent in TPM. \
"
AUTHOR = "Jia Zhang"
HOMEPAGE = "https://github.com/WindRiver-OpenSourceLabs/cryptfs-tpm2"
SECTION = "security/tpm"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=89c8ce1346a3dfe75379e84f3ba9d641"

DEPENDS += "tpm2.0-tss tpm2-abrmd pkgconfig-native"

PV = "0.6.2+git${SRCPV}"

SRC_URI = "\
    git://github.com/jiazhang0/cryptfs-tpm2.git \
"
SRCREV = "4a3eb44436184597ce9fc207d6b7fa7ce0690c62"

S = "${WORKDIR}/git"

EXTRA_OEMAKE = "\
    sbindir="${sbindir}" \
    libdir="${libdir}" \
    includedir="${includedir}" \
    tpm2_tss_includedir="${STAGING_INCDIR}/sapi" \
    tpm2_tss_libdir="${STAGING_LIBDIR}" \
    tpm2_tabrmd_includedir="${STAGING_INCDIR}" \
    CC="${CC}" \
    PKG_CONFIG="${STAGING_BINDIR_NATIVE}/pkg-config" \
    EXTRA_CFLAGS="${CFLAGS}" \
    EXTRA_LDFLAGS="${LDFLAGS}" \
"

PARALLEL_MAKE = ""

do_install() {
    oe_runmake install DESTDIR="${D}"

    if [ "${@bb.utils.contains('DISTRO_FEATURES', 'luks', '1', '0', d)}" = "1" ]; then
        install -m 0500 "${S}/scripts/init.cryptfs" "${D}"
    fi
}

PACKAGES =+ "\
    ${PN}-initramfs \
"

FILES_${PN}-initramfs = "\
    /init.cryptfs \
"

# Install the minimal stuffs only, and don't care how the external
# environment is configured.

# For luks-setup.sh
# @bash: bash
# @coreutils: echo, printf, cat, rm
# @grep: grep
# @procps: pkill, pgrep
# @cryptsetup: cryptsetup
# @tpm2.0-tools: tpm2_*
# @tpm2-abrmd: optional
RDEPENDS_${PN} += "\
    libtss2 \
    libtctidevice \
    libtctisocket \
    bash \
    coreutils \
    grep \
    procps \
    cryptsetup \
    tpm2.0-tools \
"

# For init.cryptfs
# @bash: bash
# @coreutils: echo, printf, cat, sleep, mkdir, seq, rm, rmdir, mknod, cut
# @grep: grep
# @gawk: awk
# @sed: sed
# @kmod: depmod, modprobe
# @cryptsetup: cryptsetup
# @cryptfs-tpm2: cryptfs-tpm2
# @net-tools: ifconfig
# @util-linux: mount, umount, blkid
RDEPENDS_${PN}-initramfs += "\
    bash \
    coreutils \
    grep \
    gawk \
    sed \
    kmod \
    cryptsetup \
    cryptfs-tpm2 \
    net-tools \
    util-linux-mount \
    util-linux-umount \
    util-linux-blkid \
"

RRECOMMENDS_${PN}-initramfs += "\
    kernel-module-tpm-crb \
    kernel-module-tpm-tis \
"
