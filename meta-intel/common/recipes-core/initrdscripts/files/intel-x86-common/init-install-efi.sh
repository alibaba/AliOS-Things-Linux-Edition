#!/bin/sh -e
#
# Copyright (c) 2016, Intel Corporation.
# All rights reserved.
#
# install.sh [device_name] [rootfs_name]
#
# This file is a copy of file with same name in OE:
# meta/recipes-core/initrdscripts/files/. We modify
# it for RMC feature to deploy file blobs from RMC
# database file to target.

PATH=/sbin:/bin:/usr/sbin:/usr/bin

# We need 20 Mb for the boot partition
boot_size=20

# 5% for swap
swap_ratio=5

# Get a list of hard drives
hdnamelist=""
live_dev_name=`cat /proc/mounts | grep ${1%/} | awk '{print $1}'`
live_dev_name=${live_dev_name#\/dev/}
# Only strip the digit identifier if the device is not an mmc
case $live_dev_name in
    mmcblk*)
    ;;
    nvme*)
    ;;
    *)
        live_dev_name=${live_dev_name%%[0-9]*}
    ;;
esac

echo "Searching for hard drives ..."

for device in `ls /sys/block/`; do
    case $device in
        loop*)
            # skip loop device
            ;;
        sr*)
            # skip CDROM device
            ;;
        ram*)
            # skip ram device
            ;;
        *)
            # skip the device LiveOS is on
            # Add valid hard drive name to the list
            case $device in
                $live_dev_name*)
                # skip the device we are running from
                ;;
                *)
                    hdnamelist="$hdnamelist $device"
                ;;
            esac
            ;;
    esac
done

if [ -z "${hdnamelist}" ]; then
    echo "You need another device (besides the live device /dev/${live_dev_name}) to install the image. Installation aborted."
    exit 1
fi

TARGET_DEVICE_NAME=""
for hdname in $hdnamelist; do
    # Display found hard drives and their basic info
    echo "-------------------------------"
    echo /dev/$hdname
    if [ -r /sys/block/$hdname/device/vendor ]; then
        echo -n "VENDOR="
        cat /sys/block/$hdname/device/vendor
    fi
    if [ -r /sys/block/$hdname/device/model ]; then
        echo -n "MODEL="
        cat /sys/block/$hdname/device/model
    fi
    if [ -r /sys/block/$hdname/device/uevent ]; then
        echo -n "UEVENT="
        cat /sys/block/$hdname/device/uevent
    fi
    echo
done

# Get user choice
while true; do
    echo "Please select an install target or press n to exit ($hdnamelist ): "
    read answer
    if [ "$answer" = "n" ]; then
        echo "Installation manually aborted."
        exit 1
    fi
    for hdname in $hdnamelist; do
        if [ "$answer" = "$hdname" ]; then
            TARGET_DEVICE_NAME=$answer
            break
        fi
    done
    if [ -n "$TARGET_DEVICE_NAME" ]; then
        break
    fi
done

if [ -n "$TARGET_DEVICE_NAME" ]; then
    echo "Installing image on /dev/$TARGET_DEVICE_NAME ..."
else
    echo "No hard drive selected. Installation aborted."
    exit 1
fi

device=/dev/$TARGET_DEVICE_NAME

#
# The udev automounter can cause pain here, kill it
#
rm -f /etc/udev/rules.d/automount.rules
rm -f /etc/udev/scripts/mount*

#
# Unmount anything the automounter had mounted
#
umount ${device}* 2> /dev/null || /bin/true

mkdir -p /tmp

# Create /etc/mtab if not present
if [ ! -e /etc/mtab ]; then
    cat /proc/mounts > /etc/mtab
fi

disk_size=$(parted ${device} unit mb print | grep '^Disk .*: .*MB' | cut -d" " -f 3 | sed -e "s/MB//")

swap_size=$((disk_size*swap_ratio/100))
rootfs_size=$((disk_size-boot_size-swap_size))

rootfs_start=$((boot_size))
rootfs_end=$((rootfs_start+rootfs_size))
swap_start=$((rootfs_end))

# MMC devices are special in a couple of ways
# 1) they use a partition prefix character 'p'
# 2) they are detected asynchronously (need rootwait)
rootwait=""
part_prefix=""
if [ ! "${device#/dev/mmcblk}" = "${device}" ] || \
[ ! "${device#/dev/nvme}" = "${device}" ]; then
    part_prefix="p"
    rootwait="rootwait"
fi
bootfs=${device}${part_prefix}1
rootfs=${device}${part_prefix}2
swap=${device}${part_prefix}3

echo "*****************"
echo "Boot partition size:   $boot_size MB ($bootfs)"
echo "Rootfs partition size: $rootfs_size MB ($rootfs)"
echo "Swap partition size:   $swap_size MB ($swap)"
echo "*****************"
echo "Deleting partition table on ${device} ..."
dd if=/dev/zero of=${device} bs=512 count=35

echo "Creating new partition table on ${device} ..."
parted ${device} mklabel gpt

echo "Creating boot partition on $bootfs"
parted ${device} mkpart boot fat32 0% $boot_size
parted ${device} set 1 boot on

echo "Creating rootfs partition on $rootfs"
parted ${device} mkpart root ext3 $rootfs_start $rootfs_end

echo "Creating swap partition on $swap"
parted ${device} mkpart swap linux-swap $swap_start 100%

parted ${device} print

echo "Formatting $bootfs to vfat..."
mkfs.vfat $bootfs

echo "Formatting $rootfs to ext3..."
mkfs.ext3 $rootfs

echo "Formatting swap partition...($swap)"
mkswap $swap

mkdir /tgt_root
mkdir /src_root
mkdir -p /boot

# Handling of the target root partition
mount $rootfs /tgt_root
mount -o rw,loop,noatime,nodiratime /run/media/$1/$2 /src_root
echo "Copying rootfs files..."
cp -a /src_root/* /tgt_root
if [ -d /tgt_root/etc/ ] ; then
    boot_uuid=$(blkid -o value -s UUID ${bootfs})
    swap_part_uuid=$(blkid -o value -s PARTUUID ${swap})
    echo "/dev/disk/by-partuuid/$swap_part_uuid                swap             swap       defaults              0  0" >> /tgt_root/etc/fstab
    echo "UUID=$boot_uuid              /boot            vfat       defaults              1  2" >> /tgt_root/etc/fstab
    # We dont want udev to mount our root device while we're booting...
    if [ -d /tgt_root/etc/udev/ ] ; then
        echo "${device}" >> /tgt_root/etc/udev/mount.blacklist
    fi
fi

# Handling of the target boot partition
mount $bootfs /boot
echo "Preparing boot partition..."

EFIDIR="/boot/EFI/BOOT"
mkdir -p $EFIDIR
# Copy the efi loader
cp /run/media/$1/EFI/BOOT/*.efi $EFIDIR

# RMC deployment
RMC_CMD=/src_root/usr/bin/rmc
RMC_DB=/run/media/$1/rmc.db

# We don't want to quit when a step failed. For example,
# a file system could not support some operations.
set +e

if [ -f "${RMC_DB}" ] && [ -f "${RMC_CMD}" ]; then
    echo "Found RMC database and tool, start RMC deployment"
    # query INSTALLER.CONFIG from RMC DB
    if ${RMC_CMD} -B INSTALLER.CONFIG -d "${RMC_DB}" -o /tmp/installer.config; then
	while IFS=':' read -r NAME TGT_UID TGT_GID TGT_MODE TGT_PATH; do
	    # skip comment
	    # The regexp in grep works with busybox grep which doesn't
	    # seem to have a -P to recognize '\t'. But this expression could not
	    # work with gnu grep...
	    if echo "$NAME"|grep -q $'^[ \t]*#'; then
		continue
	    fi
	    # check if we should create a directory (last char in target path is '/')
	    # or deploy a file
	    LAST_CHAR=$(echo "${TGT_PATH:$((${#TGT_PATH}-1)):1}")

	    # Do not bail out for failures but user should get stderr message
	    if [ ${LAST_CHAR} = "/" ]; then
		# name field is skipped for directory
		echo "DIR:  ${TGT_UID}:${TGT_GID}:${TGT_MODE} => ${TGT_PATH}"
		mkdir -p "$TGT_PATH"
		chown "${TGT_UID}:${TGT_GID}" "$TGT_PATH"
		chmod "${TGT_MODE}" "$TGT_PATH"
	    else
		${RMC_CMD} -B "${NAME}" -d "${RMC_DB}" -o "${TGT_PATH}"
		echo "FILE: ${NAME}:${TGT_UID}:${TGT_GID}:${TGT_MODE} => ${TGT_PATH}"
		chown "${TGT_UID}:${TGT_GID}" "$TGT_PATH"
		chmod "${TGT_MODE}" "$TGT_PATH"
	    fi
	done < /tmp/installer.config
	rm -rf /tmp/installer.config

	# remove rmc from target since we don't think it is a valid
	# case to run rmc after installation.
	rm -rf /tgt_root/usr/bin/rmc
	echo "RMC deployment finished"
    else
	echo "INSTALLER.CONFIG is not found, skip RMC deployment"
    fi

    # Final retouching by calling post-install hook
    if ${RMC_CMD} -B POSTINSTALL.sh -d "${RMC_DB}" -o /tmp/POSTINSTALL.sh; then
        echo "Found POSTINSTALL.sh execute it..."
        chmod 500 /tmp/POSTINSTALL.sh
        /tmp/POSTINSTALL.sh
        rm -rf /tmp/POSTINSTALL.sh
    fi
fi
set -e

if [ -f /run/media/$1/EFI/BOOT/grub.cfg ]; then
    root_part_uuid=$(blkid -o value -s PARTUUID ${rootfs})
    GRUBCFG="$EFIDIR/grub.cfg"
    cp /run/media/$1/EFI/BOOT/grub.cfg $GRUBCFG
    # Update grub config for the installed image
    # Delete the install entry
    sed -i "/menuentry 'install'/,/^}/d" $GRUBCFG
    # Delete the initrd lines
    sed -i "/initrd /d" $GRUBCFG
    # Delete any LABEL= strings
    sed -i "s/ LABEL=[^ ]*/ /" $GRUBCFG
    # Delete any root= strings
    sed -i "s/ root=[^ ]*/ /g" $GRUBCFG
    # Add the root= and other standard boot options
    sed -i "s@linux /vmlinuz *@linux /vmlinuz root=PARTUUID=$root_part_uuid rw $rootwait quiet @" $GRUBCFG
fi

if [ -d /run/media/$1/loader ]; then
    rootuuid=$(blkid -o value -s PARTUUID ${rootfs})
    GUMMIBOOT_CFGS="/boot/loader/entries/*.conf"
    if [ -d /boot/loader ]; then
        # Don't override loader.conf RMC already deployed
        if [ ! -f /boot/loader/loader.conf ]; then
            cp /run/media/$1/loader/loader.conf /boot/loader/
        fi
        # only copy built OE entries when RMC entries don't exist.
        if [ ! -d /boot/loader/entries ] || [ ! ls /boot/loader/entries/*.conf &>/dev/null ]; then
            cp -dr /run/media/$1/loader/entries /boot/loader
        fi
    else
        # copy config files for gummiboot
        cp -dr /run/media/$1/loader /boot
        # delete the install entry
        rm -f /boot/loader/entries/install.conf
    fi
    # delete the initrd lines
    sed -i "/initrd /d" $GUMMIBOOT_CFGS
    # delete any LABEL= strings
    sed -i "s/ LABEL=[^ ]*/ /" $GUMMIBOOT_CFGS
    # delete any root= strings
    sed -i "s/ root=[^ ]*/ /" $GUMMIBOOT_CFGS
    # add the root= and other standard boot options
    sed -i "s@options *@options root=PARTUUID=$rootuuid rw $rootwait quiet @" $GUMMIBOOT_CFGS
    # if RMC feature presents, append global kernel command line fragment when it exists.
    if [ -f "${RMC_DB}" ] && [ -f "${RMC_CMD}" ]; then
	if ${RMC_CMD} -B KBOOTPARAM -d "${RMC_DB}" -o /tmp/kbootparam; then
	    sed -i "/^[ \t]*options/ s/$/ $(cat /tmp/kbootparam)/" $GUMMIBOOT_CFGS
	    rm /tmp/kbootparam
	fi
    fi
fi

cp /run/media/$1/vmlinuz /boot

umount /src_root
umount /tgt_root
umount /boot

sync

echo "Remove your installation media, and press ENTER"

read enter

echo "Rebooting..."
reboot -f
