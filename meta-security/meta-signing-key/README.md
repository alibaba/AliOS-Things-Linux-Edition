### User Key Store
The sample keys, by default, are used by build system to sign bootloader,
kernel, IMA signature, RPM and so on. It is used for development and
demonstration. The user must know what te risk is to use the sample
keys in the product.

The user key in a general sense is able to be used in the product with
contrary of the sample key. This document defines the definitions for
the uses of various keys.

In addition, the scripts/create-user-key-store.sh provides a reference
to the creation of user key store, stored in such a layout:
```
user-keys
├── ima_keys
│   ├── x509_ima.der
│   └── x509_ima.key
├── mok_sb_keys
│   ├── shim_cert.key
│   ├── shim_cert.crt
│   ├── vendor_cert.key
│   └── vendor_cert.crt
└── uefi_sb_keys
    ├── DB.key
    ├── KEK.key
    ├── KEK.crt
    ├── PK.key
    └── PK.crt
```
If the user plans to create the user keys by self, please consider to
define the necessary variables mentioned below in local.conf, or construct
a layer for the user key store. Eventually, the build system will copy the
user key store to `$project/tmp/deploy/images/*/user-keys/` for further use.

The vital definitions include:

- `SIGNING_MODEL := "user"`  
  Prohibit using the sample keys for signing the images.

- `UEFI_SB_KEYS_DIR := "<path>"`  
  Point to the location of user keys used for UEFI secure boot.

- `MOK_SB_KEYS_DIR := "<path>"`  
  Point to the location of user keys used for MOK secure boot. Note that
  MOK secure boot is on top of UEFI secure boot so creating the user keys
  for MOK secure boot only will still introduce the security risk in your
  product.

- `IMA_KEYS_DIR := "<path>"`  
  Point to the location of user keys used for IMA appraisal.

- `USER_KEY_SHOW_VERBOSE = "1"`  
  Optional. Used to enable the verbose output for debugging purpose.

To ensure a image signed by the untrustworthy sample key cannot be loaded, e.g,
preventing the shim signed by the user key from loading the grub signed by the
sample key, certain sample keys are added to the blacklists during the build,
meaning the following precautions:

- Blacklist the sample DB and DBX in DBX database for UEFI secure boot.
- Blacklist the sample DB, shim_cert and vendor_cert in vendor_dbx database
  for MOK secure boot.
- Cascade the default blacklist mentioned above and the user specified
  blacklist if any.

For the details about UEFI secure boot and MOK secure boot, please refer
to meta-efi-secure-boot/README.md.

### Reference
