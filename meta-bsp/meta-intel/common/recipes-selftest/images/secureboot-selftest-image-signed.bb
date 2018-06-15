require secureboot-selftest-image-unsigned.bb

IMAGE_FEATURES += "secureboot"

SECURE_BOOT_SIGNING_KEY ?= "${THISDIR}/files/refkit-db.key"
SECURE_BOOT_SIGNING_CERT ?= "${THISDIR}/files/refkit-db.crt"
