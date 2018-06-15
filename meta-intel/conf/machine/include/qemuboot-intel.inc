# For runqemu
IMAGE_CLASSES += "qemuboot"
QB_SYSTEM_NAME_intel-core2-32 = "qemu-system-i386"
QB_CPU_intel-core2-32 = "-cpu coreduo"
QB_CPU_KVM_intel-core2-32 = "-cpu kvm32"

QB_SYSTEM_NAME_intel-corei7-64 = "qemu-system-x86_64"
QB_CPU_intel-corei7-64 = "-cpu Nehalem"
QB_CPU_KVM_intel-corei7-64 = "-cpu kvm64"

QB_SYSTEM_NAME_intel-quark = "qemu-system-i386"
QB_CPU_intel-quark = "-cpu coreduo"
QB_CPU_KVM_intel-quark = "-cpu kvm32"

QB_AUDIO_DRV = "alsa"
QB_AUDIO_OPT = "-soundhw ac97,es1370"
QB_KERNEL_CMDLINE_APPEND = "vga=0 uvesafb.mode_option=640x480-32 oprofile.timer=1 uvesafb.task_timeout=-1"
# Add the 'virtio-rng-pci' device otherwise the guest may run out of entropy
QB_OPT_APPEND = "-vga vmware -show-cursor -usb -usbdevice tablet -device virtio-rng-pci"
