RPIFW_DATE ?= "20180417"
SRCREV ?= "5db8e4e1c63178e200d6fbea23ed4a9bf4656658"
RPIFW_SRC_URI ?= "https://github.com/raspberrypi/firmware/archive/1.${RPIFW_DATE}.tar.gz"
RPIFW_S ?= "${WORKDIR}/firmware-1.${RPIFW_DATE}"

SRC_URI = "${RPIFW_SRC_URI}"
SRC_URI[md5sum] = "db894ff6b2538b0bb03ab7b6a8fc4116"
SRC_URI[sha256sum] = "aa3b7dfc9760c4be47f23a6210b6b989cf95b92e695dd4682facccd6c712c3a5"

PV = "${RPIFW_DATE}"

