DESCRIPTION = "IMX SERIAL RS485 TOOL"
LICENSE = "CLOSED"

SRC_URI = "file://imx_rs485.c"

S = "${WORKDIR}"

do_compile() {
	${CC} imx_rs485.c -o imx_rs485
}

do_install() {
	install -d ${D}/usr/local/bin
	install -m 755 imx_rs485 ${D}/usr/local/bin
}

INSANE_SKIP_${PN} = "ldflags"

FILES_${PN} += "/usr/local/bin"
FILES_${PN}-dbg += "/usr/local/bin/.debug"

