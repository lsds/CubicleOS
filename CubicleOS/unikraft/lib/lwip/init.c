/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Simon Kuenzer <simon.kuenzer@neclab.eu>
 *
 * Copyright (c) 2019, NEC Laboratories Europe GmbH, NEC Corporation.
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <uk/config.h>
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/dhcp.h"
#if CONFIG_LWIP_NOTHREADS
#include "lwip/timeouts.h"
#else /* CONFIG_LWIP_NOTHREADS */
#include <uk/semaphore.h>
#endif /* CONFIG_LWIP_NOTHREADS */
#include "netif/uknetdev.h"
#include <uk/init.h>

#if LWIP_NETIF_EXT_STATUS_CALLBACK && CONFIG_LWIP_NETIF_STATUS_PRINT
#include <stdio.h>

static void _netif_status_print(struct netif *nf, netif_nsc_reason_t reason,
				const netif_ext_callback_args_t *args)
{
	if (reason & LWIP_NSC_NETIF_ADDED)
		printf("%c%c%u: Added\n",
		       nf->name[0], nf->name[1], nf->num);
	if (reason & LWIP_NSC_NETIF_REMOVED)
		printf("%c%c%u: Removed\n",
		       nf->name[0], nf->name[1], nf->num);
	if (reason & LWIP_NSC_LINK_CHANGED)
		printf("%c%c%u: Link is %s\n",
		       nf->name[0], nf->name[1], nf->num,
		       args->link_changed.state ? "up" : "down");
	if (reason & LWIP_NSC_STATUS_CHANGED)
		printf("%c%c%u: Interface is %s\n",
		       nf->name[0], nf->name[1], nf->num,
		       args->status_changed.state ? "up" : "down");

#if LWIP_IPV4
	if ((reason & LWIP_NSC_IPV4_SETTINGS_CHANGED)
	    || (reason & LWIP_NSC_IPV4_ADDRESS_CHANGED)
	    || (reason & LWIP_NSC_IPV4_NETMASK_CHANGED)
	    || (reason & LWIP_NSC_IPV4_GATEWAY_CHANGED)) {
		char str_ip4_addr[17];
		char str_ip4_mask[17];
		char str_ip4_gw[17];

		ipaddr_ntoa_r(&nf->ip_addr, str_ip4_addr, sizeof(str_ip4_addr));
		ipaddr_ntoa_r(&nf->netmask, str_ip4_mask, sizeof(str_ip4_mask));
		ipaddr_ntoa_r(&nf->gw,      str_ip4_gw,   sizeof(str_ip4_gw));

		printf("%c%c%u: Set IPv4 address %s mask %s gw %s\n",
		       nf->name[0], nf->name[1], nf->num,
		       str_ip4_addr, str_ip4_mask, str_ip4_gw);
	}
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
	if (reason & LWIP_NSC_IPV6_SET)
		printf("%c%c%u: Set IPv6 address %"__PRIs8": %s (state %"__PRIu8")\n",
		       nf->name[0], nf->name[1], nf->num,
		       args->ipv6_set.addr_index,
		       ipaddr_ntoa(&nf->ip6_addr[args->ipv6_set.addr_index]),
		       nf->ip6_addr_state[args->ipv6_set.addr_index]);
	if (reason & LWIP_NSC_IPV6_ADDR_STATE_CHANGED)
		printf("%c%c%u: Set IPv6 address %"__PRIs8": %s (state %"__PRIu8")\n",
		       nf->name[0], nf->name[1], nf->num,
		       args->ipv6_set.addr_index,
		       ipaddr_ntoa(&nf->ip6_addr[
				     args->ipv6_addr_state_changed.addr_index]),
		       nf->ip6_addr_state[
				     args->ipv6_addr_state_changed.addr_index]);
#endif /* LWIP_IPV6 */
}

NETIF_DECLARE_EXT_CALLBACK(netif_status_print)
#endif /* LWIP_NETIF_EXT_STATUS_CALLBACK && CONFIG_LWIP_NETIF_STATUS_PRINT */

void sys_init(void)
{
	/*
	 * This function is called before the any other sys_arch-function is
	 * called and is meant to be used to initialize anything that has to
	 * be up and running for the rest of the functions to work. for
	 * example to set up a pool of semaphores.
	 */
}

#if !CONFIG_LWIP_NOTHREADS
static struct uk_semaphore _lwip_init_sem;

static void _lwip_init_done(void *arg __unused)
{
	uk_semaphore_up(&_lwip_init_sem);
}
#endif /* !CONFIG_LWIP_NOTHREADS */

/*
 * This function initializing the lwip network stack
 */
int liblwip_init(void)
{
#if CONFIG_LWIP_UKNETDEV && CONFIG_LWIP_AUTOIFACE
	unsigned int devid;
	struct uk_netdev *dev;
	struct netif *nf;
	const char  __maybe_unused *strcfg;
	uint16_t  __maybe_unused int16cfg;
	int is_first_nf;
#if LWIP_IPV4
	ip4_addr_t __maybe_unused ip4;
	ip4_addr_t *ip4_arg;
	ip4_addr_t __maybe_unused mask4;
	ip4_addr_t *mask4_arg;
	ip4_addr_t __maybe_unused gw4;
	ip4_addr_t *gw4_arg;
#endif /* LWIP_IPV4 */
#endif /* CONFIG_LWIP_UKNETDEV && CONFIG_LWIP_AUTOIFACE */

	uk_pr_crit("Initializing lwip\n");
#if !CONFIG_LWIP_NOTHREADS
	uk_semaphore_init(&_lwip_init_sem, 0);
#endif /* !CONFIG_LWIP_NOTHREADS */

#if 1
//CONFIG_LWIP_NOTHREADS
	lwip_init();
#else /* CONFIG_LWIP_NOTHREADS */
//	tcpip_init(_lwip_init_done, NULL);
	tcpip_init(NULL, NULL);

	/* Wait until stack is booted */
//	uk_semaphore_down(&_lwip_init_sem);
#endif /* CONFIG_LWIP_NOTHREADS */

#if LWIP_NETIF_EXT_STATUS_CALLBACK && CONFIG_LWIP_NETIF_STATUS_PRINT
	/* Add print callback for netif state changes */
	netif_add_ext_callback(&netif_status_print, _netif_status_print);
#endif /* LWIP_NETIF_EXT_STATUS_CALLBACK && CONFIG_LWIP_NETIF_STATUS_PRINT */

#if CONFIG_LWIP_UKNETDEV && CONFIG_LWIP_AUTOIFACE
	is_first_nf = 1;

	for (devid = 0; devid < uk_netdev_count(); ++devid) {
		dev = uk_netdev_get(devid);
		if (!dev)
			continue;
		if (uk_netdev_state_get(dev) != UK_NETDEV_UNCONFIGURED) {
			uk_pr_crit("Skipping to add network device %u to lwIP: Not in unconfigured state\n",
				    devid);
			continue;
		}

		uk_pr_crit("Attach network device %u to lwIP...\n",
			   devid);

#if LWIP_IPV4
		char ip[]={10,0,1,1};
		char mask[]={255,255,255,0};
		char gw[]={10,0,1,254};
		ip4_arg   = ip;
		mask4_arg = mask;
		gw4_arg   = gw;

		/*
		 * TODO: Try to get device configuration from
		 * netdev's econf interface:
		 *
		 * UK_NETDEV_IPV4_ADDR_NINT16;
		 * UK_NETDEV_IPV4_ADDR_STR;
		 * UK_NETDEV_IPV4_MASK_NINT16;
		 * UK_NETDEV_IPV4_MASK_STR;
		 * UK_NETDEV_IPV4_GW_NINT16;
		 * UK_NETDEV_IPV4_GW_STR;
		 *
		 * When successfully done, set
		 *  ip_arg = &ip;
		 *  mask_arg = &mask;
		 *  gw_arg = &gw;
		 */

		nf = uknetdev_addif(dev, ip4_arg, mask4_arg, gw4_arg);
#else /* LWIP_IPV4 */
		/*
		 * TODO: Add support for IPv6 device configuration from
		 * netdev's econf interface
		 */

		nf = uknetdev_addif(dev);
#endif /* LWIP_IPV4 */
		if (!nf) {
			uk_pr_err("Failed to attach network device %u to lwIP\n",
				  devid);
			continue;
		}

		/* Declare the first network device as default interface */
		if (is_first_nf) {
			uk_pr_crit("%c%c%u: Set as default interface\n",
				   nf->name[0], nf->name[1], nf->num);
			netif_set_default(nf);
			is_first_nf = 0;
		}
		netif_set_up(nf);

#if LWIP_IPV4 && LWIP_DHCP
		if (!ip4_arg) {
			uk_pr_crit("%c%c%u: DHCP configuration (background)...\n",
				   nf->name[0], nf->name[1], nf->num);
			dhcp_start(nf);
		}
#endif /* LWIP_IPV4 && LWIP_DHCP */
	}
#endif /* CONFIG_LWIP_UKNETDEV && CONFIG_LWIP_AUTOIFACE */
	return 0;
}
uk_lib_initcall(liblwip_init);
