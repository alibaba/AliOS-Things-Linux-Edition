
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
	file://Module-ZU200 \
	file://Chat-Module-ZU200-connect \
	file://Chat-Module-ZU200-disconnect \
	"

do_install_append () {
    install -m 755 ${WORKDIR}/Module-ZU200 ${D}/etc/ppp/peers/
    install -m 755 ${WORKDIR}/Chat-Module-ZU200-connect ${D}/etc/ppp/peers/
    install -m 755 ${WORKDIR}/Chat-Module-ZU200-disconnect ${D}/etc/ppp/peers/
}
