get_efivar -f uint8 -s secured SecureBoot

if [ "${secured}" = "1" ]; then
    if [ -s "${prefix}/password.inc" ]; then
        source "${prefix}/password.inc"
    fi
fi

get_efivar -f uint8 -s unprovisioned SetupMode

if [ "${unprovisioned}" = "1" ]; then
    # Create a boot entry for Automatic Certificate Provision.
    # This is especially useful for certain hardware, e.g,
    # Intel NUC5i3MYHE, doedn't support to display a customized
    # BIOS boot option used to launch LockDown.efi.

    if [ ! "${provision_failed}" ]; then
        # Secure boot was disabled in BIOS setup. Overwrite the
        # behavior of normal boot.
        set timeout=0
        set default="Automatic Certificate Provision"
    elif [ "${provision_failed}" = "0" ]; then
        # The auto provision was reset in BIOS setup.
        set default="Automatic Certificate Provision"
    fi

    # This menu will be hidden as long as the provision succeeds.
    menuentry "Automatic Certificate Provision" --unrestricted {
        set provision_failed="0"
        save_env provision_failed

        chainloader "${prefix}/LockDown.efi"

        # Refuse to unlimitedly run into auto provision if failed.
        set provision_failed="1"
        save_env provision_failed
    }
fi
