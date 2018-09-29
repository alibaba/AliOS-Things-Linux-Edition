SHELL=bash
V=1
ifndef V
Q=@
ECHO=@echo -e
else
Q=
ECHO=@\#
endif
XCC = ${CC}
XLD = ${LD}
XOC = ${OBJCOPY}
XNM = ${NM}
XOD = ${OBJDUMP}
XAR = ${AR}
XDB = $(CROSS_PREFIX)gdb
LZMA:=lzma-4.32
LZO:=lzop

# RULES
%.lzo: %
	$(ECHO) "\t[LZO] $@"
	$(Q)$(LZO) -9 -f $<

%.lzma: %
	$(ECHO) "\t[LZM] $@"
	$(Q)$(LZMA) -9 -c -k $< > $@

%.o: %.c
	$(ECHO) "\t [CC] $@"
	$(Q)$(XCC) -c -o $@ $(FGS) $<

%.o: %.S
	$(ECHO) "\t[ASM] $@"
	$(Q)$(XCC) -c -o $@ $(FGS) $<

%.d: %.c
	@#$(ECHO) "\t[DEP] $@"
	$(Q)$(XCC) $(FGS) -MM -MT $(<:.c=.o) $< > $@
	
%.d: %.S
	@#$(ECHO) "\t[DEP] $@"
	$(Q)$(XCC) $(FGS) -MM -MT $(<:.S=.o) $< > $@

%.bin: %
	$(ECHO) "\t[BIN] $@"
	$(Q)$(XOC) -R .MIPS.abiflags -R .reginfo -O binary $< $@

%.map: %
	$(ECHO) "\t[MAP] $@"
	$(Q)$(XNM) $< | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)' | sort > $@

%.dis: %
	$(XOD) -S --show-raw-insn $< > $@

%.dis: %.o
	$(XOD) -S --show-raw-insn $< > $@

ALL_OBJS=$(B1_OBJS) $(B2_OBJS)
ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
