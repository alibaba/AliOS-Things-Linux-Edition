SUMMARY = "The utility to manipulate machines owner keys which managed in shim"

LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

DEPENDS += "openssl efivar"

PV = "0.3.0+git${SRCPV}"

SRC_URI = "\
    git://github.com/lcp/mokutil.git \
"
SRCREV = "e19adc575c1f9d8f08b7fbc594a0887ace63f83f"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OEMAKE += "\
    EFIVAR_LIBS='-L${STAGING_LIBDIR} -lefivar' \
    OPENSSL_LIBS='-L${STAGING_LIBDIR} -lssl -lcrypto' \
"

COMPATIBLE_HOST = '(i.86|x86_64|arm|aarch64).*-linux'

FILES_${PN} += "${datadir}/bash-completion/*"

RDEPENDS_${PN} += "openssl efivar"
