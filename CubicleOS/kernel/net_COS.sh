ip tuntap add dev cubicle_tap0 mode tap user `whoami`
ip link set dev cubicle_tap0 up
ip addr add dev cubicle_tap0 10.0.1.254/24

cd /CubicleOS/kernel

LD_LIBRARY_PATH=./nginx unbuffer ./loader nginx | ts -s '%M:%.S' &

sleep 10

for ((k=1; k<=8*1024*1024;k*=2))
do
for i in {1..1}
do
#    echo -e "$k\t$i\t$rr\t\c"
    ts=$(date +%s%N)
    for j in {1..1}
    do
        curl -w ' %{size_download}' 'http://10.0.1.1/'$k -so  /dev/null
    done
    tt=$((($(date +%s%N) - $ts)/1000000/1))
    echo -e "\t$tt"
done
done
