### Overview
This layer consists of two widely used secure boot technologies: UEFI Secure
Boot and MOK (Machine Owner Key) Secure Boot.

- UEFI Secure Boot is the industry standard defined in the UEFI spec, allowing
the images loaded by UEFI firmware to be verified with the certificates
corresponding to the trusted keys.
- MOK Secure Boot is based on UEFI Secure Boot, adding the shim bootloader to
chainloader the next stage bootloader with the integrity check using the
shim-managed certificates corresponding to another set of trusted keys, which
may be different than the trusted keys used by UEFI Secure Boot.

This layer introduces the SELoader as the second-stage bootloader and eventually
chainliader to the third-stage bootloader "grub". With the extension provided
by SELoader, grub configuration files, kernel (even without EFI stub support)
and initrd can be authenticated. This capability is not available in the shim
bootloader.

Grub bootloader is also enhanced to support lockdown mode. In this mode, the
edit, rescue and command line are protected in order to prevent from
tampering the kernel command line or loading an unsigned boot component. Hence,
this lockdown protection can effectively defeat the attempts to disable the
kernel security mechanisms, e.g, globally disable SELinux or IMA. The
flexibility is also provided with the user authentication in grub. The user
authenticated by a password check can enter into edit and command line.

Therefore, using UEFI Secure Boot, shim, SELoader, and grub lockdown together,
the boot process is completely trustworthy.

A complete boot flow looks like as following:

- UEFI firmware boot manager (UEFI Secure Boot enabled) ->
  - shim (verified by a DB certificate) ->
    - SELoader (ditto) ->
      - grub (ditto) ->
        - grub.cfg (ditto)
        - kernel (ditto)
        - initramfs (ditto)

### Quick Start For The First Boot
- Deploy the rootfs

- Power up the system

- Enter to BIOS setup and remove the enrolled certificates
  * It is recommended to still turn on UEFI Secure Boot option if allowed.

- Exit BIOS setup

- Manually launch a reboot immediately via Ctrl + Alt + Del
  * Otherwise, a misleading error message about the verification failure
  will be displayed.

- Automatically boot to the boot option "Automatic Certificate Provision" in
  grub boot menu.

- (Optional) Enter to BIOS setup to turn on UEFI Secure Boot option and then
  exit BIOS setup

- Boot to the system with the protection provided by UEFI and MOK Secure Boot

### Key Management
Refer to meta-signing-key/README.md for the initial cognition about key
management.

Note that the sample key and user key are the concepts in the key signing
model according to the ownership and secrecy. In UEFI Secure Boot, a policy
object such as PK, KEK, DB and DBX is always mapped to a key useed by the
key signing model.

#### Sample Keys
This layer, by default, use **the sample keys** to sign and verify images for
the purpose of development and demonstration. **Please ensure you know what your
risk is to use the sample keys in your product, because they are completely
public.**

The sample keys used for UEFI Secure Boot are centrally placed under
meta-signing-key/files/uefi_sb_keys/.

- PK.crt  
  The X509 certificate enrolled to UEFI firmware, used to update/delete PK/KEK.

- PK.key  
  The private key corresponding to PK.crt, used to sign the EFI signature
  list for PK/KEK enrollment.

- KEK.crt  
  The X509 certificate enrolled to UEFI firmware, used to update/delete
  DB/DBX.

- KEK.key  
  The private key corresponding to KEK.crt, used to sign the EFI signature
  list for DB/DBX enrollment.

- DB.crt  
  The X509 certificate enrolled to UEFI firmware, used to verify the images
  directly loaded by UEFI firmware.

- DB.key  
  The private key corresponding to DB.crt, used to sign the images directly
  loaded by UEFI firmware.

- DBX  
  This directory contains any number of X509 certificate enrolled to UEFI
  firmware, used to blacklist the revoked certificates. Note the revoked
  certificates must be PEM-formatted.

The sample keys used for MOK Secure Boot are centrally placed under
`meta-signing-key/files/mok_sb_keys/`.

- shim_cert.crt  
  The X509 certificate embedded in shim, used to verify the images either
  directly or indirectly loaded by shim. Currently, this certificate is
  not used by default.

- shim_cert.key  
  The private key corresponding to shim_cert.crt, used to sign the images
  either directly or indirectly loaded by shim. Currently, this certificate
  is not used by default.

- vendor_cert.crt  
  Act as the same way as shim_cert.crt. In addition, vendor certificate
  is the switch to enable MOK Verify Protocol, which facilitates the
  verification for the SELoader and MOK Manager.

- vendor_cert.key  
  The private key corresponding to vendor_cert.crt, acting as the same fuction
  as shim_cert.key.

- vendor_dbx  
  This directory contains any number of X509 certificate embedded in shim,
  used to blacklist the revoked certificates.

#### User Keys
Refer to meta-signing-key/README.md for the details about how to generate/use
the keys owned by the end user.

#### Automatic Certificate Provision
The certificate provision is required to enable UEFI Secure Boot. By default,
the system may be already provisioned with default certificates enrolled during
the manufacture.

In order to use the bootloader and kernel signed by the sample or self-owned
key to boot up the system, this layer provides a process of automatic
certificate provison for the convenience. The detailed descriptions are
given below.

##### Remove the enrolled certificates in BIOS setup
The EFI/BOOT/LockDown.efi is used to run the automatic certificate provision.
However, LockDown.efi cannot be launched if UEFI Secure Boot is already
enabled. In addition, the enrolled certificates may be not the ones the user
hopes to use.

The provisioned certificates can be removed through BIOS setup. The detailed
steps may vary between the systems. Refer to the corresponding BIOS manual for
the instructions.

##### Launch the automatic certificate provision
The Lockdown.efi will automatically provision UEFI Secure Boot after removing
the enrolled certificates in BIOS setup. More specifically, the new PK, KEK, DB
and DBX (if any) will be enrolled and begin to take affect after a reboot.

The new PK, KEK, DB and DBX (if any) were built into LockDown.efi during the
build.

##### Turn on UEFI Secure Boot option
If UEFI Secure Boot option was turned off, the user has to enter to BIOS setup
again after the automatic certificate provision in order to manually turn on
this option.

If this option was not turned off when removing the enrolled certificates in
BIOS setup, this step is skippable.

##### Re-trigger automatic certificate provision
The boot option "Automatic Certificate Provision" is hidden in grub boot menu
for the first boot. If the user would like to clear the certificates
provisioned by the option "Automatic Certificate Provision" in BIOS setup, this
hidden boot option will be shown, allowing to re-trigger it if necessary.

### Signing
By default, the build system uses DB.key to sign shim, and uses vendor_cert.key
to sign SELoader, grub, grub configuration file, kernel and initramfs image
during the build.

### Verification

#### UEFI firmware verification
UEFI firmware will validate the integrity of shim bootloader with a certificate
in DB before launching it.

#### Bootloader verification
This layer employs 3-level bootloader for secure boot process. Each former
bootloader must check the integrity e.g, when the
SELoader loads grub, if both UEFI Secure Boot and MOK Secure Boot are already
enabled, the former bootloader uses a list of certificate to check the
integrity of the later bootloader.

- Blacklist check
  If the later bootloader is signed with a key corresponding to a certificate
  within any of a policy object below, the later bootloader is denied to
  launch immediately, without the necessity to go through the following
  processes.

  * Vendor DBX
  * DBX
  * MokListX (the blacklist of MOK certificate)

- Whitelist check
  If the later bootloader is signed with a key corresponding to a certificate
  within any of a policy object below, the later bootloader is granted to
  launch.

  * DB
  * MokList (the whitelist of MOK certificate)
  * Shim certificate (only for PE image)
  * Vendor certificate

If the later bootloader is not signed or signed by a key not corresponding to
any policy objects mentioned above, the later bootloader is denied to launch.

The benefit of showing this checklist allows the end user to use an
appropriater way to manage the key and boot up the system, even without the
ownership of a signing key, such as the DB key widely used on Microsoft
certificated hardware.

##### SELoader verification
The SELoader is designed to authenticate the non-PE files, such as grub.cfg,
kernel (without EFI stub support) and initramfs, which cannot be verified by
the MOK Verify Protocol registered by the shim loader.

In order to conveniently authenticate the PE file with gBS->LoadImage()
and gBS->StartImage(), the SELoader hooks EFI Security2 Architectural
Protocol and employs MOK Verify Protocol to verify the PE file. If only
UEFI Secure Boot is configured and enabled, the SELoader just simplily calls
gBS->LoadImage() and gBS->StartImage() to allow UEFI firmware to verify the
PE file.

The SELoader publishes MOK2 Verify Protocol which provides a flexible interface
to allow the bootloader to verify the file, file buffer or memory buffer
without knowing the file format. This design allows the non-PE files to be
verified by the same certificate used for authenticating PE files.

In order to establish the chain of trust, the SELoader is required to be
signed by a private key corresponding to a DB certificate, the shim
certificate, the vendor certificate or a MOK certificate mentioned above.
The specific key used is determined by the secure boot scheme you will use.

See more details about README in SELoader.

#### Grub configuration file verification
Grub is enhanced to have the capability of calling MOK2 Verify Protocol
registered by the SELoader to validate the integrity of grub configuration
file before parsing it.

This protection prevents from tampering the grub configuration file from
globally disabling certains kernel security mechanism such as SELinux and IMA
which are activated in kernel command line.

#### Kernel verification
When grub loads the kernel image with the command "linux", if both UEFI
Secure Boot and MOK Secure Boot are already enabled, grub will call the
MOK2 Verify Protocol installed by SELoader to validate the kernel image.

It is recommended to avoid using the command "chainloader" to load kernel
image. The build system also avoids signing the kernel with EFI-stub
bootloader.

By default, the kernel image is signed by vendor certificate and generate
the .p7b signature file.

#### Initramfs verification
When grub loads the kernel image with the command "initrd", if both UEFI
Secure Boot and MOK Secure Boot are already enabled, grub will call the
MOK2 Verify Protocol installed by SELoader to validate the initramfs image.

By default, the initramfs image is signed by vendor certificate and generate
the .p7b signature file.

#### Verification failure
Either situation will cause a failure of verification.
- A boot component is not signed.
- A boot component is signed by a key which doesn't correspond to any
  certificate in whitelists such as DB and shim-managed certificates as
  mentioned above.
- A boot component is signed by a key which corresponds to a certificate in
  blacklist such as DBX and shim-managed certificates in blacklist as
  mentioned above.

Each boot component may have different verification failure phenomenon.
- If shim fails signature check, UEFI firmware boot manager will print an
  error message about the image authentication failure.
- If SELoader fails signature check, shim will print an error message about
  the security violation.
- If grub fails signature check, an image authentication failure message is
  printed and the system hangs.
- If a grub configuration file fails the signature check, an authentication
  failure message is printed and grub hangs.
- If kernel image fails signature check, grub returns back to the boot menu.
- If initrd fails signature check, grub returns back to the boot menu.

### MOK Secure Boot and the shim bootloader
MOK Secure Boot is based on UEFI Secure Boot, adding the shim bootloader to
chainloader the bootloader "SELoader" and eventually chainliader to the
bootloader "grub".

[ Quoting: https://github.com/rhboot/shim ]
shim is a trivial EFI application that, when run, attempts to open and
execute another application. It will initially attempt to do this via the
standard EFI LoadImage() and StartImage() calls. If these fail (because secure
boot is enabled and the binary is not signed with an appropriate key, for
instance) it will then validate the binary against a built-in certificate. If
this succeeds and if the binary or signing key are not blacklisted then shim
will relocate and execute the binary.

shim will also install a protocol which permits the second-stage bootloader
to perform similar binary validation. This protocol has a GUID as described
in the shim.h header file and provides a single entry point. On 64-bit systems
this entry point expects to be called with SysV ABI rather than MSABI, and
so calls to it should not be wrapped.
[ End of Quote ]

In most cases, the hardware coming out of the factory is already provisioned
with a default certificate used to verify the bootloader and issued by
Microsoft Corporation UEFI CA 2011. This kind of hardware is so-called
Microsoft certificated hardware.

Obviously, this requirement needs a bootloader loaded by BIOS must be signed
by Microsoft. Microsoft provides the signing service (not free), but only
accept shim bootloader for Linux world. Refer to [Microsoft's signing policy](http://blogs.msdn.com/b/windows_hardware_certification/archive/2013/12/03/microsoft-uefi-ca-signing-policy-updates.aspx).

It is allowed to remove all default certificates and use the self-owned keys to
provision UEFI firmware, but this is not practical for ODM/OEM devices during
the manufacture phrase. See the section "Out-of-box Experience".

For a good user experience, shim + SELoader + grub is an excellent combination
to handle Microsoft certificated hardware. With this model, SELoader and grub
are signed by a shim-managed certificate without being subject to the limit
from Microsoft's signing policy, and the manual provision is thus unnecessary.

#### mokutil and MOK Manager
mokutil is a tool to import or delete the machines owner keys stored in the
database of shim. mokutil creates the requests and MOK manager will be
automatically launched by shim as long as it detects the pending requests.
The physical present user will be prompted to run the operations corresponding
to the requests. Note the operation is required to be authenticated by MOK
management password set by mokutil.

Refer to mokutil man page for the detailed usages.

##### MOK Management Password
MOK management password is the authentication information to allow MOK manager
to grant the request regarding of MOK management. To set the password, run
mokutil with the option --password. In addition, there are 4 input methods to
provide the password. By default, mokutil prompts the user to input the
password and then wraps the password to sha256 password hash. For other 3
methods, refer to the uses of option --hash-file, --root-pw and --simple-hash.

##### Enroll the MOK certificate
Here is an example showing how to enroll a DER formatted X509 certificate to
the database of shim.
```
# mokutil --import <cert.cer>
```
where `<cert.cer>` is the MOK certificate corresponding to the private key used
to sign either grub or kernel.

To convert a PEM, for exmaple, the shim_cert.crt, to a DER formatted X509
certificate, type the command:
```
$ openssl x509 -in shim_cert.crt -inform PEM -out shim_cert.cer -outform DER
```

##### List the enrollment requests
The several enrollment requests can be submitted before system reboot. Run the
following command to check all enrollment requests.
```
# mokutil --list-new
```

##### Revoke the enrollment requests
Note the revocation operation will remove all enrollment requests.
```
# mokutil --revoke-import
```

##### Test the MOK certificate
If you cannot confirm whether a certificate has been enrolled or not, type the
following command for a check:
```
# mokutil --test-key <cert.cer>
```

##### Delete the MOK certificate
Removing an useless MOK certificate is also supported.
```
# mokutil --delete <cert.cer>
```
Refer to the options --list-delete and --revoke-delete to list and revoke the
MOKs.

##### Reset MOK certificates
This request will clear all enrolled MOK certificates.
```
# mokutil --reset
```

##### Disable/Enable MOK Secure Boot
MOK Secure Boot can be enabled or disabled regardless of the setting of UEFI
Secure Boot.
```
# mokutil --disable-validation  // disable MOK Secure Boot
# mokutil --enable-validation   // enable MOK Secure Boot
```

Note that MOK Secure Boot is based on UEFI Secure Boot. If UEFI Secure Boot
is disabled, MOK Secure Boot will be automatically inactive. Type the
following command to check the status of UEFI Secure Boot.
```
# mokutil --sb-state
```

##### Other options
Refer to the options --import-hash and --delete-hash to manage hash-based
signature. The options --pk, --kek, --db and --dbx are useful to check
the content of the policy objects used in UEFI Secure Boot.

##### Manage blacklist
All above mentioned are talking MOK which is acting as whitelist to
authenticate the verified image to launch. Actually, there is a contrary
policy object called MOKX, acting as blacklist to deny the untrusted
image to launch. Also, MOKX as blacklist is handled by shim prior to MOK
as whitelist.

For the management of blacklist, add the option --mokx with the following
options to change the operation target from MOK to the following options.

--list-enrolled
--test-key
--list-new
--list-delete
--import
--delete
--import-hash
--delete-hash
--reset
--revoke-import
--revoke-delete

##### Handle MOK Secure Boot failure with MOK Manager
If either grub or SELoader is not signed or signed with an unauthorized
certificate, the shim will prompt the end user a UI called MOK Manager to
guide the user to enroll the certificate or hash of the image.

The policy of the selection between digest and certificate for next step is
decided by whether the unauthorized grub or SELoader is signed or not.

If the grub or SELoader is not signed at all, you have to always select
the calculation of the digest based on the file. Note that once grub or
SELoader is updated and its digest is changed, you have to relaunch the MOK
Manager to enroll the new digests.

If the grub or SELoader is signed by an unauthorized certificate, enrolling the
signing certificate is the preferred way. Copy the certificate to the boot
drive and then select the certificate in MOK manager. Note that the
certificate for the selection must be **DER-formatted**.

If doing so, the unauthorized grub or SELoader will be verified successfully
after exiting MOK Manager.

### Grub Lockdown
In order to prevent from tampering the kernel command line or loading an
unsigned boot component, grub is locked if UEFI Secure Boot is enabled. In this
situation, the end user cannot enter into command or edit line via pressing 'c'
and 'e'.

If the user authentication is enabled, the access to command or edit line is
protected by a password. In this situation, grub is unlockable.

Rescue mode is always disabled as long as UEFI Secure Boot is enabled.

### Known Issues
- The 32-bit MOK Secure Boot is not validated. In other words, loading 32-bit
shim, MOK manager, grub and kernel is not supported.
- grub module is not supported by SELoader for the integrity check.

### Reference
[shim - implement MOK Verify Protocol](https://github.com/rhboot/shim)

[SELoader - implement MOK2 Verify Protocol](https://github.com/jiazhang0/SELoader)

[grub - Mok2Verify patch](https://github.com/jiazhang0/meta-secure-core/blob/master/meta-efi-secure-boot/recipes-bsp/grub/grub-efi/mok2verify-support-to-verify-non-PE-file-with-PKCS-7.patch)

[SecureCore - a reference implementation based on OpenEmbedded](https://github.com/jiazhang0/SecureCore)
