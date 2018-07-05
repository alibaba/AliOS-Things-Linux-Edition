### meta-integrity
OpenEmbedded layer for Linux integrity support

#### Integrity Measurement Architecture (IMA)
The Linux IMA subsystem introduces hooks within the Linux kernel to support
measuring the integrity of files that are loaded (including application code)
before it is executed or mmap()ed to memory. The measured value (hash) is then
registered in a log that can be consulted by administrators.

To support proven integrity of the files, the IMA subsystem can interact with
the TPM chip within the system to protect the registered hashes from tampering
by a rogue administrator or application. The IMA subsystem, as already
supported by the Linux kernel, supports reporting on the hashes of files and
commands ran by privileged accounts (and more if you create your own
measurement policies).

In addition, IMA appraisal can even register the measured value as an extended
attribute, and after subsequent measurement(s) validate this extended attribute
against the measured value and refuse to load the file (or execute the
application) if the hash does not match. In that case, the IMA subsystem allows
files and applications to be loaded if the hashes match (and will save the
updated hash if the file is modified) but refuse to load it if it doesn't. This
provides some protection against offline tampering of the files.

By default, the following constraint conditions are applied by design of this
layer:

- Appraise the files for exec'd (the executables), files mmap'd for exec
  (shared libraries), kernel modules and firmwares in effective root identity
  (euid=0).
- Enforce verifying the IMA signature when running the executables, shared
  libraries, kernel modules and firmwares.
- Deny to run the newly created executables, shared libraries, kernel modules
  and firmwares.
- Deny to run the tampered executables, shared libraries, kernel modules and
  firmwares.
- Deny to run any executables, shared libraries, kernel modules and firmwares
  in the filesystems without file extended attribute supported.
- Allow to run the manually signed executables, shared libraries, kernel
  modules and firmwares.
- Allow to run the updated executables, shared libraries, kernel modules and
  firmwares during RPM installation.
- Enforce the subsequent policy file write to be verified by a trusted IMA
  certificate.

NOTE:
- The different behaviors when executing a script, e.g, launching a python
script with "./test.py" is allowed only when test.py is signed, and launching
a python script with "python test.py" is always allowed as long as the python
interpreter is signed.
- Extended file system attribute is required for IMA appraisal, but not
all file systems can support it. Typically, the pseudo file systems, such as
sysfs, proc, tmpfs and ramfs, certain disk-based file systems, such as FAT,
and network file systems, such as NFS, don't support extended attribute,
meaning IMA appraisal is not available with them.

##### Dependency
- [meta-tpm](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm)  
  This layer provides the kernel configurations and TSS for TPM 1.x enablement.

- [meta-tpm2](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm2)  
  This layer provides the kernel configurations and TSS for TPM 2.0 enablement.

##### Use The External IMA Policy
initramfs is a good place to run some IMA initializations, such as loading
the IMA policy, as well as the trusted IMA certificate used to verify IMA
signatures.

###### The default external IMA policy
The default external IMA policy enforces appraising all the executable, shared
library, kernel modules and firmwares with the digital signature in the
effective root identity (euid=0). Hence, the opportunity of loading the default
external IMA policy occurs at the end of initramfs initializations, just before
switch_root.

Instead of running switch_root directly from initramfs, a statically linked
switch_root from the real rootfs is launched and it must be already signed
properly. Otherwise, switch_root will fail to mount the real rootfs and kernel
panic will happen due to this failure.

The default external IMA policy is located at `/etc/ima/ima_policy.default` in
initramfs.

###### The custom external IMA policy
If the default external IMA policy cannot meet the protection requirement, it
is allowed to define the custom external IMA policy, which will be used instead
of the default external IMA policy.

The custom external IMA policy file is eventually installed to `/etc/ima/ima_policy`
in initramfs.

In addition, the IMA policies signed by the trusted IMA certificate in the real
rootfs are also attempted to be loaded if any, in the pattern of file name as
`/etc/ima/ima_policy*`.

##### IMA certificate & private Key
The private key come in two flavors; one used to sign all regular files in
rootfs and one used by RPM to re-sign the executable, shared library, kernel
module and firmware during RPM installation. Correspondingly, the IMA
certificate is used to verify the IMA signature signed by the private key.

In addition, initramfs is a good place to import the IMA certificate likewise.

Note that the IMA certificate must be signed by the system trusted key by
design. This guarantees the imported IMA certificate is always trustworthy.

###### The default IMA certificate & private key
The default IMA certificate & private key are generated by the build system. By
default, the sample keys are used for the purpose of development and
demonstration. Please ensure you know what your risk is to use the sample keys
in your product, because they are completely public.

### RPM File Signing
The payloads in a RPM are signed by the private key during the build, and each
IMA signatures for the corresponding  payload file will be eventually written
to the filesystem during RPM installation.

In order to check whether a RPM is signed, run the command
`rpm -qp --queryformat "%{FILESIGNATURES:arraysize}\n" <rpm>`

If the result is not none or zero, the specified RPM contains the signed
payloads.

### Tarball Signing
Packing the IMA signatures into a tarball is another method to preserve the
IMA signatures. Be aware of using `--xattrs --xattrs-include=security\\.ima`
with both extraction and creation operations.

### Best practice
The following best practices should be applied with using IMA.

- Enable UEFI/MOK secure boot
  UEFI/MOK secure boot can verify the integrity of initramfs, providing the
  protection against tampering of the external IMA policy files and IMA public
  keys stored in initramfs.

- Moderate measuring
  Measuring the files owned by non-root user may introduce malicious attack.
  Malicious user may create lots of files with different names or trigger
  violation conditions to generate a mass of event logs recorded in the runtime
  measurement list, and thus exhaust the persistent kernel memory.

- Performance influence
  Moderate policy can make a good balance between the performance and security.
  Tune the default external policy (`/etc/ima_policy.default`) and modulate the
  custom policy for the product requirement.

- Use IMA digital signature to protect the executable
  Using the digital signature scheme DIGSIG is safer than digest-based scheme.
  Meanwhile, use `appraise_type=imasig` in your IMA policy to enforce running
  this.

- Use the measurement and audit rules together
  The runtime measurement list is unable to track down the order of changes for
  a file, e.g, a file content varies in order of X -> Y -> X. However, audit log
  can record these changes in the right order.

##### Known Issues
- The following operations may break the behavior of appraisal and cause the
  failure of launching the executables, shared libraries, kernel modules and
  firmwares:
  - the syscalls used to set file last access and modification times.
  - the syscalls used to set ownership of a file.
  - the syscalls used to set permissions of a file.

  To fix the failure, manually re-sign the affected file.

- Overwriting an existing file with the same content is deemed as tampering of
  the file.

- The default IMA rules provides the ability of measuring the boot components
  and calculating the aggregate integrity value for attesting. However, this
  function conflicts with the luks feature which employs PCR policy session to
  retrieve the passphrase in a safe way. If both of them are enabled, the
  default IMA rules will be not used.

### Reference
[Official IMA wiki page](https://sourceforge.net/p/linux-ima/wiki/Home/)

[OpenEmbedded layer for EFI Secure Boot](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-efi-secure-boot)

[OpenEmbedded layer for signing key management](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-signing-key)

[OpenEmbedded layer for TPM 1.x](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm)

[OpenEmbedded layer for TPM 2.0](https://github.com/jiazhang0/meta-secure-core/tree/master/meta-tpm2)
