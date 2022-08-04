# mmdb2dat
After Maxmind discontinued support for the GeoIP Country DAT files in favor of the newer MMDB file format, I found the need to implement a drop in solution for some applications like nginx community edition, which did not yet natively support the MMDB format.

This program is written in C++ and has been tested compiling with G++ using C++ 2020 on Debian Linux 11.

This program does not take any parameters, it simply looks for the GeoIP2-Country.mmdb file in the currect directory, and outputs a GeoIP.dat file in the current directory.

# MMDB file format
---

Todo


# Dat file format
---

* **Record size:** 3 bytes
* **Country begin index:** 16776960

> ecoding! is little endian, maximum of 3 bytes, so 32 bit unsigned integers will be cropped to 24 bits

each trie segment/node is either
1. empty so returns just the Country begin index as a default -> u32
2. a reference to the index of another node/segment -> u32
3. a leaf node/segment with the Country begin index + the index of the country code in the static predefined list(compiled in) -> u32

## Begin
*encoded! loop through entire trie*
> ...

*three 0x00 bytes of padding*
> 0x00, 0x00, 0x00

*ASCII formatted text comment, can be anything, any length*
> Converted with mmdb2dat by Warp Speed Computers - https://www.warp.co.nz

*three 0xff bytes of padding*
> 0xff, 0xff, 0xff

*dat file edition, 1 = GeoIP to Country code*
> 0x01

*encoded! total number of trie segments/nodes*
> 0xf7, 0x42, 0x8b

# References
---
* https://github.com/sherpya/geolite2legacy/blob/master/pygeoip_const.py
* https://github.com/sherpya/geolite2legacy/blob/master/geolite2legacy.py
* https://github.com/mteodoro/mmutils/blob/master/csv2dat.py
* https://maxmind.github.io/MaxMind-DB/