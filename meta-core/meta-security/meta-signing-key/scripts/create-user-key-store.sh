#!/bin/bash

_S="${BASH_SOURCE[0]}"
_D=`dirname "$_S"`
ROOT_DIR="`cd "$_D" && pwd`"

KEYS_DIR="$ROOT_DIR/user-keys"
GPG_KEYNAME=
GPG_EMAIL=

function show_help()
{
    cat <<EOF
$1 - creation tool for user key store

(C)Copyright 2017, Jia Zhang <lans.zhang2008@gmail.com>

Usage: $1 options...

Options:
 -d <dir>
    Set the path to save the generated user keys.
    Default: `pwd`/user-keys

 -n <gpg key name>
    Set the gpg's key name
    Default: SecureCore

 -m <gpg key ower's email address>
    Set the ower's email address of the gpg key
    Default: SecureCore@foo.com

 -h|--help
    Show this help information.

EOF
}

print_critical() {
    printf "\033[1;35m"
    echo "$@"
    printf "\033[0m"
}

print_error() {
    printf "\033[1;31m"
    echo "$@"
    printf "\033[0m"
}

print_warning() {
    printf "\033[1;33m"
    echo "$@"
    printf "\033[0m"
}

print_info() {
    printf "\033[1;32m"
    echo "$@"
    printf "\033[0m"
}

print_verbose() {
    printf "\033[1;36m"
    echo "$@"
    printf "\033[0m"
}

while [ $# -gt 0 ]; do
    opt=$1
    case $opt in
        -d)
            shift && KEYS_DIR="$1"
            ;;
        -n)
            shift && GPG_KEYNAME="$1"
            ;;
        -m)
            shift && GPG_EMAIL="$1"
            ;;
        -h|--help)
            show_help `basename $0`
            exit 0
            ;;
        *)
            echo "Unsupported option $opt"
            exit 1
            ;;
    esac
    shift
done

echo "KEYS_DIR: $KEYS_DIR"

UEFI_SB_KEYS_DIR="$KEYS_DIR/uefi_sb_keys"
MOK_SB_KEYS_DIR="$KEYS_DIR/mok_sb_keys"
SYSTEM_KEYS_DIR="$KEYS_DIR/system_trusted_keys"
IMA_KEYS_DIR="$KEYS_DIR/ima_keys"
RPM_KEYS_DIR="$KEYS_DIR/rpm_keys"
MODSIGN_KEYS_DIR="$KEYS_DIR/modsign_keys"
EXTRA_SYSTEM_KEYS_DIR="$KEYS_DIR/extra_system_trusted_keys"

pem2der() {
    local src="$1"
    local dst="${src/.crt/.der}"

    openssl x509 -in "$src" -outform DER -out "$dst"
}

ca_sign() {
    local key_dir="$1"
    local key_name="$2"
    local ca_key_dir="$3"
    local ca_key_name="$4"
    local subject="$5"
    local encrypted="$6"

    # Self signing ?
    if [ "$key_name" = "$ca_key_name" ]; then
        openssl req -new -x509 -newkey rsa:2048 \
            -sha256 -nodes -days 3650 \
            -subj "$subject" \
            -keyout "$key_dir/$key_name.key" \
            -out "$key_dir/$key_name.crt"
    else
        if [ -z "$encrypted" ]; then
            openssl req -new -newkey rsa:2048 \
                -sha256 -nodes \
                -subj "$subject" \
                -keyout "$key_dir/$key_name.key" \
                -out "$key_dir/$key_name.csr"
        else
            # Prompt user to type the password
            openssl genrsa -des3 -out "$key_dir/$key_name.key" 2048

            openssl req -new -sha256 \
                -subj "$subject" \
                -key "$key_dir/$key_name.key" \
                -out "$key_dir/$key_name.csr"
        fi

        local ca_cert="$ca_key_dir/$ca_key_name.crt"
        local ca_cert_form="PEM"

        [ ! -s "$ca_cert" ] && {
            ca_cert="$ca_key_dir/$ca_key_name.der"
            ca_cert_form="DER"
        }

        openssl x509 -req -in "$key_dir/$key_name.csr" \
            -CA "$ca_cert" \
            -CAform "$ca_cert_form" \
            -CAkey "$ca_key_dir/$ca_key_name.key" \
            -set_serial 1 -days 3650 \
            -extfile "$ROOT_DIR/openssl.cnf" -extensions v3_req \
            -out "$key_dir/$key_name.crt"

        rm -f "$key_dir/$key_name.csr"
    fi
}

create_uefi_sb_user_keys() {
    local key_dir="$UEFI_SB_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" PK "$key_dir" PK \
        "/CN=PK Certificate/"
    ca_sign "$key_dir" KEK "$key_dir" KEK \
        "/CN=KEK Certificate"
    ca_sign "$key_dir" DB "$key_dir" DB \
        "/CN=DB Certificate"
}

create_mok_sb_user_keys() {
    local key_dir="$MOK_SB_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" shim_cert "$key_dir" shim_cert \
        "/CN=Shim Certificate/"
    ca_sign "$key_dir" vendor_cert "$key_dir" vendor_cert \
        "/CN=Vendor Certificate/"
}

create_system_user_key() {
    local key_dir="$SYSTEM_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" system_trusted_key "$key_dir" system_trusted_key \
        "/CN=System Trusted Certificate/"
}

create_modsign_user_key() {
    local key_dir="$MODSIGN_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" modsign_key "$key_dir" modsign_key \
        "/CN=MODSIGN Certificate/"
}

create_extra_system_user_key() {
    local key_dir="$EXTRA_SYSTEM_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" extra_system_trusted_key "$SYSTEM_KEYS_DIR" system_trusted_key \
        "/CN=Extra System Trusted Certificate/"
}

create_ima_user_key() {
    local key_dir="$IMA_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -p "$key_dir"

    ca_sign "$key_dir" x509_ima "$SYSTEM_KEYS_DIR" system_trusted_key \
        "/CN=IMA Trusted Certificate/" "enc"

    pem2der "$key_dir/x509_ima.crt"
    rm -f "$key_dir/x509_ima.crt"
}

create_rpm_user_key() {
    local gpg_ver=`gpg --version | head -1 | awk '{ print $3 }' | awk -F. '{ print $1 }'`
    local key_dir="$RPM_KEYS_DIR"

    [ ! -d "$key_dir" ] && mkdir -m 0700 -p "$key_dir"

    local gpg_key_name="SecureCore"
    local gpg_email="SecureCore@foo.com"

    if [ ! -z $GPG_KEYNAME ]; then
	    gpg_key_name=$GPG_KEYNAME
    fi

    if [ ! -z $GPG_EMAIL ]; then
	    gpg_email=$GPG_EMAIL
    fi

    local priv_key="$key_dir/RPM-GPG-PRIVKEY-$gpg_key_name"
    local pub_key="$key_dir/RPM-GPG-KEY-$gpg_key_name"

    if [ "$gpg_ver" == "2" ]; then
        gpg --homedir "$key_dir" --quick-generate-key --batch \
            "$gpg_key_name" default default never

        gpg --homedir "$key_dir" --export --armor "$gpg_key_name" > "$pub_key"

        gpg --homedir "$key_dir" --export-secret-keys --armor "$gpg_key_name" > "$priv_key"

        cd "$key_dir"
        rm -rf openpgp-revocs.d private-keys-v1.d pubring.kbx* \
            trustdb.gpg
        cd -
    else
        cat >"$key_dir/gen_rpm_keyring" <<EOF
Key-Type: RSA
Key-Length: 2048
Name-Real: $gpg_key_name
Name-Comment: RPM Signing Certificate
Name-Email: $gpg_email
Expire-Date: 0
%pubring $pub_key.pub
%secring $priv_key.sec
%commit
%echo RPM keyring $gpg_key_name created
EOF

        gpg --batch --gen-key "$key_dir/gen_rpm_keyring"

        gpg="gpg --no-default-keyring --secret-keyring \
            $priv_key.sec --keyring $pub_key.pub"

        $gpg --list-secret-keys

        print_error "Please type passwd to modify the passphrase, and type quit to exit"

        $gpg --edit-key "$gpg_key_name"

        $gpg --export --armor "$gpg_key_name" > "$pub_key"
        $gpg --export-secret-keys --armor "$gpg_key_name" > "$priv_key"

        rm -f "$key_dir/gen_rpm_keyring"
        rm -f "$priv_key.sec" "$pub_key.pub"
    fi
}

create_user_keys() {
    echo "Creating the user keys for UEFI Secure Boot"
    create_uefi_sb_user_keys

    echo "Creating the user keys for MOK Secure Boot"
    create_mok_sb_user_keys

    echo "Creating the user key for system"
    create_system_user_key

    echo "Creating the user key for system extra"
    create_extra_system_user_key

    echo "Creating the user key for modsign"
    create_modsign_user_key

    echo "Creating the user key for IMA appraisal"
    create_ima_user_key

    echo "Creating the user key for RPM"
    create_rpm_user_key
}

create_user_keys

