DEPENDS_append_intel-x86-common = " rmc rmc-efi"

EXTRA_OEMAKE_append_intel-x86-common = ' EFI_LDFLAGS="-L${STAGING_DIR_HOST}/usr/lib" EFI_CFLAGS="-I${STAGING_INCDIR}/rmc -DRMC_EFI"'

SRC_URI_append_intel-x86-common = " \
            file://0001-sd-boot-Link-RMC-library-into-bootloader-and-stub.patch \
            file://0002-sd-boot-Load-board-specific-boot-entries-from-RMC-da.patch \
            file://0003-sd-boot-Support-global-kernel-command-line-fragment.patch \
            file://0004-sd-boot-Support-global-kernel-command-line-fragment-in-EFI-stub.patch \
            file://0001-efi-boot.c-workaround-for-Joule-BIOS-hang.patch \
            file://0001-sd-boot-stub-check-LoadOptions-contains-data.patch \
            "
