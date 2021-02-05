CC=gcc
CFLAGS=-ldl -m64 -fPIC -O3 -lpthread
#CFLAGS=-ldl -m64 -fPIC -g -lpthread

DEPS = gen_hooks.h

OBJ = elf_hook.o plthook_elf.o getcontext.o setcontext.o mini-printf.o

HELLO_DIR=helloworld
HELLO_OS_SRC = ../app-$(HELLO_DIR)/build
HELLO_SOS = \
	$(HELLO_DIR)/liblinuxuplat.so   $(HELLO_DIR)/libukboot.so $(HELLO_DIR)/libukboot_main.so \
	$(HELLO_DIR)/libposix_user.so $(HELLO_DIR)/libposix_sysinfo.so $(HELLO_DIR)/libukdebug.so \
	$(HELLO_DIR)/libukargparse.so $(HELLO_DIR)/libukalloc.so $(HELLO_DIR)/libukallocbbuddy.so \
	$(HELLO_DIR)/libvfscore.so $(HELLO_DIR)/libramfs.so  \
	$(HELLO_DIR)/libuktime.so $(HELLO_DIR)/libukmmap.so \
	$(HELLO_DIR)/libposix_process.so  $(HELLO_DIR)/libnolibc.so \

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.S 
	$(CC) -c -o $@ $< $(CFLAGS)


loader: $(OBJ)
	$(CC) loader.c -o $@ $^ $(CFLAGS) $(LIBS)

loader3: $(OBJ)
	$(CC) loader.c -o $@ $^ $(CFLAGS) $(LIBS) -DSQLITE_FINAL -DSQLITE_3

loader4: $(OBJ)
	$(CC) loader.c -o $@ $^ $(CFLAGS) $(LIBS) -DSQLITE_FINAL

loader_nginx_uni: $(OBJ)
	$(CC) loader.c -o $@ $^ $(CFLAGS) $(LIBS) -DNOMPK


gen_hooks.h:
	python3 fig.py ../unikraft/ > gen_hooks.h

$(HELLO_DIR)/%.so: $(HELLO_OS_SRC)/%.o
	$(shell mkdir -p $(HELLO_DIR))
	$(CC) -nostdinc -nostdlib -m64 -shared -o $@ $< -fPIC



$(HELLO_DIR)/hello.so: $(HELLO_SOS)
	$(CC)  -nostdinc -nostdlib -g3 -m64 -fPIC -shared \
	../app-$(HELLO_DIR)/build/apphelloworld.o \
	-Wl,--start-group     -Wl,--end-group -Wl,-T,../app-$(HELLO_DIR)/build/liblinuxuplat/link64.lds \
	-Wl,-T,../unikraft/lib/ukdebug/extra.ld -Wl,-T,../unikraft/lib/vfscore/extra.ld \
	-o $(HELLO_DIR)/app.so -L$(HELLO_DIR)/ \
	-lposix_sysinfo -lukallocbbuddy -lukboot_main \
	-llinuxuplat -lposix_user     -lukalloc        -lukboot       -luktime \
	-lnolibc -lramfs -lukdebug      -lvfscore
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libvfscore.so ../app-$(HELLO_DIR)/build/libvfscore.o -fPIC -L$(HELLO_DIR) -llinuxuplat -lramfs -luktime
#	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libramfs.so ../app-$(HELLO_DIR)/build/libramfs.o -fPIC -L$(HELLO_DIR) -lnolibc
	clang-7 -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libramfs.so  ../app-$(HELLO_DIR)/ipc.ll ../app-$(HELLO_DIR)/libramfs3.ll -fPIC -L$(HELLO_DIR) -lnolibc
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libukdebug.so ../app-$(HELLO_DIR)/build/libukdebug.o -fPIC -L$(HELLO_DIR) -llinuxuplat
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libukallocbbuddy.so ../app-$(HELLO_DIR)/build/libukallocbbuddy.o -fPIC -L$(HELLO_DIR) -lukdebug -lukalloc
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libukboot.so ../app-$(HELLO_DIR)/build/libukboot.o -fPIC -L$(HELLO_DIR) -lukargparse -lukdebug -lukallocbbuddy -lvfscore -lposix_user
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/libnolibc.so ../app-$(HELLO_DIR)/build/libnolibc.o -fPIC -L$(HELLO_DIR) -lukallocbbuddy -lukalloc
	gcc -nostdinc -nostdlib -m64 -shared -o $(HELLO_DIR)/liblinuxuplat.so ../app-$(HELLO_DIR)/build/liblinuxuplat.o -fPIC -L$(HELLO_DIR) -lukboot -luktime


##################3 SQLITE

SQLITE_DIR=sqlite
SQLITE_OS_SRC = ../app-$(SQLITE_DIR)/build
SQLITE_SOS = \
	$(SQLITE_DIR)/liblinuxuplat.so   $(SQLITE_DIR)/libukboot.so $(SQLITE_DIR)/libukboot_main.so \
	$(SQLITE_DIR)/libposix_user.so $(SQLITE_DIR)/libposix_sysinfo.so $(SQLITE_DIR)/libukdebug.so \
	$(SQLITE_DIR)/libukargparse.so $(SQLITE_DIR)/libukalloc.so $(SQLITE_DIR)/libukallocbbuddy.so \
	$(SQLITE_DIR)/libvfscore.so $(SQLITE_DIR)/libramfs.so $(SQLITE_DIR)/libdevfs.so \
	$(SQLITE_DIR)/libuktime.so $(SQLITE_DIR)/libukmmap.so \
	$(SQLITE_DIR)/libposix_process.so  $(SQLITE_DIR)/libnewlibc.so \
	$(SQLITE_DIR)/libnewlibm.so $(SQLITE_DIR)/libnewlibglue.so


$(SQLITE_DIR)/%.so: $(SQLITE_OS_SRC)/%.o
	$(shell mkdir -p $(SQLITE_DIR))
	$(CC) -nostdinc -nostdlib -m64 -shared -o $@ $< -fPIC

$(SQLITE_DIR)/sqlite.so: $(SQLITE_SOS)
	$(CC)  -nostdinc -nostdlib -g3 -m64 -fPIC -shared \
	../app-$(SQLITE_DIR)/build/libsqlite.o \
	-Wl,--start-group     -Wl,--end-group -Wl,-T,../app-$(SQLITE_DIR)/build/liblinuxuplat/link64.lds \
	-Wl,-T,../unikraft/lib/ukdebug/extra.ld -Wl,-T,../unikraft/lib/vfscore/extra.ld \
	-o $(SQLITE_DIR)/app.so \
	-L$(SQLITE_DIR)/ \
	-ldevfs -lnewlibglue -lposix_sysinfo -lukallocbbuddy -lukboot_main -lukmmap \
	-llinuxuplat -lnewlibm        -lposix_user     -lukalloc        -lukboot       -luktime \
	-lnewlibc -lposix_process  -lramfs          -lukargparse     -lukdebug      -lvfscore
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libukdebug.so ../app-$(SQLITE_DIR)/build/libukdebug.o -fPIC -L$(SQLITE_DIR) -llinuxuplat
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libukallocbbuddy.so ../app-$(SQLITE_DIR)/build/libukallocbbuddy.o -fPIC -L$(SQLITE_DIR) -lukdebug -lukalloc
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libvfscore.so ../app-$(SQLITE_DIR)/build/libvfscore.o -fPIC -L$(SQLITE_DIR) -llinuxuplat -ldevfs -lramfs
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libramfs.so ../app-$(SQLITE_DIR)/build/libramfs.o -fPIC -L$(SQLITE_DIR) -lnewlibc
#	clang-7 -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libramfs.so  ../app-$(SQLITE_DIR)/ipc.ll ../app-$(SQLITE_DIR)/libramfs3.ll -fPIC -L$(SQLITE_DIR) -lnewlibc
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libdevfs.so ../app-$(SQLITE_DIR)/build/libdevfs.o -fPIC -L$(SQLITE_DIR) -lvfscore -lnewlibc -lukdebug
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libukboot.so ../app-$(SQLITE_DIR)/build/libukboot.o -fPIC -L$(SQLITE_DIR) -lukargparse -lukdebug -lukallocbbuddy -lvfscore
	gcc -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libnewlibm.so ../app-$(SQLITE_DIR)/build/libnewlibm.o -fPIC -L$(SQLITE_DIR) -lnewlibc
	gcc -fno-plt -nostdinc -nostdlib -m64 -shared -o $(SQLITE_DIR)/libnewlibglue.so ../app-$(SQLITE_DIR)/build/libnewlibglue.o -fPIC -L$(SQLITE_DIR) -lnewlibc

######################3 PYTHON 
PYTHON3_DIR=python3
PYTHON3_OS_SRC = ../app-$(PYTHON3_DIR)/build
PYTHON3_SOS = \
	$(PYTHON3_DIR)/liblinuxuplat.so   $(PYTHON3_DIR)/libukboot.so $(PYTHON3_DIR)/libukboot_main.so \
	$(PYTHON3_DIR)/libposix_user.so $(PYTHON3_DIR)/libposix_sysinfo.so $(PYTHON3_DIR)/libukdebug.so \
	$(PYTHON3_DIR)/libukargparse.so $(PYTHON3_DIR)/libukalloc.so $(PYTHON3_DIR)/libukallocbbuddy.so \
	$(PYTHON3_DIR)/libvfscore.so $(PYTHON3_DIR)/libramfs.so $(PYTHON3_DIR)/libdevfs.so \
	$(PYTHON3_DIR)/libuktime.so $(PYTHON3_DIR)/libukmmap.so \
	$(PYTHON3_DIR)/libposix_process.so  $(PYTHON3_DIR)/libnewlibc.so \
	$(PYTHON3_DIR)/libnewlibm.so $(PYTHON3_DIR)/libnewlibglue.so $(PYTHON3_DIR)/libnewlibc.so\
	$(PYTHON3_DIR)/libuksched.so $(PYTHON3_DIR)/libposix_user.so $(PYTHON3_DIR)/libuknetdev.so $(PYTHON3_DIR)/libuklock.so \
	$(PYTHON3_DIR)/liblwip.so $(PYTHON3_DIR)/libukswrand.so $(PYTHON3_DIR)/libukmpi.so $(PYTHON3_DIR)/libpthread-embedded.so \
	$(PYTHON3_DIR)/libukschedcoop.so 	$(PYTHON3_DIR)/libsyscall_shim.so $(PYTHON3_DIR)/libposix_libdl.so


$(PYTHON3_DIR)/%.so: $(PYTHON3_OS_SRC)/%.o
	$(shell mkdir -p $(PYTHON3_DIR))
	$(CC) -nostdinc -nostdlib -m64 -shared -o $@ $< -fPIC

$(PYTHON3_DIR)/python3.so: $(PYTHON3_SOS)
	$(CC)  -nostdinc -nostdlib -g3 -m64 -fPIC -shared \
	../app-$(PYTHON3_DIR)/build/libpython3.o \
	../app-$(PYTHON3_DIR)/build/libpython3_core.o \
	../app-$(PYTHON3_DIR)/build/libpython3_extensions.o \
	../app-$(PYTHON3_DIR)/build/libffi.o \
	-Wl,--start-group     -Wl,--end-group -Wl,-T,../app-$(PYTHON3_DIR)/build/liblinuxuplat/link64.lds \
	-Wl,-T,../unikraft/lib/ukdebug/extra.ld -Wl,-T,../unikraft/lib/vfscore/extra.ld \
	-o $(PYTHON3_DIR)/app.so \
	-L$(PYTHON3_DIR)/ \
	-ldevfs -lnewlibglue -lposix_sysinfo -lukallocbbuddy -lukboot_main -lukmmap \
	-llinuxuplat -lnewlibm        -lposix_user     -lukalloc        -lukboot       -luktime \
	-lnewlibc     -lposix_process  -lramfs          -lukargparse     -lukdebug      -lvfscore -luksched -lposix_user \
	-luknetdev -llwip -lposix_libdl
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukdebug.so ../app-$(PYTHON3_DIR)/build/libukdebug.o -fPIC -L$(PYTHON3_DIR) -llinuxuplat
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukallocbbuddy.so ../app-$(PYTHON3_DIR)/build/libukallocbbuddy.o -fPIC -L$(PYTHON3_DIR) -lukdebug -lukalloc
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libvfscore.so ../app-$(PYTHON3_DIR)/build/libvfscore.o -fPIC -L$(PYTHON3_DIR) -llinuxuplat -ldevfs -lramfs
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libramfs.so ../app-$(PYTHON3_DIR)/build/libramfs.o -fPIC -L$(PYTHON3_DIR) -lnewlibc -lukdebug
#	clang-7 -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libramfs.so  ../app-$(PYTHON3_DIR)/ipc.ll ../app-$(PYTHON3_DIR)/libramfs3.ll -fPIC -L$(PYTHON3_DIR) -lnewlibc
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libdevfs.so ../app-$(PYTHON3_DIR)/build/libdevfs.o -fPIC -L$(PYTHON3_DIR) -lvfscore -lnewlibc -lukdebug
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukboot.so ../app-$(PYTHON3_DIR)/build/libukboot.o -fPIC -L$(PYTHON3_DIR) -lukargparse -lukdebug -lukallocbbuddy -lvfscore -llwip
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/liblinuxuplat.so ../app-$(PYTHON3_DIR)/build/liblinuxuplat.o -fPIC -L$(PYTHON3_DIR) -luknetdev -luklock -lukboot
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libnewlibm.so ../app-$(PYTHON3_DIR)/build/libnewlibm.o -fPIC -L$(PYTHON3_DIR) -lnewlibc
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libnewlibc.so ../app-$(PYTHON3_DIR)/build/libnewlibc.o -fPIC -L$(PYTHON3_DIR) -lnewlibm
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libnewlibglue.so ../app-$(PYTHON3_DIR)/build/libnewlibglue.o -fPIC -L$(PYTHON3_DIR) -lnewlibc -lsyscall_shim
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libuksched.so ../app-$(PYTHON3_DIR)/build/libuksched.o -fPIC -L$(PYTHON3_DIR) -lukschedcoop
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libuklock.so ../app-$(PYTHON3_DIR)/build/libuklock.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/liblwip.so ../app-$(PYTHON3_DIR)/build/liblwip.o -fPIC -L$(PYTHON3_DIR) -lukswrand -lukmpi -lpthread-embedded
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukswrand.so ../app-$(PYTHON3_DIR)/build/libukswrand.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukmpi.so ../app-$(PYTHON3_DIR)/build/libukmpi.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libpthread-embedded.so ../app-$(PYTHON3_DIR)/build/libpthread-embedded.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libukschedcoop.so ../app-$(PYTHON3_DIR)/build/libukschedcoop.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libsyscall_shim.so ../app-$(PYTHON3_DIR)/build/libsyscall_shim.o -fPIC -L$(PYTHON3_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(PYTHON3_DIR)/libposix_libdl.so ../app-$(PYTHON3_DIR)/build/libposix_libdl.o -fPIC -L$(PYTHON3_DIR)

###################### NGINX
NGINX_DIR=nginx
NGINX_OS_SRC = ../app-$(NGINX_DIR)/build
NGINX_SOS = \
	$(NGINX_DIR)/liblinuxuplat.so   $(NGINX_DIR)/libukboot.so $(NGINX_DIR)/libukboot_main.so \
	$(NGINX_DIR)/libposix_user.so $(NGINX_DIR)/libposix_sysinfo.so $(NGINX_DIR)/libukdebug.so \
	$(NGINX_DIR)/libukargparse.so $(NGINX_DIR)/libukalloc.so $(NGINX_DIR)/libukallocbbuddy.so \
	$(NGINX_DIR)/libvfscore.so $(NGINX_DIR)/libramfs.so $(NGINX_DIR)/libdevfs.so \
	$(NGINX_DIR)/libuktime.so $(NGINX_DIR)/libukmmap.so \
	$(NGINX_DIR)/libposix_process.so  $(NGINX_DIR)/libnewlibc.so \
	$(NGINX_DIR)/libnewlibm.so $(NGINX_DIR)/libnewlibglue.so $(NGINX_DIR)/libnewlibc.so\
	$(NGINX_DIR)/libuksched.so $(NGINX_DIR)/libposix_user.so $(NGINX_DIR)/libuknetdev.so $(NGINX_DIR)/libuklock.so \
	$(NGINX_DIR)/liblwip.so $(NGINX_DIR)/libukswrand.so $(NGINX_DIR)/libukmpi.so $(NGINX_DIR)/libpthread-embedded.so \
	$(NGINX_DIR)/libukschedcoop.so 	$(NGINX_DIR)/libsyscall_shim.so $(NGINX_DIR)/libposix_libdl.so


$(NGINX_DIR)/%.so: $(NGINX_OS_SRC)/%.o
	$(shell mkdir -p $(NGINX_DIR))
	$(CC) -nostdinc -nostdlib -m64 -shared -o $@ $< -fPIC

$(NGINX_DIR)/nginx.so: $(NGINX_SOS)
	$(CC)  -nostdinc -nostdlib -g3 -m64 -fPIC -shared \
	../app-$(NGINX_DIR)/build/libnginx.o \
	-Wl,--start-group     -Wl,--end-group -Wl,-T,../app-$(NGINX_DIR)/build/liblinuxuplat/link64.lds \
	-Wl,-T,../unikraft/lib/ukdebug/extra.ld -Wl,-T,../unikraft/lib/vfscore/extra.ld \
	-o $(NGINX_DIR)/app.so \
	-L$(NGINX_DIR)/ \
	-ldevfs -lnewlibglue -lposix_sysinfo -lukallocbbuddy -lukboot_main -lukmmap \
	-llinuxuplat -lnewlibm        -lposix_user     -lukalloc        -lukboot       -luktime \
	-lnewlibc     -lposix_process  -lramfs          -lukargparse     -lukdebug      -lvfscore -luksched -lposix_user \
	-luknetdev -llwip -lposix_libdl
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukdebug.so ../app-$(NGINX_DIR)/build/libukdebug.o -fPIC -L$(NGINX_DIR) -llinuxuplat
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukallocbbuddy.so ../app-$(NGINX_DIR)/build/libukallocbbuddy.o -fPIC -L$(NGINX_DIR) -lukdebug -lukalloc
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libvfscore.so ../app-$(NGINX_DIR)/build/libvfscore.o -fPIC -L$(NGINX_DIR) -llinuxuplat -ldevfs -lramfs
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libramfs.so ../app-$(NGINX_DIR)/build/libramfs.o -fPIC -L$(NGINX_DIR) -lnewlibc -lukdebug
#	clang-7 -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libramfs.so  ../app-$(NGINX_DIR)/ipc.ll ../app-$(NGINX_DIR)/libramfs3.ll -fPIC -L$(NGINX_DIR) -lnewlibc
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libdevfs.so ../app-$(NGINX_DIR)/build/libdevfs.o -fPIC -L$(NGINX_DIR) -lvfscore -lnewlibc -lukdebug
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukboot.so ../app-$(NGINX_DIR)/build/libukboot.o -fPIC -L$(NGINX_DIR) -lukargparse -lukdebug -lukallocbbuddy -lvfscore -llwip
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/liblinuxuplat.so ../app-$(NGINX_DIR)/build/liblinuxuplat.o -fPIC -L$(NGINX_DIR) -luknetdev -luklock -lukboot
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libnewlibm.so ../app-$(NGINX_DIR)/build/libnewlibm.o -fPIC -L$(NGINX_DIR) -lnewlibc
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libnewlibc.so ../app-$(NGINX_DIR)/build/libnewlibc.o -fPIC -L$(NGINX_DIR) -lnewlibm
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libnewlibglue.so ../app-$(NGINX_DIR)/build/libnewlibglue.o -fPIC -L$(NGINX_DIR) -lnewlibc -lsyscall_shim
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libuksched.so ../app-$(NGINX_DIR)/build/libuksched.o -fPIC -L$(NGINX_DIR) -lukschedcoop
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libuklock.so ../app-$(NGINX_DIR)/build/libuklock.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/liblwip.so ../app-$(NGINX_DIR)/build/liblwip.o -fPIC -L$(NGINX_DIR) -lukswrand -lukmpi -lpthread-embedded
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukswrand.so ../app-$(NGINX_DIR)/build/libukswrand.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukmpi.so ../app-$(NGINX_DIR)/build/libukmpi.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libpthread-embedded.so ../app-$(NGINX_DIR)/build/libpthread-embedded.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libukschedcoop.so ../app-$(NGINX_DIR)/build/libukschedcoop.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libsyscall_shim.so ../app-$(NGINX_DIR)/build/libsyscall_shim.o -fPIC -L$(NGINX_DIR)
	gcc -nostdinc -nostdlib -m64 -shared -o $(NGINX_DIR)/libposix_libdl.so ../app-$(NGINX_DIR)/build/libposix_libdl.o -fPIC -L$(NGINX_DIR)


.PHONY: clean run

clean:
	rm -rf ./*.o loader loaer4 loader3 ./*.so $(HELLO_DIR) $(SQLITE_DIR) $(PYTHON3_DIR) $(NGINX_DIR)

#| ts -s '%M:%.S'

helloworld: loader $(HELLO_DIR)/hello.so
	LD_LIBRARY_PATH=./hello unbuffer ./loader hello 

sqlite: loader loader3 loader4 $(SQLITE_DIR)/sqlite.so
#	LD_LIBRARY_PATH=./sqlite unbuffer ./loader sqlite --size 100 -mmap 0 --stats testing | ts -s '%M:%.S'

nginx: loader loader_nginx_uni $(NGINX_DIR)/nginx.so
#	LD_LIBRARY_PATH=./nginx unbuffer ./loader nginx | ts -s '%M:%.S'
