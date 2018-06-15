FILESEXTRAPATHS_prepend_intel-x86-common := "${THISDIR}/systemd-boot:"

# Pin systemd revision down for systemd-boot recipe.
# Patches could not be applied cleanly when systemd in OE is updated,
# though we don't expect a lot of changes could happen in bootloader.
# RMC is designed to support a large number of types of boards, so we
# should do explicit update with validation to prevent regression even
# resolving conflicts for a new tip could be done in a short time.

# Revision: systemd v232 in OE
SRCREV_intel-x86-common = "a1e2ef7ec912902d8142e7cb5830cbfb47dba86c"

include systemd-boot/${EFI_PROVIDER}.inc

PACKAGE_ARCH_intel-x86-common = "${INTEL_COMMON_PACKAGE_ARCH}"

do_compile_append_intel-x86-common() {
	oe_runmake linux${SYSTEMD_BOOT_EFI_ARCH}.efi.stub
}

do_deploy_append_intel-x86-common() {
	install ${B}/linux*.efi.stub ${DEPLOYDIR}
}
