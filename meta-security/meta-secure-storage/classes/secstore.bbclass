RDEPENDS_${PN} += " libsecstore"
DEPENDS += " libsecstore"

SECSTORE_USER_NAME ?= " root "

do_install_append () {
    install -d ${D}/secstore

    for uname in ${SECSTORE_USER_NAME}
    do
        install -d -m 0700 ${D}/secstore/$uname
        install -d ${D}/secstore/$uname/secret
        install -d ${D}/secstore/$uname/.secret
    done

}

FILES_${PN}_append += " /secstore/* "
