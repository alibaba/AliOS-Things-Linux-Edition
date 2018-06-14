SUMMARY = "Utilities for signing UEFI binaries for use with secure boot"

LICENSE = "GPLv3"

LIC_FILES_CHKSUM = "\
    file://LICENSE.GPLv3;md5=9eef91148a9b14ec7f9df333daebc746 \
    file://COPYING;md5=a7710ac18adec371b84a9594ed04fd20 \
"

DEPENDS += "binutils-native openssl-native gnu-efi-native util-linux-native"

PV = "0.6+git${SRCPV}"

SRC_URI = "\
    git://kernel.ubuntu.com/jk/sbsigntool \
    file://ccan.git.tar.bz2 \
    file://fix-mixed-implicit-and-normal-rules.patch;apply=0 \
    file://disable-man-page-creation.patch \
    file://Fix-for-multi-sign.patch \
    file://sbsign-add-x-option-to-avoid-overwrite-existing-sign.patch \
    file://image-fix-the-segment-fault-caused-by-the-uninitiali.patch \
    file://Fix-the-deprecated-ASN1_STRING_data-in-openssl-1.1.0.patch \
    file://Update-OpenSSL-API-usage-to-support-OpenSSL-1.1.patch \
"
SRCREV="951ee95a301674c046f55330cd7460e1314deff2"

S = "${WORKDIR}/git"

inherit autotools-brokensep pkgconfig native

def efi_arch(d):
    import re
    arch = d.getVar("TARGET_ARCH")
    if re.match("i[3456789]86", arch):
        return "ia32"
    return arch

# Avoids build breaks when using no-static-libs.inc
#DISABLE_STATIC_class-target = ""

#EXTRA_OECONF_remove_class-target += "\
#    --with-libtool-sysroot \
#"

EXTRA_OEMAKE += "\
    INCLUDES='-I${S}/lib/ccan.git' \
    EFI_CPPFLAGS='-I${STAGING_INCDIR}/efi \
                  -I${STAGING_INCDIR}/efi/${@efi_arch(d)}' \
"

do_configure() {
    cd "${S}"
    rm -rf "lib/ccan.git"
    git clone "${WORKDIR}/ccan.git" lib/ccan.git
    cd lib/ccan.git && \
        git apply "${WORKDIR}/fix-mixed-implicit-and-normal-rules.patch" && \
        cd -

    OLD_CC="${CC}"

    if [ ! -e lib/ccan ]; then
        export CC="${BUILD_CC}"
        lib/ccan.git/tools/create-ccan-tree \
            --build-type=automake lib/ccan \
                talloc read_write_all build_assert array_size endian || exit 1
    fi

    export CC="${OLD_CC}"
    ./autogen.sh --noconfigure
    oe_runconf
}
