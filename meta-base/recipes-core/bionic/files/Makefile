TARGET_ARCH ?= x86_64
TARGET_PREFIX ?= x86_64-linux-android
SYSROOT ?= /usr
DESTDIR ?= $(SYSROOT)/$(TARGET_PREFIX)
GCC_VERSION ?= 6.3.0
CC ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-gcc
CXX ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-g++
LD ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-ld
AS ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-as
AR ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-ar
STRIP ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-strip
OBJCOPY ?= $(SYSROOT)/bin/$(TARGET_PREFIX)-objcopy
GCC_INCLUDE ?= $(SYSROOT)/lib/gcc/$(TARGET_PREFIX)/$(GCC_VERSION)
GCC_LIB ?= $(SYSROOT)/lib/gcc/$(TARGET_PREFIX)/$(GCC_VERSION)
KERNEL_INCLUDE ?= $(SYSROOT)/$(TARGET_PREFIX)

ifeq ($(TARGET_ARCH),x86_64)
kernel_uapi_asm_dir := ./bionic/libc/kernel/uapi/asm-x86/asm
libm_machine_dir := ./bionic/libm/include/amd64/machine
TARGET_ARCH_CANONICAL := x86_64
endif

ifeq ($(TARGET_ARCH),aarch64)
kernel_uapi_asm_dir := ./bionic/libc/kernel/uapi/asm-arm64/asm
libm_machine_dir := ./bionic/libm/include/arm64/machine
TARGET_ARCH_CANONICAL := arm64
endif

SUBDIRS = \
	external/jemalloc \
	bionic/libdl \
	bionic/libc \
	bionic/libm \
	bionic/linker \
	system/core/base \

all: install-headers install-libs install-linker

install-headers: install-kernel-headers install-bionic-headers

install-kernel-headers:
	@cp -rf $(kernel_uapi_asm_dir) $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/asm-generic $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/drm $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/linux $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/mtd $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/rdma $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/scsi $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/sound $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/video $(DESTDIR)/usr/include
	@cp -rf ./bionic/libc/kernel/uapi/xen $(DESTDIR)/usr/include

install-bionic-headers:
	@cp -r ./bionic/libc/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/upstream-openbsd/android/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/upstream-openbsd/lib/libc/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/upstream-netbsd/android/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/upstream-netbsd/lib/libc/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/upstream-freebsd/android/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/dns/include $(DESTDIR)/usr
	@cp -r ./bionic/libc/arch-$(TARGET_ARCH_CANONICAL)/include $(DESTDIR)/usr
	@cp -r ./bionic/libm/include $(DESTDIR)/usr
	@cp -r $(libm_machine_dir) $(DESTDIR)/usr/include

	@$(MAKE) -C ./bionic/libc/ libc_crt TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB)
	@cp ./bionic/libc/crtbegin_dynamic.o $(DESTDIR)/usr/lib
	@cp ./bionic/libc/crtbegin_so.o $(DESTDIR)/usr/lib
	@cp ./bionic/libc/crtbegin_static.o $(DESTDIR)/usr/lib
	@cp ./bionic/libc/crtend_android.o $(DESTDIR)/usr/lib
	@cp ./bionic/libc/crtend_so.o $(DESTDIR)/usr/lib
	$(CC) -nostdlib -nostartfiles -shared -x c /dev/null -o $(DESTDIR)/usr/lib/libc.so

libs: jemalloc libc libm libdl

install-libs:
	@install -m 0644 ./bionic/libc/libc.a $(DESTDIR)/usr/lib
	@install -m 0644 ./bionic/libc/libc_nomalloc.a $(DESTDIR)/usr/lib
	@install -m 0755 ./bionic/libc/libc.so.1 $(DESTDIR)/usr/lib
	@ln -srf $(DESTDIR)/usr/lib/libc.so.1 $(DESTDIR)/usr/lib/libc.so
	@install -m 0644 ./bionic/libdl/libdl.a $(DESTDIR)/usr/lib
	@install -m 0755 ./bionic/libdl/libdl.so.1 $(DESTDIR)/usr/lib
	@ln -srf $(DESTDIR)/usr/lib/libdl.so.1 $(DESTDIR)/usr/lib/libdl.so
	@install -m 0644 ./bionic/libm/libm.a $(DESTDIR)/usr/lib
	@install -m 0755 ./bionic/libm/libm.so.1 $(DESTDIR)/usr/lib
	@ln -srf $(DESTDIR)/usr/lib/libm.so.1 $(DESTDIR)/usr/lib/libm.so

jemalloc:
	@$(MAKE) -C external/jemalloc TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB)

libc: jemalloc libdl
	@$(MAKE) -C bionic/libc TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB)

libdl:
	@$(MAKE) -C bionic/libdl TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB)

libm: libc
	@$(MAKE) -C bionic/libm TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB)

install-linker:
	@install -m 0755 ./bionic/linker/linker64 $(DESTDIR)/lib

	@ln -srf $(DESTDIR)/lib/linker64 $(DESTDIR)/lib/linker

libbase:
	@$(MAKE) -C system/core/base TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB) TARGET_NAME=$(TARGET_NAME)

linker:
	@$(MAKE) -C bionic/linker TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) OBJCOPY=$(OBJCOPY) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB) TARGET_NAME=$(TARGET_NAME)

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean TARGET_ARCH=$(TARGET_ARCH_CANONICAL) TARGET_PREFIX=$(TARGET_PREFIX) DESTDIR=$(DESTDIR) SYSROOT=$(SYSROOT) GCC_VERSION=$(GCC_VERSION) CC='$(CC)' CXX='$(CXX)' LD='$(LD)' AS=$(AS) AR=$(AR) STRIP=$(STRIP) GCC_INCLUDE=$(GCC_INCLUDE) GCC_LIB=$(GCC_LIB); \
	done
