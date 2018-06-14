### Storage Encryption
This feature provides secure storage for application data. Some applications
need secure storage for sensitive data which must not be accessible to another
device. For example, only an application with the right signature can update
the data on an encrypted SD card. If you move that SD card to another device,
the data cannot be either read or updated. One application of this capability
is a POS application. The application keeps tax information in secure storage
that cannot be modified by another device.

This feature gives 2 types of granularity for storage encryption. Data volume
encryption allows the user to create encryption partition with a passphrase
typed by the end user. Root filesystem encryption enables the data encryption
on the entire rootfs except the boot partition.

Both types of storage encryption are based on device-mapper crypt target,
which provides transparent encryption of block devices using the kernel crypto
API. Additionally, the utility cryptsetup is used to conveniently set up disk
encryption, aka LUKS partition, based on device-mapper crypt target.

Due to the use of TPM state to seal the passphrase used to encrypt the storage,
the encrypted storage cannot be accessed on another machine, preventing from
the so-called offline attack.

### Dependency
This feature depends on [meta-tpm2](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm2).

Note:
Even though the hardware doesn't have a TPM 2.0 device, this feature can still
run on it, although without the guarantee of compromise detection.

### Limit
- TPM 2.0 is validated and officially supported. But TPM 1.2 device is not
  supported by this feature.

### Data Volume Encryption
#### Use case 1: manual operation
##### Create the LUKS partition
```
# cryptsetup --type luks --cipher aes-xts-plain --hash sha256 \
      --use-random luksFormat /dev/$dev
```
where $dev is the device node of the partition to be encrypted.

This command initializes a LUKS partition and prompts to input an initial
passphrase used to encrypt the data. Don't disclose the passphrase used for
product.

##### Open the LUKS partition
```
# cryptsetup luksOpen /dev/$dev $name
```
This command opens the LUKS device $dev and sets up a mapping $name after
successful verification of the supplied passphrase typed interactively. From
now on, the data written to /dev/mapper/$name is encrypted and the data
read back from /dev/mapper/$name is decrypted transparently and automatically.

##### Create the filesystem on top of the LUKS partition
The user can run any available filesytem formatting program on
/dev/mapper/$name to create the filesytem with the data encryption.

##### Close the LUKS partition
```
# cryptsetup luksClose $name
```
This command removes the existing mapping $name and wipes the key from kernel
memory.

To access the encryped partition, follow the instruction "Open the LUKS partition"
and then manually mount /dev/mapper/$name to a mount point.

#### Use case 2: luks-setup.sh
This script provides a semi automatic method to set up LUKS partition. The user
still needs to manually create the filesystem on top of the newly created LUKS
partition.

##### LUKS partition creation
In runtime, for example, create LUKS partition on /dev/sdb1 with the
name "my_luks_part":
```
# luks-setup.sh -d /dev/sdb1 -n my_luks_part -e
```
Note: if TPM is detected, the passphrase will be generated automatically.

For more uses about luks-setup.sh, run it with -h option.

##### Retrieve the passphrase
```
# cryptfs-tpm2 -q unseal passphrase -P sha1 -o /tmp/passphrase
```
This command will unseal the passphrase from TPM device and save the content
of passphrase to the file /tmp/passphrase.

The passphrase should not be disclosed and needs to be backed up to a off-line
storage.

##### Open the LUKS partition
```
# cryptsetup luksOpen --key-file /tmp/passphrase /dev/$dev $name
```
##### Mount the LUKS partition
```
# mount /dev/mapper/$name $mount_point
```
The remaining operations are left to the user. Don't forget to close the LUKS
partition if not used.

Note:
If TPM device exists in the system, the passphrase will be bound to PCR 7,
gating the unseal operation. If the value of PCR 7 when unsealing the
passphrase doesn't match up the value when creating the passphrase, the
passphrase cannot be unsealed. The value of PCR 7 is usually affected by the
status of UEFI secure boot.

### Root Filesystem Encryption
This enables the data encryption on the rootfs without the need of a human
entering an user passphrase. Therefore, it is required to employ an initramfs
where the unique identity from the platform is collected from the devices on
the platform and used to unlock the root filesystem encryption. Meanwhile, use
TPM to protect the user passphrase for volume decryption to avoid disclosing
the user passphrase. If the TPM device is not detected, the end user will be
prompted to type the user passphrase.

#### Operations
Note:
The instructions below with the prefix "[TPM]" indicate the operation
requires TPM device. Oppositely, the prefix "[Non-TPM]" indicates this
operation is required if the target board doesn't have a TPM device.

- Ensure a hard drive is attached on target board
  WARNNING: the following instructions will wipe all data in the hard drive.

- Create overc installer on a USB device
  Refer to layers/meta-overc/README.install for the details about how to
  run cubeit to install overc installer to a USB device.

- Attach the USB device to the board

- Power on

- [TPM] Clear TPM
  Refer to meta-tpm2/README.md for the details.

- Boot to Linux

- Install overc runtime on the hard drive
  Refer to layers/meta-overc/README.install for the details about how to
  run cubeit-installer to install overc runtime to a hard drive. In
  addition, beware of specifying "--encrypt" option to set up an
  encrypted rootfs.

- Reboot
  After reboot to initramfs, it employs a init script to transparently
  unseal the passphrase and mount the rootfs without any interaction.

### Best Practice
- The benefit of anchoring the TPM is that the machine status cannot be
  compromised by any attack. If compromised, the system cannot boot up
  due to the failure when mouting the rootfs, or access the encrypted partition
  when mounting the LUKS partition. This is caused by the fact that the content
  of PCR 7 is different with the value when creating the encrypted storage.
  Usually, the content of PCR 7 is calculated based on the status of UEFI
  secure boot.

  Based on the above conclusion, it is recommended to provision UEFI secure
  boot and turn on it prior to setting up the encrypted storage.

- The non-default seal secret is supported to provide extra protection, and it
  is user configurable. Modify the values of CRYPTFS_TPM2_PRIMARY_KEY_SECRET
  and CRYPTFS_TPM2_PASSPHRASE_SECRET in cryptfs-tpm2 with your preference.

### Known Issues
- The default IMA rules provides the ability of measuring the boot components
  and calculating the aggregate integrity value for attesting. However, this
  function conflicts with this feature which employs PCR policy session to
  retrieve the passphrase in a safe way. If the installer enables both of
  them, the default IMA rules will be not used.

### Reference
- [OpenEmbedded layer for TPM 2.0 enablement](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm2)
