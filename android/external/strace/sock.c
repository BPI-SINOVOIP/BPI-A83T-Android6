/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include <sys/socket.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#if defined(ALPHA) || defined(SH) || defined(SH64)
# if defined(HAVE_SYS_IOCTL_H)
#  include <sys/ioctl.h>
# elif defined(HAVE_IOCTLS_H)
#  include <ioctls.h>
# endif
#endif
#include <net/if.h>

#include "xlat/iffflags.h"

static void
print_addr(struct tcb *tcp, long addr, struct ifreq *ifr)
{
	if (ifr->ifr_addr.sa_family == AF_INET) {
		struct sockaddr_in *sinp;
		sinp = (struct sockaddr_in *) &ifr->ifr_addr;
		tprintf("inet_addr(\"%s\")", inet_ntoa(sinp->sin_addr));
	} else
		printstr(tcp, addr, sizeof(ifr->ifr_addr.sa_data));
}

static void
print_ifname(const char *ifname)
{
	print_quoted_string(ifname, IFNAMSIZ + 1, QUOTE_0_TERMINATED);
}

int
sock_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	struct ifreq ifr;
	struct ifconf ifc;
	const char *str = NULL;
	unsigned char *bytes;

	if (entering(tcp)) {
		switch (code) {
		case SIOCGIFCONF:
			if (umove(tcp, tcp->u_arg[2], &ifc) >= 0
			    && ifc.ifc_buf == NULL)
				tprintf(", {%d -> ", ifc.ifc_len);
			else
				tprints(", {");
			break;
		case SIOCSIFNAME:
			if (umove(tcp, tcp->u_arg[2], &ifr) < 0)
				tprintf(", %#lx", tcp->u_arg[2]);
			else {
				tprints(", {ifr_name=");
				print_ifname(ifr.ifr_name);
				tprints(", ifr_newname=");
				print_ifname(ifr.ifr_newname);
				tprints("}");
			}
			break;
		}
		return 0;
	}

	switch (code) {
#ifdef SIOCSHIWAT
	case SIOCSHIWAT:
#endif
#ifdef SIOCGHIWAT
	case SIOCGHIWAT:
#endif
#ifdef SIOCSLOWAT
	case SIOCSLOWAT:
#endif
#ifdef SIOCGLOWAT
	case SIOCGLOWAT:
#endif
#ifdef FIOSETOWN
	case FIOSETOWN:
#endif
#ifdef FIOGETOWN
	case FIOGETOWN:
#endif
#ifdef SIOCSPGRP
	case SIOCSPGRP:
#endif
#ifdef SIOCGPGRP
	case SIOCGPGRP:
#endif
#ifdef SIOCATMARK
	case SIOCATMARK:
#endif
		printnum_int(tcp, arg, ", %d");
	case SIOCSIFNAME:
		return 1;
	case SIOCGIFNAME:
	case SIOCGIFINDEX:
	case SIOCGIFADDR:
	case SIOCSIFADDR:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCGIFFLAGS:
	case SIOCSIFFLAGS:
	case SIOCGIFMETRIC:
	case SIOCSIFMETRIC:
	case SIOCGIFMTU:
	case SIOCSIFMTU:
	case SIOCGIFSLAVE:
	case SIOCSIFSLAVE:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
	case SIOCGIFTXQLEN:
	case SIOCSIFTXQLEN:
	case SIOCGIFMAP:
	case SIOCSIFMAP:
		if (umove(tcp, tcp->u_arg[2], &ifr) < 0)
			tprintf(", %#lx", tcp->u_arg[2]);
		else if (syserror(tcp)) {
			if (code == SIOCGIFNAME) {
				tprintf(", {ifr_index=%d, ifr_name=???}",
					ifr.ifr_ifindex);
			} else {
				tprints(", {ifr_name=");
				print_ifname(ifr.ifr_name);
				tprints(", ???}");
			}
		} else if (code == SIOCGIFNAME) {
			tprintf(", {ifr_index=%d, ifr_name=", ifr.ifr_ifindex);
			print_ifname(ifr.ifr_name);
			tprints("}");
		} else {
			tprints(", {ifr_name=");
			print_ifname(ifr.ifr_name);
			tprints(", ");
			switch (code) {
			case SIOCGIFINDEX:
				tprintf("ifr_index=%d", ifr.ifr_ifindex);
				break;
			case SIOCGIFADDR:
			case SIOCSIFADDR:
				str = "ifr_addr";
			case SIOCGIFDSTADDR:
			case SIOCSIFDSTADDR:
				if (!str)
					str = "ifr_dstaddr";
			case SIOCGIFBRDADDR:
			case SIOCSIFBRDADDR:
				if (!str)
					str = "ifr_broadaddr";
			case SIOCGIFNETMASK:
			case SIOCSIFNETMASK:
				if (!str)
					str = "ifr_netmask";
				tprintf("%s={", str);
				printxval(addrfams,
					  ifr.ifr_addr.sa_family,
					  "AF_???");
				tprints(", ");
				print_addr(tcp, ((long) tcp->u_arg[2]
						 + offsetof(struct ifreq,
							     ifr_addr.sa_data)),
					   &ifr);
				tprints("}");
				break;
			case SIOCGIFHWADDR:
			case SIOCSIFHWADDR:
				/* XXX Are there other hardware addresses
				   than 6-byte MACs?  */
				bytes = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
				tprintf("ifr_hwaddr=%02x:%02x:%02x:%02x:%02x:%02x",
					bytes[0], bytes[1], bytes[2],
					bytes[3], bytes[4], bytes[5]);
				break;
			case SIOCGIFFLAGS:
			case SIOCSIFFLAGS:
				tprints("ifr_flags=");
				printflags(iffflags, ifr.ifr_flags, "IFF_???");
				break;
			case SIOCGIFMETRIC:
			case SIOCSIFMETRIC:
				tprintf("ifr_metric=%d", ifr.ifr_metric);
				break;
			case SIOCGIFMTU:
			case SIOCSIFMTU:
				tprintf("ifr_mtu=%d", ifr.ifr_mtu);
				break;
			case SIOCGIFSLAVE:
			case SIOCSIFSLAVE:
				tprints("ifr_slave=");
				print_ifname(ifr.ifr_slave);
				break;
			case SIOCGIFTXQLEN:
			case SIOCSIFTXQLEN:
				tprintf("ifr_qlen=%d", ifr.ifr_qlen);
				break;
			case SIOCGIFMAP:
			case SIOCSIFMAP:
				tprintf("ifr_map={mem_start=%#lx, "
					"mem_end=%#lx, base_addr=%#x, "
					"irq=%u, dma=%u, port=%u}",
					ifr.ifr_map.mem_start,
					ifr.ifr_map.mem_end,
					(unsigned) ifr.ifr_map.base_addr,
					(unsigned) ifr.ifr_map.irq,
					(unsigned) ifr.ifr_map.dma,
					(unsigned) ifr.ifr_map.port);
				break;
			}
			tprints("}");
		}
		return 1;
	case SIOCGIFCONF:
		if (umove(tcp, tcp->u_arg[2], &ifc) < 0) {
			tprints("???}");
			return 1;
		}
		tprintf("%d, ", ifc.ifc_len);
		if (syserror(tcp)) {
			tprintf("%lx", (unsigned long) ifc.ifc_buf);
		} else if (ifc.ifc_buf == NULL) {
			tprints("NULL");
		} else {
			unsigned int i;
			unsigned int nifra = ifc.ifc_len / sizeof(struct ifreq);
			struct ifreq ifra[nifra];

			if (umoven(tcp, (unsigned long) ifc.ifc_buf,
				sizeof(ifra), ifra) < 0) {
				tprintf("%lx}", (unsigned long) ifc.ifc_buf);
				return 1;
			}
			tprints("{");
			for (i = 0; i < nifra; ++i ) {
				if (i > 0)
					tprints(", ");
				tprints("{");
				print_ifname(ifra[i].ifr_newname);
				tprints(", {");
				if (verbose(tcp)) {
					printxval(addrfams,
						  ifra[i].ifr_addr.sa_family,
						  "AF_???");
					tprints(", ");
					print_addr(tcp, ((long) tcp->u_arg[2]
							 + offsetof(struct ifreq,
								     ifr_addr.sa_data)
							 + ((char *) &ifra[i]
							    - (char *) &ifra[0])),
						   &ifra[i]);
				} else
					tprints("...");
				tprints("}}");
			}
			tprints("}");
		}
		tprints("}");
		return 1;
	default:
		return 0;
	}
}

SYS_FUNC(socketcall)
{
	return printargs(tcp);
}
