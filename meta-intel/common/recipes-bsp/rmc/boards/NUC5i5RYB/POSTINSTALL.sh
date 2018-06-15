# There is no tty device on this board.
sed -i '/start_getty.\+ttyS.*/d' /tgt_root/etc/inittab
