SUMMARY = "Tools for TPM2."
DESCRIPTION = "tpm2.0-tools"
SECTION = "security/tpm"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=91b7c548d73ea16537799e8060cea819"

DEPENDS += "tpm2.0-tss tpm2-abrmd openssl curl autoconf-archive pkgconfig"

PV = "3.0.3+git${SRCPV}"

SRC_URI = "\
    git://github.com/tpm2-software/tpm2-tools.git;branch=3.X \
    file://0001-tpm2-tools-use-dynamic-linkage-with-tpm2-abrmd.patch \
"
SRCREV = "6b4385f098bd5d39e1cfc6cd2b038b68c960413f"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "\
    --with-tcti-device \
    --without-tcti-socket \
    --with-tcti-tabrmd \
"

EXTRA_OEMAKE += "\
    CFLAGS="${CFLAGS} -Wno-implicit-fallthrough" \
    LIBS=-ldl \
"

do_configure_prepend() {
    # execute the bootstrap script
    currentdir="$(pwd)"
    cd "${S}"
    ACLOCAL="aclocal --system-acdir=${STAGING_DATADIR}/aclocal" \
        ./bootstrap
    cd "${currentdir}"
}

RDEPENDS_${PN} += "libtss2 libtctidevice"
