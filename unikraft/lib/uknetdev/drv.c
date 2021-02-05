#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uk/print.h>
#include <uk/assert.h>
#include <uk/essentials.h>
#include <uk/arch/types.h>
#include <uk/arch/limits.h>
#include <uk/netbuf.h>
#include <uk/netdev.h>
#include <uk/netdev_core.h>
#include <uk/netdev_driver.h>

#include "ipc.h"

struct uk_netdev_tx_queue {
	/* The virtqueue reference */
	struct virtqueue *vq;
	/* The hw queue identifier */
	uint16_t hwvq_id;
	/* The user queue identifier */
	uint16_t lqueue_id;
	/* The nr. of descriptor limit */
	uint16_t max_nb_desc;
	/* The nr. of descriptor user configured */
	uint16_t nb_desc;
	/* The flag to interrupt on the transmit queue */
	uint8_t intr_enabled;
	/* Reference to the uk_netdev */
	struct uk_netdev *ndev;
	/* The scatter list and its associated fragements */
//	struct uk_sglist sg;
//	struct uk_sglist_seg sgsegs[NET_MAX_FRAGMENTS];
};

/**
 * @internal structure to represent the receive queue.
 */
struct uk_netdev_rx_queue {
	/* The virtqueue reference */
	struct virtqueue *vq;
	/* The virtqueue hw identifier */
	uint16_t hwvq_id;
	/* The libuknet queue identifier */
	uint16_t lqueue_id;
	/* The nr. of descriptor limit */
	uint16_t max_nb_desc;
	/* The nr. of descriptor user configured */
	uint16_t nb_desc;
	/* The flag to interrupt on the transmit queue */
	uint8_t intr_enabled;
	/* User-provided receive buffer allocation function */
	uk_netdev_alloc_rxpkts alloc_rxpkts;
	void *alloc_rxpkts_argp;
	/* Reference to the uk_netdev */
	struct uk_netdev *ndev;
	/* The scatter list and its associated fragements */
//	struct uk_sglist sg;
//	struct uk_sglist_seg sgsegs[NET_MAX_FRAGMENTS];
};



int sock_r;

static unsigned virtio_net_promisc_get(struct uk_netdev *n) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	while(1);
}

#ifdef LLVM
struct uk_hwaddr mac;
#else
static struct pad_s {
	struct uk_hwaddr mac;
	char pad[4096-sizeof(struct uk_hwaddr)];
} __attribute__ ((aligned (4096))) macpad;
#endif


static const struct uk_hwaddr *virtio_net_mac_get(struct uk_netdev *n) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
#ifdef LLVM
	mac.addr_bytes[0] = 0x82;
	mac.addr_bytes[1] = 0x1d;
	mac.addr_bytes[2] = 0xb0;
	mac.addr_bytes[3] = 0x80;
	mac.addr_bytes[4] = 0x34;
	mac.addr_bytes[5] = 0x00;
	return &mac;
#else
	macpad.mac.addr_bytes[0] = 0x82;
	macpad.mac.addr_bytes[1] = 0x1d;
	macpad.mac.addr_bytes[2] = 0xb0;
	macpad.mac.addr_bytes[3] = 0x80;
	macpad.mac.addr_bytes[4] = 0x34;
	macpad.mac.addr_bytes[5] = 0x00;
	UKNETDEV_ipc_tab_globals[0].begin = &macpad.mac;
	return &macpad.mac;
#endif
}

static void virtio_net_info_get(struct uk_netdev *dev,
				struct uk_netdev_info *dev_info) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	dev_info->max_rx_queues = 1;
	dev_info->max_tx_queues = 1;
	dev_info->nb_encap_tx = 1;
	dev_info->nb_encap_rx = 1;
}

static int virtio_net_start(struct uk_netdev *n) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	uk_pr_crit("Let's say we are configuredd\n");

	return 0;
}


static int virtio_net_rx_intr_disable(struct uk_netdev *n,
				      struct uk_netdev_rx_queue *queue) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
}
static int virtio_net_rx_intr_enable(struct uk_netdev *n,
				     struct uk_netdev_rx_queue *queue) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	return 0;
}


static __u16 virtio_net_mtu_get(struct uk_netdev *n) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	return 1500;
}

static int virtio_netdev_txq_info_get(struct uk_netdev *dev, __u16 queue_id,
				      struct uk_netdev_queue_info *qinfo) {
	uk_pr_crit("%d:%s\n", __LINE__,__func__);
	while(1);

}

struct uk_netdev_rx_queue rxq;

static struct uk_netdev_rx_queue *virtio_netdev_rx_queue_setup(
					struct uk_netdev *n,
					uint16_t queue_id, uint16_t nb_desc,
					struct uk_netdev_rxqueue_conf *conf) {
	uk_pr_crit("%d:%s, %d\n", __LINE__,__func__, nb_desc);
	uk_pr_crit("I can get RX \n");
	return &rxq;
}

struct uk_netdev_tx_queue txq;


static struct uk_netdev_tx_queue *virtio_netdev_tx_queue_setup(
				struct uk_netdev *n, uint16_t queue_id __unused,
				uint16_t nb_desc __unused,
				struct uk_netdev_txqueue_conf *conf __unused) {
	uk_pr_crit("%d:%s, %d\n", __LINE__,__func__, nb_desc);
	uk_pr_crit("I can TX  \n");
	return &txq;
}


#define PATH_NET_TUN "/dev/net/tun"

#include <stdarg.h>

#define __SC_READ    0
#define __SC_WRITE   1
#define __SC_OPEN    2
#define __SC_CLOSE   3
#define __SC_MMAP    9
#define __SC_MUNMAP 11
#define __SC_RT_SIGACTION   13
#define __SC_RT_SIGPROCMASK 14
#define __SC_IOCTL  16
#define __SC_EXIT   60
#define __SC_ARCH_PRCTL       158
#define __SC_TIMER_CREATE     222
#define __SC_TIMER_SETTIME    223
#define __SC_TIMER_GETTIME    224
#define __SC_TIMER_GETOVERRUN 225
#define __SC_TIMER_DELETE     226
#define __SC_CLOCK_GETTIME    228
#define __SC_PSELECT6 270



#define syscall3(num, arg0, arg1, arg2)			\
({							\
	register long _nret asm("rax") = (num);		\
	register long _arg0 asm("rdi") = (arg0);	\
	register long _arg1 asm("rsi") = (arg1);	\
	register long _arg2 asm("rdx") = (arg2);	\
							\
	asm volatile ("syscall"				\
		      : /* output */			\
			"=r" (_nret)			\
		      : /* input */			\
			"r" (_nret),			\
			"r" (_arg0),			\
			"r" (_arg1),			\
			"r" (_arg2)			\
		      : /* clobbers */			\
			"r10", "r11", "rcx", "memory");	\
	_nret;						\
})

static inline ssize_t sys_read(int fd, const char *buf, size_t len)
{
	return (ssize_t) syscall3(__SC_READ,
				  (long) (fd),
				  (long) (buf),
				  (long) (len));
}

static inline ssize_t sys_write(int fd, const char *buf, size_t len)
{
	return (ssize_t) syscall3(__SC_WRITE,
				  (long) (fd),
				  (long) (buf),
				  (long) (len));
}


static inline int sys_ioctl(int fd, unsigned long req, ...)
{
	void *argp;
	va_list va;

	va_start(va, req);
	argp = va_arg(va, void *);
	va_end(va);

	return (int) syscall3(__SC_IOCTL,
			      (long) fd,
			      (long) req,
			      (long) argp);
}


static inline int sys_open(const char *pathname, int flags, mode_t mode)
{
	return (int) syscall3(2,
				  (long) (pathname),
				  (long) (flags),
				  (long) (mode));
}
#include <sys/socket.h>

#define IFHWADDRLEN	6
#define IFNAMSIZ	16

struct ifmap {
	unsigned long int mem_start;
	unsigned long int mem_end;
	unsigned short int base_addr;
	unsigned char irq;
	unsigned char dma;
	unsigned char port;
};

struct ifreq2 {
	union {
		char ifrn_name[IFNAMSIZ];
	} ifr_ifrn;
	union {
		struct sockaddr ifru_addr;
		struct sockaddr ifru_dstaddr;
		struct sockaddr ifru_broadaddr;
		struct sockaddr ifru_netmask;
		struct sockaddr ifru_hwaddr;
		short int ifru_flags;
		int ifru_ivalue;
		int ifru_mtu;
		struct ifmap ifru_map;
		char ifru_slave[IFNAMSIZ];
		char ifru_newname[IFNAMSIZ];
		char *ifru_data;
	} ifr_ifru;
};

#define ifr_flags	ifr_ifru.ifru_flags
#define ifr_name	ifr_ifrn.ifrn_name

#define O_RDWR 2
#define IFF_TAP 2
#define IFF_NO_PI 0x1000
#define TUNSETIFF 0x400454ca

static int virtio_netdev_configure(struct uk_netdev *n,
				   const struct uk_netdev_conf *conf) {
	int rc = 0;
	int ret,fd;

	UK_ASSERT(n);
	UK_ASSERT(conf);

    fd = sys_open(PATH_NET_TUN, O_RDWR | (2048), 0);
    if (fd < 0) {
        uk_pr_crit("could not open %s: %m\n", PATH_NET_TUN);
		while(1);
    }

    struct ifreq2 ifr;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

	char ifname[]="cubicle_tap0";

    if (ifname[0] != '\0')
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
    else
        strncpy(ifr.ifr_name, "tap%d", IFNAMSIZ-1);

    ret = sys_ioctl(fd, TUNSETIFF, (void *)&ifr);
    if (ret != 0) {
        if (ifname[0] != '\0') {
            uk_pr_crit("could not configure %s (%s): %m\n", PATH_NET_TUN, ifr.ifr_name);
        } else {
            uk_pr_crit("could not configure %s: %m\n", PATH_NET_TUN);
        }
		while(1);
    }

	sock_r = fd;

	return rc;
}


static int virtio_netdev_rxq_info_get(struct uk_netdev *dev,
				      __u16 queue_id,
				      struct uk_netdev_queue_info *qinfo)
{
	uk_pr_crit("%d\n",__LINE__);
	while(1);

}

char data[1500];
char priv[4096];

struct uk_netbuf buf;

extern struct uk_netbuf *lwip_alloc_netbuf(struct uk_alloc *a, size_t alloc_size, size_t headroom);

volatile int psize = 0;

int pull_rx(void *dptr) {
		if(psize > 0)
			return 0;
#if 0
//non blocking
		psize = sys_read(sock_r, dptr, 1400);
		if(psize <= 0 ) {
			return psize;
		}
#else
//blocking
		while(psize <= 0)
			psize = sys_read(sock_r, dptr, 1400);
#endif
//		uk_pr_crit("psize = %d\n", psize);
}

static int virtio_netdev_recv(struct uk_netdev *dev,
			      struct uk_netdev_rx_queue *queue,
			      struct uk_netbuf **pkt)
{
#if 0
//non-blocking
		pull_rx(data);

		if(psize <= 0)
			return 0;

		struct uk_netbuf *apb = lwip_alloc_netbuf(uk_alloc_get_default(), 2048, ETH_PAD_SIZE);
		memcpy(apb->data, data, psize);
#else
//blocking
		struct uk_netbuf *apb = lwip_alloc_netbuf(uk_alloc_get_default(), 2048, ETH_PAD_SIZE);
		pull_rx(apb->data);
#endif
//		uknetdev_ipc_add_heap(apb, 2048, (1 << 21) | ( 1 << 11));
//		uk_pr_crit("SIZE: %lx %p, %p\n", psize, apb, apb->data);

		apb->len = psize;
		(*pkt) = apb;

		psize = 0;

		return 1;
}

static int virtio_netdev_xmit(struct uk_netdev *dev,
			      struct uk_netdev_tx_queue *queue,
			      struct uk_netbuf *pkt) {
//	uk_pr_crit("%d\n",__LINE__);
//	uk_pr_crit("PKT: data %p, len %d \n",pkt->data, pkt->len);
	int ret = sys_write(sock_r, pkt->data, pkt->len);
	if(ret != pkt->len) {
		uk_pr_crit("ret = %d, expected %d\n", ret, pkt->len);while(1);
		return 0;
	}
//	uk_pr_crit("XMIT: %lx\n", ret);
	return 1;
}


static const struct uk_netdev_ops virtio_netdev_ops = {
	.configure = virtio_netdev_configure,
	.rxq_configure = virtio_netdev_rx_queue_setup,
	.txq_configure = virtio_netdev_tx_queue_setup,
	.start = virtio_net_start,
	.rxq_intr_enable = virtio_net_rx_intr_enable,
	.rxq_intr_disable = virtio_net_rx_intr_disable,
	.info_get = virtio_net_info_get,
	.promiscuous_get = virtio_net_promisc_get,
	.hwaddr_get = virtio_net_mac_get,
	.mtu_get = virtio_net_mtu_get,
	.txq_info_get = virtio_netdev_txq_info_get,
	.rxq_info_get = virtio_netdev_rxq_info_get,
};

struct uk_netdev netdev;

int net_add()
{
	netdev.rx_one = virtio_netdev_recv;
	netdev.tx_one = virtio_netdev_xmit;
	netdev.ops = &virtio_netdev_ops;

	int rc = uk_netdev_drv_register(&netdev, uk_alloc_get_default(), "linuxu_net");
	if (rc < 0) {
		uk_pr_err("Failed to register virtio-net device with libuknet\n");
		while(1);
	}

#if CONFIG_LIBLWIP
	extern void liblwip_init();
	liblwip_init();
#endif


	return 0;
}

