/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_misc.h"
#include "ut_assert.h"
#include <sys/time.h>

/*!
    UT_gettimeofday() fills in the timeval structure with current
    time; the platform implementation needs to be as accurate as
    possible since this function is used in the UT_UUID class.
 */
void UT_gettimeofday(struct timeval *tv)
{
	gettimeofday(tv,NULL);
}

/*!
    retrieve the 6-byte address of the network card; returns true on success
    This implementation is from libuuid; it has not been debugged or
    even compiled
*/
bool UT_getEthernetAddress(UT_EthernetAddress &a)
{
	UT_UNUSED(a);
#if 0
	// TODO -- someone should debug this and turn it on
#ifdef HAVE_NET_IF_H
    int         sd;
    struct ifreq    ifr, *ifrp;
    struct ifconf   ifc;
    char buf[1024];
    int     n, i;
    unsigned char   *a;
    
/*
 * BSD 4.4 defines the size of an ifreq to be
 * max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
 * However, under earlier systems, sa_len isn't present, so the size is 
 * just sizeof(struct ifreq)
 */
#ifdef HAVE_SA_LEN
#define ifreq_size(i) max(sizeof(struct ifreq),\
     sizeof((i).ifr_name)+(i).ifr_addr.sa_len)
#else
#define ifreq_size(i) sizeof(struct ifreq)
#endif /* HAVE_SA_LEN*/

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sd < 0) {
        return false;
    }
    memset(buf, 0, sizeof(buf));
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl (sd, SIOCGIFCONF, (char *)&ifc) < 0) {
        close(sd);
        return false;
    }
    n = ifc.ifc_len;
    for (i = 0; i < n; i+= ifreq_size(*ifr) ) {
        ifrp = (struct ifreq *)((char *) ifc.ifc_buf+i);
        strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
        if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
            continue;
        a = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
#else
#ifdef SIOCGENADDR
        if (ioctl(sd, SIOCGENADDR, &ifr) < 0)
            continue;
        a = (unsigned char *) ifr.ifr_enaddr;
#else
        /*
         * XXX we don't have a way of getting the hardware
         * address
         */
        close(sd);
        return false;
#endif /* SIOCGENADDR */
#endif /* SIOCGIFHWADDR */
        if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5])
            continue;
        if (node_id) {
            memcpy(node_id, a, 6);
            close(sd);
            return true;
        }
    }
    close(sd);
#endif
#else
	// someone needs to debug/fix, ect., the code above
	UT_ASSERT(UT_NOT_IMPLEMENTED);
#endif
    return false;
}

