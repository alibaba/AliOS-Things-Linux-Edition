# Joule uses only S2 for serial, so remove S0
sed -i '/start_getty.\+ttyS0/d' /tgt_root/etc/inittab
