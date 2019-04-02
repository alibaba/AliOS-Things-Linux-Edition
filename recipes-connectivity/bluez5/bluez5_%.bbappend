# Create a small bluez5 package which only includes several utilities needed by WiFi
# configuration via bluetooth.

PACKAGES_prepend = "${PN}-small "

FILES_${PN}-small = " \
    ${bindir}/hciconfig \
    ${bindir}/hciattach \
    ${bindir}/hcidump \
    "

