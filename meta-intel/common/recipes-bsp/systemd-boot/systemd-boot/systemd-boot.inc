EXTRA_OEMAKE_append_intel-x86-common = ' EFI_LDFLAGS="-L${STAGING_DIR_HOST}/usr/lib" '

SRC_URI_append_intel-x86-common = " \
            file://0001-efi-boot.c-workaround-for-Joule-BIOS-hang.patch \
            file://0001-sd-boot-stub-check-LoadOptions-contains-data.patch \
            "

