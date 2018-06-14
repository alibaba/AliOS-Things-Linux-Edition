### TPM 1.2
This feature enables tpm 1.2 support, including kernel option changes to
enable tpm drivers, and picking up packages trousers, tpm-tools,
openssl-tpm-engine, tpm-quote-tools.

### How to use TPM 1.2
For TPM 1.2, the following typical steps can be performed to get the TPM
ready for use:

- Clear and enable TPM from the BIOS.
- Take TPM ownership.
  ```
  # tpm_takeownership -y -z
  ```
- Change owner and SRK passwords. These password are used for the
  access permission to other functions including generate keys.
  ```
  # tpm_changeownerauth -z -s -o
  ```
Then, you can use the TPM for a specific need, such as key generation,
sealing encrypted data, etc.

### openssl tpm engine for TPM 1.2
openssl-tpm-engine package provides tpm engine lib for openssl applications.
It is an interface between openssl applications and TPM hardware.

- Wrap software key using the TPM engine
  - If tpm is not owned, run "tpm_takeownership -y -z".
    This also set tpm SRK and Owner password to well-known key.
    You can then run tpm_changeownerauth to set new SRK and Owner password.
    To reset SRK and Owner password to well-known key, run "tpm_changeownerauth -r -s -o".
  - Generate tpm hardware key: (add -z for using well-known key)
    ```
    $ create_tpm_key rootkey.pem [-z]
    ```
  - Wrap software key into TPM storage:
    $ openssl genrsa -out softkey.pem 1024
    $ create_tpm_key -w softkey.pem -s 1024 rootkey.pem [-z]
- Create a self-signed cert using the TPM engine
  - Generate a TPM key and write it to a file:
    ```
    $ create_tpm_key <keyfilename>
    ```
- Make the openssl certificate request:
  ```
  $ openssl req -keyform engine -engine tpm -key <keyfilename> -new -x509 -days 365 -out <certfilename>
  ```
- How to use tpm engine lib
The name of tpm engine library is libtpm.so.
There is an enhencement to the original opensource code.
Add an additional way to pass SRK passwork to libtpm.so,
that is using environment variable "TPM_SRK_PW".
For example:
```
	env TPM_SRK_PW=xxx openssl s_server ...
```
Note:
- "env TPM_SRK_PW=#WELLKNOWN#" is used to pass well-known key.
- Detail description about openssl-tpm-engine, please refer to the README in source code.
