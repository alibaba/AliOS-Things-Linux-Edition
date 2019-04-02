SUMMARY = "Intel Open VINO"
LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://opt/intel/computer_vision_sdk_fpga_2018.2.300/licensing/EULA.txt;md5=b11907ad3c94e21bc5706c0c7d271b3e"

SRC_URI = "file://intel-cv-sdk-full-documentation-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-documentation-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-shared-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-shared-pset-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-setupvars-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-gfx-install-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-opencl-2018ww15-010713.x86_64-igdrcl.rpm;subdir=${BP};name=openvino-rpm \
file://intel-opencl-cpu-r5.0-63503.x86_64.rpm;subdir=${BP};name=openvino-rpm \
file://intel-opencl-devel-r5.0-63503.x86_64.rpm;subdir=${BP};name=openvino-rpm \
file://intel-opencl-r5.0-63503.x86_64.rpm;subdir=${BP};name=openvino-rpm \
file://aocl-pro-rte-17.1.2-304.x86_64.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-models-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-model-optimizer-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-inference-engine-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-media-stack-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://opencl-headers-2.2-1.20180306gite986688.el6.noarch.rpm;subdir=${BP};name=openvino-rpm \
file://intel-cv-sdk-full-l-ocv-yocto-300-2018.0-300.noarch.rpm;subdir=${BP};name=openvino-rpm"

DEPENDS += "libva numactl ffmpeg  libpng cairo pango glib-2.0 gtk+3 libusb gstreamer1.0"
RDEPENDS_${PN} += " wayland perl bash"
RDEPENDS_${PN}-samples += " perl"

INSANE_SKIP_${PN} = " ldflags already-stripped dev-so staticdev"
INSANE_SKIP_${PN}-plugins = " ldflags"

inherit bin_package

PACKAGE_BEFORE_PN = "${PN}-samples ${PN}-plugins"

SYSROOT_PREPROCESS_FUNCS += "msdk_populate_sysroot"
msdk_populate_sysroot() {
        sysroot_stage_dir ${D}/opt ${SYSROOT_DESTDIR}/opt
}
addtask wa_handle_unpack after do_unpack before do_patch
do_wa_handle_unpack() {
        echo "do task wa_handle_unpack"
        sed -i "s/INSTALLDIR=<INSTALLDIR>/INSTALLDIR=\/opt\/intel\/computer_vision_sdk_fpga_2018.2.300\//g" ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/bin/setupvars.sh
        sed -i "s/poky 2.0/AliOS 1.3/g" ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/inference_engine/share/InferenceEngineConfig.cmake
        sed -i "s/sudo -E $pip_binary/$pip_binary/g" ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/demo/demo_squeezenet_download_convert_run.sh
	sed "91 apython_binary=python3" -i ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/demo/demo_squeezenet_download_convert_run.sh
	sed "92 apip_binary=pip3" -i ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/demo/demo_squeezenet_download_convert_run.sh
	sed "76 apython_binary=python3" -i ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/model_optimizer/install_prerequisites/install_prerequisites.sh
	sed "44 aIE_PLUGINS_PATH=\$INTEL_CVSDK_DIR/deployment_tools/inference_engine/lib/ubuntu_16.04/intel64" -i ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/bin/setupvars.sh
	rm -rf ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/inference_engine/tools/centos_7.*/
	rm -rf ${WORKDIR}/${BPN}-${PV}/opt/intel/computer_vision_sdk_fpga_2018.2.300/deployment_tools/inference_engine/lib/centos_7.*/
}
