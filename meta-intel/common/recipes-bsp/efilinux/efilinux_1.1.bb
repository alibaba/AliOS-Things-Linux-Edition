DESCRIPTION = "A UEFI OS loader"
LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://efilinux.h;beginline=5;endline=27;md5=f8d56e644672ac63fd81b55c205283ad"

DEPENDS = "gnu-efi"

inherit deploy

SRCREV = "a995826f9e43f1134baea61610eafd8c173bb776"
PV = "1.1+git${SRCPV}"

SRC_URI = "git://git.kernel.org/pub/scm/boot/efilinux/efilinux.git \
           file://0001-Disable-address-of-packed-member-warning.patch \
           file://0002-initialize-char-pointers.patch \
           "

S = "${WORKDIR}/git"

COMPATIBLE_HOST = '(x86_64|i.86).*-(linux|freebsd.*)'

EXTRA_OEMAKE = "INCDIR=${STAGING_INCDIR} LIBDIR=${STAGING_LIBDIR}"

# syslinux uses $LD for linking, strip `-Wl,' so it can work
export LDFLAGS = "`echo $LDFLAGS | sed 's/-Wl,//g'`"

do_deploy () {
        install ${S}/efilinux.efi ${DEPLOYDIR}/efilinux.efi
}
addtask deploy before do_build after do_compile

python () {
    ccargs = d.getVar('TUNE_CCARGS').split()
    if '-mx32' in ccargs:
        # use x86_64 EFI ABI
        ccargs.remove('-mx32')
        ccargs.append('-m64')
        d.setVar('TUNE_CCARGS', ' '.join(ccargs))
}
