# NUC Gen 6 specific retouch after RMC deployment

# The generated inittab from OE build causes error messages:
# "auth.err getty[615]: tcgetattr: Input/output error"
# in /var/log/messages because NUC Gen 6 doesn't have any
# serial tty. We delete line(s) here on target.
sed -i '/start_getty.\+ttyS.*/d' /tgt_root/etc/inittab
