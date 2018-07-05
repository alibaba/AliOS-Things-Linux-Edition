SUMMARY = "The tpm-tools package contains commands to allow the platform administrator the ability to manage and diagnose the platform's TPM."
DESCRIPTION = "\
The tpm-tools package contains commands to allow the platform administrator \
the ability to manage and diagnose the platform's TPM.  Additionally, the \
package contains commands to utilize some of the capabilities available \
in the TPM PKCS#11 interface implemented in the openCryptoki project. \
"
SECTION = "security/tpm"

LICENSE = "CPL-1.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=059e8cd6165cb4c31e351f2b69388fd9"

DEPENDS = "libtspi openssl10"
DEPENDS_class-native = "trousers-native"

PV = "1.3.9.1+git${SRCPV}"

SRC_URI = "\
    git://git.code.sf.net/p/trousers/tpm-tools \
    file://tpm-tools-extendpcr.patch \
    file://03-fix-bool-error-parseStringWithValues.patch;apply=0 \
"
SRCREV = "bdf9f1bc8f63cd6fc370c2deb58d03ac55079e84"

S = "${WORKDIR}/git"

inherit autotools-brokensep gettext

do_configure_prepend() {
    mkdir -p po
    mkdir -p m4
    cp -R po_/* po/
    touch po/Makefile.in.in
    touch m4/Makefile.am
}

do_install_append() {
    #install -m 0755 "src/tpm_mgmt/tpm_startup" "${D}${sbindir}/tpm_startup"
    #install -m 0744 "src/tpm_mgmt/tpm_reset" "${D}${sbindir}/tpm_reset"
    #install -m 0744 "../tpm_integrationtest" "${D}${bindir}/tpm_integrationtest"
}

BBCLASSEXTEND = "native"
