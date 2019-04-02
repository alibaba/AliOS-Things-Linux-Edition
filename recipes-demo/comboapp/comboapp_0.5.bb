SUMMARY = "Simple BLE/WiFi combo provision application"
DESCRIPTION = "A BLE/WiFi combo provision application."
SECTION = "examples"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

DEPENDS += "linkkit (>=3.0)"
DEPENDS += "combo (>=2.0)"
DEPENDS += "uota"

SRC_URI = "file://*.c \
           file://*.h \
           file://combo.init"

#SRC_URI += "file://linkkit_example_solo.c"

S = "${WORKDIR}"

# systemd init or sysvinit
#inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }

# only support sysvinit for now.
inherit update-rc.d

INITSCRIPT_NAME = "combo"
INITSCRIPT_PARAMS = "defaults 87"

# Here is a little demonstration application. In order to build your package, you may need to inherit
# scons/cmake/autotools bbclasses which are located in meta-yp/meta/classes according to the build
# system of your package.
do_compile() {
    ${CC} comboapp.c linkkit_example_solo.c cJSON.c ${LDFLAGS} -o comboapp -lcombo -lOTA -liot_sdk -liot_hal -lmbedtls -lmbedcrypto -lmbedx509 -lpthread -lrt
}

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${INIT_D_DIR}
    install -m 0755 ${B}/comboapp ${D}${bindir}
    install -m 0755 ${B}/combo.init ${D}${INIT_D_DIR}/combo
}
