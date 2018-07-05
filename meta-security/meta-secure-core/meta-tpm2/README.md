### TPM 2.0
This feature enables Trusted Platform Module (TPM 2.0) support, including 
kernel option changes to enable tpm drivers, and picking up TPM 2.0 packages.

Trusted Platform Module (TPM 2.0) is a microcontroller that stores keys,
passwords, and digital certificates. A TPM 2.0 offers the capabilities as
part of the overall platform security requirements.

### Clear TPM
For TPM 2.0, the following typical steps can be performed to get the TPM
ready for use:

- Clear and enable TPM from the BIOS or set the security jumper on the board.
- Take TPM ownership, setting Owner/Endorsement/Lockout passwords if
  necessary. These passwords are used for the authorization to certain
  TPM 2.0 commands.
```
  # tpm2_takeownership -o <ownerPasswd> -e <endorsePasswd> -l <lockPasswd>
```
Then, you can use the TPM for a specific need, such as key generation,
sealing encrypted data, etc.
