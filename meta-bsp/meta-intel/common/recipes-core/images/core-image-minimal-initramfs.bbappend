# Use initramfs-framework instead of initramfs-live*
PACKAGE_INSTALL_remove_intel-x86-common = "initramfs-live-boot initramfs-live-install initramfs-live-install-efi"
PACKAGE_INSTALL_append_intel-x86-common = " initramfs-framework-base initramfs-module-udev initramfs-module-setup-live initramfs-module-install-efi"

# Add i915 graphics firmware
PACKAGE_INSTALL_append_intel-x86-common = " linux-firmware-i915"

# Add gptfdisk package, set partuuid by sgdisk command in gptfdisk package,
# when install os to hard disk through iso live image.
PACKAGE_INSTALL_append_intel-x86-common = " gptfdisk"
