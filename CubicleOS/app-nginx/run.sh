#find . -print -depth | sort | cpio -vo -H newc > ../root.cpio
xxd -i root.cpio > ../unikraft/lib/libnginx/rootfs.h

