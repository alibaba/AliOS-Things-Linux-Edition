<p align ="center"><img src=logo-big.svg /></p>  

# Micro-iot
Yocto based Linux distribution for Internet of Things capable low profile SBCs

### Supported modules:
1. SeeedStudio LinkitSmart MT7688 (Mediatek MT7688AN chipset with MIPS24KEc core)

### Project goals and principles:
- transparency
- simplicity (KISS & YAGNI)
- quality

### Features
- Linux kernel 4.9.73 (rev. b3e88217e2f95b004da89a0ff931e1dc45d3d094) with OpenWRT patches (OpenWRT rev.
  c5ca1c9ab65bfe1e6fc74230f8c0121230562b1c)
- JFFS2 filesystem image
- flashing via MT7688 u-boot
- OpenWRT backported Wifi drivers and MT76 driver compiled out-of-tree
- tested with Yocto Release Rocko (rev. 16e22f3e37788afb83044f5089d24187d70094bd)

### Quick Start  
git clone git://git.yoctoproject.org/poky  
git checkout 16e22f3e37788afb83044f5089d24187d70094bd  
cd poky  
git clone https://github.com/micro-iot/micro-iot.git  
cd micro-iot  
mv meta-linkit7688 ..  
mv build ..  
mv meta-micro-iot ..  
cd ..  
. oe-init-build-env build  
bitbake micro-iot-basic-image  
bitbake -c prepare_sysupgrade micro-iot-basic-image  
