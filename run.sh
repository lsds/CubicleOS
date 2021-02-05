docker build . --tag cubicles

mkdir -p parser/paper_repro

# Speedtest1 benchmarks

##################################
## Vanilla Linux 	01_vanila.txt
docker run --privileged --rm -it  cubicles:latest /CubicleOS/vanilla/vanilla --size 100 -mmap 0 --stats testing | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g'> parser/paper_repro/01_vanila.txt
## Unikraft		02_unikraft.txt
docker run --privileged --rm -it  cubicles:latest /CubicleOS/app-sqlite/build/app-sqlite_linuxu-x86_64 --size 100 -mmap 0 --stats testing | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/02_unikraft.txt

##################################
## The Genode framework with 3 components and Linux
## Linux		03_genode_3.txt
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 run/sqlite | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/03_genode_3.txt
## Linux		04_genode_4.txt
docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 run/sqlite4 | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/04_genode_4.txt

################################# REQUIRE MPK ##########################
## CubicleOS with 3 components	05_cubicle_3.txt
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader3 /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/05_cubicle_3.txt
## CubicleOS with 4 components	06_cubicle_4.txt
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader4 /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g'> parser/paper_repro/06_cubicle_4.txt
## CubicleOS with 7 components	07_cubicle_7.txt
docker run --env LD_LIBRARY_PATH=/CubicleOS/kernel/sqlite --privileged --rm -it  cubicles:latest /CubicleOS/kernel/loader /CubicleOS/kernel/sqlite --size 100 -mmap 0 --stats testing | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g'> parser/paper_repro/07_cubicle_7.txt

################################# REQUIRE VIRTUALISATION ###############
## Genode
## SeL4	3 components		08_sel4_3.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=sel4 BOARD=pc run/sqlite | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' | sed 's/\r//g'> parser/paper_repro/08_sel4_3.txt
### SeL4 4 components		09_sel4_4.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=sel4 BOARD=pc run/sqlite4 | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/09_sel4_4.txt

### Fiasco.OC 3 components	10_foc_3.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=foc BOARD=pc run/sqlite | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g' > parser/paper_repro/10_foc_3.txt
### Fiasco.OC 4 components	11_foc_4.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=foc BOARD=pc run/sqlite4  | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g'  |  sed 's/\r//g'> parser/paper_repro/11_foc_4.txt

### NOVA 3 components		12_nova_3.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=nova BOARD=pc run/sqlite | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' |   sed 's/\r//g' > parser/paper_repro/12_nova_3.txt
### NOVA 4 components		13_nova_4.txt
#docker run --privileged --rm -it  cubicles:latest make -C /genode/build/x86_64 KERNEL=nova BOARD=pc run/sqlite4 | grep "\.\.\.\." | grep -v "TOTAL" | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' |  sed 's/\r//g' > parser/paper_repro/13_nova_4.txt

################################# NGINX
# Simple curl-based benchmark for NGINX

## Baseline Unikraft
#docker run --cap-add=NET_ADMIN --device /dev/net/tun --env LD_LIBRARY_PATH=/CubicleOS/kernel/nginx --privileged --rm -it  cubicles:latest /bin/bash /CubicleOS/kernel/net_UNI.sh

## CubicleOS 
#docker run --cap-add=NET_ADMIN --device /dev/net/tun --env LD_LIBRARY_PATH=/CubicleOS/kernel/nginx --privileged --rm -it  cubicles:latest /bin/bash /CubicleOS/kernel/net_COS.sh
