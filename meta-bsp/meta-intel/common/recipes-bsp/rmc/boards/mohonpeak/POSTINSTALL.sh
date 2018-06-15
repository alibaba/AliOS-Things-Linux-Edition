# Mohonpeak uses only S1 for serial, so remove S0 and S2
sed -i '/start_getty.\+ttyS0/d' /tgt_root/etc/inittab
sed -i '/start_getty.\+ttyS2/d' /tgt_root/etc/inittab
