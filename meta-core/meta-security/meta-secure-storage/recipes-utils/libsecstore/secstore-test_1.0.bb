SUMMARY = " Secure store tests"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = " \
           file://secstore-test-1.0 \
          "

inherit secstore useradd

CFLAGS += "-DSECSTORE_DATA_MAX_LENGTH=65536"

do_configure () {
    :
}

do_compile () {
    oe_runmake
}

do_install () {
    install -d ${D}${bindir}
    oe_runmake install 'DESTDIR=${D}'
}

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM_${PN} = "-r g1; \
                        -r g2"
USERADD_PARAM_${PN} = " -r -g g1 ssuser1; \
                        -r -g g1 ssuser2; \
                        -r -g g2 ssuser3"

SECSTORE_USER_NAME += "ssuser1 ssuser2 ssuser3"
