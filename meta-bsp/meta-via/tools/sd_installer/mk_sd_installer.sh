#! /bin/bash

# change log 
# 20160722 - support ubuntu 10.04


# $ sudo ./mk_sd_installer.sh /dev/sdx [--yocto]
#
# - /dev/sdx is the device of the SD card.
# - use "--yocto" option to make yocto SD card.

clear
local_path=`pwd -P`

colored_print()
{
	local color="$1"
	local msg="$2"
	local code=""

	if [ "$color" == "red" ]; then
		code='0;31'
	elif [ "$color" == "green" ]; then
		code='0;32'
	elif [ "$color" == "yellow" ]; then
		code='1;33'
	elif [ "$color" == "blue" ]; then
		code='0;34'
	else
		echo "unsupported color!"
		return
	fi

	printf "\033["$code"m$msg\033[0m"
}

create_sd_installer()
{
	local sd_dev="$1"
	local sd_opt="$2"
	local i=0
	local tmpdir=`mktemp -d`

	# umount partitions.
	umount "$sd_dev"* && sync

	colored_print yellow "start creating the SD card installer....\n"
	
	# delete old partitions.
	i=1
	while [ 1 ]; do
		partitions=`echo p | fdisk $sd_dev | egrep "$sd_dev[0-9]+" | wc -l`
		if [ $partitions -eq 0 ]; then
			break
		fi
		
		colored_print yellow "delete partition $i\n"
		printf "d\n$i\np\nw\n" | fdisk $sd_dev && sync
		i=`expr $i \+ 1`
	done

	# create partition table.
#	printf "n\np\n\n\n+32M\np\nw\n" | fdisk -u $sd_dev
#	printf "n\np\n\n\n\np\nw\n" | fdisk -u $sd_dev
	printf "2048,65536,L\n67584,,L\n" | sfdisk $sd_dev -uS --force
	sync

	partprobe
	sync

	# 1st is boot partion in fat32 format.
	mkfs.vfat -n boot -S 512 "$sd_dev"1
	# 2nd is image partition in ext4 format.
	mkfs.ext4 -F -L rootfs "$sd_dev"2
	sync

	mkdir -p $tmpdir/boot && mount -t vfat "$sd_dev"1 $tmpdir/boot || exit
	mkdir -p $tmpdir/image && mount -t ext4 "$sd_dev"2 $tmpdir/image || exit
	sync

	# the kernel for installer with SD card.
#	cp -r ./image/boot/zImage $tmpdir/boot/zImage.installer && sync || exit
	cp -r ./image/boot/zImage $tmpdir/boot/zImage && sync || exit
	
	# device tree for installer with SD card.
	items=`find ./image/boot/ -type f -name "*.dtb" | tr '\n' ' '`
	for f in $items; do
		src="$f"
		dst="$tmpdir/boot/"

		if [ ! -e "$src" ]; then
			colored_print red "$src doesn't exist! Abort!\n"
			exit
		fi

		colored_print yellow "copy $src to $dst .. \n"
#		cp -arfL --no-preserve=ownership $src $dst
		cp -r $src $dst
		sync
	done

	# rootfs
	if [ "$sd_opt" != "--yocto" ]; then
		#colored_print yellow "untar the installer rootfs..\n"
		#tar xf installer_fs.tgz -C $tmpdir/image || exit

		# build ramdisk
		colored_print yellow "build ramdisk..\n"
		mkdir -p $tmpdir/ramdisk
        tar xf image/rootfs.tgz -C $tmpdir/ramdisk || exit
        cd $tmpdir/ramdisk
        ln -s sbin/init init
        find . | cpio -o -H newc | gzip -9 > $local_path/image/initrd.img.gz
        cd $local_path

		sync
	else
		colored_print yellow "untar the yocto rootfs..\n"
		tar xf image/rootfs.tgz -C $tmpdir/image || exit
		sync
		
		# copy bin version
		if [ -f image/viabin_ver ]; then
			colored_print yellow "copy viabin_ver...\n"
			cp image/viabin_ver $tmpdir/image/etc/viabin_ver || exit
			sync
		fi
	fi

	# u-boot
	colored_print yellow "copy the u-boot...\n"
	dd if=image/u-boot.bin of=$sd_dev bs=512 seek=2 || exit
	sync
	
	# copy u-boot commands for ramdisk
	if [ "$sd_opt" != "--yocto" ]; then
		colored_print yellow "copy boot.scr ...\n"
		cp ./boot.scr $tmpdir/boot/boot.scr && sync || exit
	fi

	# images
	if [ "$sd_opt" != "--yocto" ]; then
		colored_print yellow "copy the images for system...\n"
		cp -arf $local_path/image/* $tmpdir/image/
		sync
	fi

	umount -f -r $tmpdir/boot
	umount -f -r $tmpdir/image
	rm -fr $tmpdir
	sync

	eject $sd_dev
	sync

	colored_print yellow "Done! The SD card can be removed now!\n"
	sync
}

main()
{
	local sd_dev="$1"
	local sd_opt="$2"
	local confirm=""

	if [ -z "$sd_dev" ]; then
		colored_print yellow "usage: $0 /dev/sdx\n"
		colored_print yellow "\t --- where /dev/sdx is the device of the SD card.\n"
		return;
	fi

	colored_print yellow "The specified device is '$sd_dev', it will be formated! "
	read -p "Continue? (y/n) " confirm
	if [ "$confirm" == "n" ] || [ "$confirm" == "N" ]; then
		colored_print red "Abort!\n"
		return
	elif [ "$confirm" == "y" ] || [ "$confirm" == "Y" ]; then
		create_sd_installer $sd_dev $sd_opt
	else
		colored_print red "unrecognized input!\n"
	fi

	sync
}

main $@

