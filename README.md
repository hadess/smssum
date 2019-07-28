# Sega Master System ROM Checksum Checker

This tool can be used to verify the validity of header checksums in
Sega Master System ROM dumps.

Note that some [Game Gear and Japanese releases tend to have invalid
values](http://www.smspower.org/Development/ROMHeader#Checksum7ffa2Bytes)
as their BIOSes did not check the validity of the checksum.

# Copyright and license

Based on [MSChecksumFixer](http://www.smspower.org/forums/16629-MasterSystemChecksumFixer)
by Daniel Hall (Dandaman955) but with the checksum writing removed, as we
don't want "corrected" versions showing up as those are usually bad dumps.

See COPYING file for license.
