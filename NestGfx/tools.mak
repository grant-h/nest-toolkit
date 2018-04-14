# Requires that Nest has dropbear with scp installed
# Also dropbear has to allow no password from root
# or anything to enable password-less SSH
NESTIP=192.168.1.141
NESTUSER=root

# allow tools to use these
export NESTIP
export NESTUSER

# misc tools
#TOOLS=$(CURDIR)/tools
SCP=/usr/bin/scp
CMD=$(TOOLS)/cmd
TESTMODE=$(TOOLS)/testmode
