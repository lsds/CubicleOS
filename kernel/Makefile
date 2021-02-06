CC=gcc
CFLAGS=-ldl -m64 -fPIC -O3 -lpthread
#CFLAGS=-ldl -m64 -fPIC -g -lpthread

DEPS = gen_hooks.h

OBJ = elf_hook.o plthook_elf.o getcontext.o setcontext.o mini-printf.o

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

gen_hooks.h:
	python3 fig.py ../unikraft/ > gen_hooks.h

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


.PHONY: clean run

clean:
	rm -rf ./*.o loader loaer4 loader3 ./*.so $(SQLITE_DIR)

#| ts -s '%M:%.S'

sqlite: loader loader3 loader4 $(SQLITE_DIR)/sqlite.so
#	LD_LIBRARY_PATH=./sqlite unbuffer ./loader sqlite --size 100 -mmap 0 --stats testing | ts -s '%M:%.S'

