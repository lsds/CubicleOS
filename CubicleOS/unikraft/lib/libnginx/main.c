#include <stdio.h>
#include <stdlib.h>

#include "rootfs.h"
#include "cpio.h"

#include "ipc.h"

extern int nginx_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	app_ipc_add_global(0, root_cpio, sizeof(root_cpio), (1 << 21) | (1 << 0));

	copy_cpio(root_cpio);

	return nginx_main(argc, argv);
}
