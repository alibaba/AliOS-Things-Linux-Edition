# Note the initrd command becomes not working if the command for
# loading image is changed to the chainloader command instead of
# the linux command.

menuentry "Sample EFI boot" --unrestricted {
    savedefault
    set fallback=1
    linux /bzImage root=/dev/sda2 ro rootwait
    initrd /initrd
}

menuentry "Sample EFI boot (Recovery)" --unrestricted {
    linux /bzImage_bakup root=/dev/sda2 ro rootwait
    initrd /initrd_bakup
}
