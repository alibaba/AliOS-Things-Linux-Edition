RDEPENDS_${PN}_append_rpi = " rpi-u-boot-scr"

require ${@oe.utils.all_distro_features(d, 'uota', 'u-boot-uota.inc')}

