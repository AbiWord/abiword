/* AbiSource Program Utilities
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "ut_misc.h"
#include <windows.h>
#include <winsock.h>
#include <nb30.h>
/*!
    UT_gettimeofday() fills in the timeval structure with current
    time; the platform implementation needs to be as accurate as
    possible since this function is used in the UT_UUID class.

    this implementation is from:
    http://bugzilla.vovida.org/bugzilla/show_bug.cgi?id=751
 */
void UT_gettimeofday(struct timeval *tv)
{
    FILETIME ft;
    GetSystemTimeAsFileTime (&ft);

    ULARGE_INTEGER _100ns = {ft.dwLowDateTime,ft.dwHighDateTime};

    _100ns.QuadPart -= 0x19db1ded53e8000;

    tv->tv_sec = long (_100ns.QuadPart / (10000 * 1000));
    tv->tv_usec = (long) ((_100ns.LowPart % (DWORD) (10000 * 1000)) / 10);
}

/*!
    retrieve the 6-byte address of the network card; returns true on success
*/

typedef struct _ASTAT
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER    NameBuff [30];
}ASTAT, * PASTAT;

bool UT_getEthernetAddress(UT_EthernetAddress &A)
{
#if 1
	NCB Ncb;
	UT_uint32 iRet;
	bool bRet = true;
	
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBRESET;
	Ncb.ncb_lana_num = 0;

	iRet = Netbios(&Ncb);
	bRet = (NRC_GOODRET	== iRet);
	UT_DEBUGMSG(("UT_getEthernetAddress: netbios ret1 0x%x\n", iRet));

	if(!bRet)
		return false;
	
	memset(&Ncb, 0, sizeof (Ncb));
	Ncb.ncb_command = NCBASTAT;
	Ncb.ncb_lana_num = 0;

	char namebuf[] = "*               ";
	strcpy((char *)Ncb.ncb_callname, namebuf);

	ASTAT Adapter;
	Ncb.ncb_buffer = (unsigned char *) &Adapter;
	Ncb.ncb_length = sizeof(Adapter);

	iRet = Netbios(&Ncb);
	UT_DEBUGMSG(( "UT_getEthernetAddress: netbios ret2: 0x%x \n", iRet));
	bRet &= (NRC_GOODRET	== iRet);
	
	if(!bRet)
		return false;

	for(UT_uint32 i = 0; i < 6; ++i)
		A[i] = Adapter.adapt.adapter_address[i];

	UT_DEBUGMSG(( "The Ethernet Number is: %02x-%02x-%02x-%02x-%02x-%02x\n",
				Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5] ));

	return true;
#else
	// I am not sure that the netbios is always available; this is the
	// Unix way of getting the address via a socket, but I have not
	// been able to adapt it to win32; I will leave it here for future
	// reference
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
    if (sd < 0)
	{
        return false;
    }
	
    memset(buf, 0, sizeof(buf));
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
	
    if (ioctl (sd, SIOCGIFCONF, (char *)&ifc) < 0)
	{
        close(sd);
        return false;
    }
	
    n = ifc.ifc_len;
	
    for (i = 0; i < n; i+= ifreq_size(*ifr) )
	{
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
    return false;
	
#endif
}

