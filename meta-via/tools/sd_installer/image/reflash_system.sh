#! /bin/sh

clear

# enable recovery function
enable_recovery=0

# device on which the system is installed.
emmc_dev="/dev/mmcblk0"

# device on which the bootloader is installed.
spi_dev="/dev/mtdblock0"

# path for all re-flash data on the SD card
sdcard_path="mmcblk1p2"

delete_all_partitions()
{
	local device="$1"
	local i=1

	if [ ! -e "$device" ]; then
		echo "$device doesn't exist ..."
		return
	fi

	while [ 1 ]; do
		partitions=`echo p | fdisk $device | egrep ""$device"p[0-9]+" | wc -l`
		if [ $partitions -eq 0 ]; then
			break
		fi

		echo "delete "$device"p$i .."
		printf "d\n$i\nw\n" | fdisk $device && sync

		i=`expr $i \+ 1`
	done

	sync
}

reprobe_partitions()
{
	echo "re-probe partitions ..."
	partprobe
	sync
}

main()
{
	#local i=0
	
	echo "Start to install the system..."
	
	if [ ! -e "$emmc_dev" ]; then
		echo "$emmc_dev doesn't exist! Abort!"
		return
	fi

	if [ ! -e "$spi_dev" ]; then
		echo "$spi_dev doesn't exist! Abort!"
		return
	fi

	# umount any mounted parititions.
	(mount | grep $emmc_dev > /dev/null) && umount -f -r $(mount | grep $emmc_dev | awk '{print $1}') && sync

	# delete any old partitions.
	delete_all_partitions "$emmc_dev"

	# re-probe the partitions.	
	reprobe_partitions

	# =========================================
	# create MBR.
	echo "Create MBR..."
	# boot
	printf "n\np\n1\n\n+16M\np\nw\n" | fdisk $emmc_dev && sync
	# rootfs
	printf "n\np\n2\n\n+2048M\np\nw\n" | fdisk $emmc_dev && sync

	if [ $enable_recovery == "1" ]; then
		# recovery
		printf "n\np\n3\n\n+16M\np\nw\n" | fdisk $emmc_dev && sync
		# extended
		printf "n\ne\n4\n\n\np\nw\n" | fdisk $emmc_dev && sync
		# image
		printf "n\n\n+1024M\np\nw\n" | fdisk $emmc_dev && sync
		# the remaining space
		printf "n\n\n\np\nw\n" | fdisk $emmc_dev && sync
	else
		# extended
		printf "n\np\n3\n\n\np\nw\n" | fdisk $emmc_dev && sync
	fi


	reprobe_partitions

	if [ $enable_recovery == "1" ]; then
		partition_list="\
			boot:"$emmc_dev"p1:vfat:/mnt/boot/ \
			rootfs:"$emmc_dev"p2:ext4:/mnt/rootfs/ \
			recovery:"$emmc_dev"p3:vfat:/mnt/recovery/ \
			image:"$emmc_dev"p5:vfat:/mnt/image/ \
			data:"$emmc_dev"p6:ext4:/mnt/data/"
	else
		partition_list="\
			boot:"$emmc_dev"p1:vfat:/mnt/boot/ \
			rootfs:"$emmc_dev"p2:ext4:/mnt/rootfs/ \
			data:"$emmc_dev"p3:ext4:/mnt/data/"
	fi

	# =========================================
	# format each partition.
	for entry in $partition_list; do
		name=`echo $entry | cut -d: -f1`
		dev=`echo $entry | cut -d: -f2`
		fmt=`echo $entry | cut -d: -f3`

		echo "format $dev, name: $name, fs type: $fmt"

		if [ "$fmt" == "vfat" ]; then
			mkfs.vfat -n $name -S 512 $dev
		elif [ "$fmt" == "ext3" ]; then
			mkfs.ext3 -F -L $name $dev
		elif [ "$fmt" == "ext4" ]; then
			mkfs.ext4 -F -L $name $dev
		fi
		
		sync		
	done
	sync

	# =========================================
	# get started to reflash the system images.	
	for entry in $partition_list; do
		dev=`echo $entry | cut -d: -f2`
		fmt=`echo $entry | cut -d: -f3`
		mnt=`echo $entry | cut -d: -f4`

		(mount | grep $mnt > /dev/null) && umount $mnt

		# mount the partitions.
		echo "mount $dev to $mnt, format $fmt"
		mkdir -p $mnt && sync
		mount -t $fmt $dev $mnt || exit 1
		sync
	done
	
	if [ -d "/mnt/$sdcard_path/boot" ]; then
		# boot partition.
		for f in `find /mnt/$sdcard_path/boot/ -maxdepth 1 -type f`; do
			src="$f"
			dst="/mnt/boot/"
			echo "copying $src to $dst ..." || exit 1
			cp -rf $src $dst
			sync
		done
	else
		echo "/mnt/$sdcard_path/boot directory doesn't exist! Abort!"
		exit 1
	fi

	if [ -f "/mnt/$sdcard_path/u-boot.bin" ]; then
		# u-boot
		echo "copying /mnt/$sdcard_path/u-boot.bin to $spi_dev ..."

		dd if=/dev/zero of=$spi_dev bs=512 count=2048 2>&1 > /dev/null
		sync

		dd if=/mnt/$sdcard_path/u-boot.bin of=$spi_dev bs=512 seek=2
		sync
	else
		echo "/mnt/$sdcard_path/u-boot.bin doesn't exist! Abort!"
		exit 1
	fi

	# rootfs
	if [ -f "/mnt/$sdcard_path/rootfs.tgz" ]; then
		echo "untar /mnt/$sdcard_path/rootfs.tgz to /mnt/rootfs/ ..."
		tar xf /mnt/$sdcard_path/rootfs.tgz -C /mnt/rootfs/
		sync
	else
		echo "/mnt/$sdcard_path/rootfs.tgz doesn't exist! Abort!"
		exit 1
	fi
	
	# copy bin ver
	if [ -f "/mnt/$sdcard_path/viabin_ver" ]; then		
		echo "copying /mnt/$sdcard_path/viabin_ver to /mnt/rootfs/etc/ ..."
		cp /mnt/$sdcard_path/viabin_ver /mnt/rootfs/etc/viabin_ver
		sync
#	else
#		echo "/mnt/$sdcard_path/viabin_ver doesn't exist! Abort!"
#		exit 1
	fi


	if [ $enable_recovery == "0" ]; then
		echo ""
	elif [ -d "/image/recovery" ]; then
		# recovery partition.
		for f in `find /image/boot/ /image/recovery/ -maxdepth 1 -type f`; do
			src="$f"
			dst="/mnt/recovery/"
			echo "copying $src to $dst ..." || exit 1
			cp -rf $src $dst || exit 1
			sync
		done
	else
		echo "/recovery directory doesn't exist! Abort!"
		exit 1
	fi

	if [ $enable_recovery == "1" ]; then
		# image partition.
		image_part_items="\
			/image/boot/* \
			/image/u-boot.bin \
			/image/rootfs.tgz"

		for entry in $image_part_items; do
			src="$entry"
			dst="/mnt/image/"
			echo "copying $src to $dst ..."
			cp -rf $src $dst && sync || exit 1
		done
	fi
	
	#umount -f -r "$emmc_dev"p*
	#sync
	
	sync
	
	#echo "Please remove the SD card and power on the system!"
	#poweroff
	echo -e "Finish. All successed.\n"
}
main $@
