# RMC database bbclass
# provide functions to generate RMC database file on build host (native)

DEPENDS += "rmc-native"

# rmc_generate_db()
# $1: a list of directories. Each directory holds directories for a group of
# boards.
# $2: path_name of rmc generates database file and records
#
# WARNING: content of directory of database file will be removed.
#
# Each board directory shall contain a fingerprint file (*.fp) at least, with
# optional file blob(s) associated to the type of board. If a board directory
# has no file blob, no record is created for that board.
#
# An example of two directories each of which contains two boards for RMC:
# (All file and directory names are for illustration purpose.)
#
# dir_1/
#     board_1/
#         board_1_fingerprint.fp
#         file_1.blob
#     board_2/
#         board_2.fp
# dir_2/
#     board_3/
#         b3.fp
#         file_1.blob
#         file_2.conf
#     board_4/
#         board_foo.fp
#         mylib.config
#
# To generate a RMC database "rmc.db" with data of all (actually 3) of boards in
# a directory "deploy_dir":
#
# rmc_generate_db "dir_1 dir_2" "deploy_dir/rmc.db"
#
# The board_2 will be skipped. No record or any data for it is packed in
# generated database because it only contains a fingerprint file.
#

rmc_generate_db () {
	RMC_BOARD_DIRS=$1

	if [ "$#" -ne 2 ]; then
		echo "rmc_generate_db(): Wrong number of arguments: $#"
		return 1
	fi

	RMC_DB_DIR=$(dirname "$2")
	RMC_RECORDS=""

	rm -rf ${RMC_DB_DIR}
	mkdir -p ${RMC_DB_DIR}

	# generate rmc database
	for topdir in ${RMC_BOARD_DIRS}; do
		# For all board dirs in a topdir:
		CUR_BOARD_DIRS=$(find ${topdir}/* -type d)
		for board_dir in ${CUR_BOARD_DIRS}; do
			CUR_FINGERPRINT=$(find ${board_dir}/ -name "*.fp")

			# disallow a board directory without any fingerprint file in it.
			if [ -z "${CUR_FINGERPRINT}" ]; then
				echo "Cannot find RMC fingerprint file in ${board_dir}"
				return 1
			fi

			CUR_FILES=$(find ${board_dir}/ -type f |grep -v '\.fp$' || true)

			# allow a directory only with fingerprint file. Developer may
			# check in fingerprint for future use.
			if [ -z "${CUR_FILES}" ]; then
				continue
			fi

			for fp in ${CUR_FINGERPRINT}; do
				fullname=$(basename ${fp})
				CUR_TAG="${fullname%.*}"
				CUR_RECORD=${RMC_DB_DIR}/${CUR_TAG}.rec
				rmc -R -f ${fp} -b ${CUR_FILES} -o ${CUR_RECORD}
				RMC_RECORDS="${RMC_RECORDS} ${CUR_RECORD}"
			done
		done
	done

	if [ ! -z "${RMC_RECORDS}" ]; then
		rmc -D ${RMC_RECORDS} -o "$2"
	fi
}
