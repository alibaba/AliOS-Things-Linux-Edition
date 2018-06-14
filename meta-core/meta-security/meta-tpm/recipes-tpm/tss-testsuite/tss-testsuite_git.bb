SUMMARY = "Testcases to exercise the TSS stack/TSS API"
DESCRIPTION = "\
These are the testcases that exercise the TSS stack. They can be run \
either through the the LTP framework or standalone.  The testcases \
have been tested against the 20040304 version of LTP. \
\
Please do not execute these testcases on a machine where you are actively \
using the TPM. \
"
HOMEPAGE = "${SOURCEFORGE_MIRROR}/projects/trousers"
SECTION = "security/tpm"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=751419260aa954499f7abaabaa882bbe"

DEPENDS = "trousers"

PV = "git${SRCPV}"

SRC_URI = "\
    git://git.code.sf.net/p/trousers/testsuite \
    file://fix-failure-of-.so-LD-with-cortexa8t-neon-wrswrap-linux.patch \
    file://testsuite-transport-init.patch \
    file://Tspi_TPM_LoadMaintenancePubKey01.patch \
    file://transport-Tspi_TPM_Delegate.patch \
    file://common_c_no_des.patch \
    file://Tspi_TPM_CreateIdentity_no_des.patch \
    file://Tspi_TPM_CreateIdentityWithCallbacks_no_des.patch \
"
SRCREV = "f8f5c8684fd18522f8b8a82c416b088e886a1b7d"

S = "${WORKDIR}/git"

EXTRA_OEMAKE = "-C tcg 'CC=${CC}'"

testsuite_SUBDIRS = "\
    cmk context data delegation hash highlevel init key nv \
    pcrcomposite policy tpm transport tspi \
"

CFLAGS += "-DOPENSSL_NO_DES"
LDFLAGS += "-L${STAGING_LIBDIR} -lcrypto -lpthread"

do_configure_prepend() {
    cp -f "${S}/tcg/Makefile" "${S}"
    cp -f "${S}/tcg/init/makefile" "${S}/tcg/init/Makefile"
    # remove test case about DES
    rm -rf "${S}/tcg/context/Tspi_Context_GetCapability13.c"
}

do_install() {
    install -d "${D}/opt/tss-testsuite/tcg"
    for i in ${testsuite_SUBDIRS}; do \
        echo "Installing ${i}"; \
        cp -rf "tcg/${i}" "${D}/opt/tss-testsuite/tcg/"; \
    done;
    install -m 0755 tsstests.sh "${D}/opt/tss-testsuite"
}
 
FILES_${PN} += "/opt/*"
FILES_${PN}-dbg += "\
    /opt/tss-testsuite/tcg/*/.debug \
    /opt/tss-testsuite/tcg/*/*/.debug \
"

RDEPENDS_${PN} += "tpm-tools openssl bash"
