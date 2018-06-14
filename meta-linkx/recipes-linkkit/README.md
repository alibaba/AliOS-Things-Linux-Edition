## build linkkit
### Overview
* get source code from github master branch: [AliOS-Things](https://github.com/alibaba/AliOS-Things)
* alios-things lastest release version not support linkkit，so i choose the master branch
* for now,recipes-linkkit support `qemux86`、`qemux86-64`、`raspberrypi3`
* for `qemux86-64` and `raspberrypi3` recipes-linkkit will applying patches ，`qemux86` will not.
* alios-things use aos-cube to compile source code，so need to build python-aos-cube-native
* another way, modify `TARGET_APP` in `linkkit.inc` can compile other app
### Directory description
|Directory|description|
|--|--|
|`linkkit`|linkkit bb files|
|`linkkit/patches`|patches|
|`python-aos-cube`|aos-cube and the depend-packages bb files|
### Quick Start
#### configure envirenment
```sh
. ./oe-init-build-env
```
#### build linkkit
```sh
bitbake linkkit
```
### output
#### rpm-packages
```
build/tmp/work/${PACKAGE_ARCH}-poky-linux/linkkit/1.2.2-r1/deploy-rpms/${PACKAGE_ARCH}
```
PACKAGE_ARCH：
|machine|PACKAGE_ARCH|
|--|:--|
|qemux86|`i586`|
|qemux86-64|`core2_64`|
|raspberrypi3|`cortexa7hf_neon_vfpv4`|
#### elf files
```
build/tmp/work/${PACKAGE_ARCH}-poky-linux/linkkit/1.2.2-r1/image/usr/bin/linkkitapp@linuxhost
```
> contact chenan.xxw for more details