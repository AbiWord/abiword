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
#include <snmp.h>
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

typedef BOOL(WINAPI * pSnmpExtensionInit) (IN DWORD dwTimeZeroReference,
										   OUT HANDLE * hPollForTrapEvent,
										   OUT AsnObjectIdentifier * supportedView);

typedef BOOL(WINAPI * pSnmpExtensionTrap) (OUT AsnObjectIdentifier * enterprise,
										   OUT AsnInteger * genericTrap,
										   OUT AsnInteger * specificTrap,
										   OUT AsnTimeticks * timeStamp,
										   OUT RFC1157VarBindList * variableBindings);

typedef BOOL(WINAPI * pSnmpExtensionQuery) (IN BYTE requestType,
											IN OUT RFC1157VarBindList * variableBindings,
											OUT AsnInteger * errorStatus,
											OUT AsnInteger * errorIndex);

typedef BOOL(WINAPI * pSnmpExtensionInitEx) (OUT AsnObjectIdentifier * supportedView);



bool UT_getEthernetAddress(UT_EthernetAddress &A)
{
	// the following code by James Marsh <James.Marsh@sandtechnology.com>
	// was found at http://tangentsoft.net/wskfaq/examples/getmac-snmp.html
    WSADATA WinsockData;
    if(WSAStartup(MAKEWORD(2, 0), &WinsockData) != 0)
	{
		UT_DEBUGMSG(("UT_getEthernetAddress (win32): need Winsock 2.x!\n"));
		goto  try_netbios;
    }
	
	{
		HINSTANCE            m_hInst;
		pSnmpExtensionInit   m_Init;
		pSnmpExtensionInitEx m_InitEx;
		pSnmpExtensionQuery  m_Query;
		pSnmpExtensionTrap   m_Trap;
		HANDLE               PollForTrapEvent;
		AsnObjectIdentifier  SupportedView;

		UINT OID_ifEntryType[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 3};
		UINT OID_ifEntryNum[] = {1, 3, 6, 1, 2, 1, 2, 1};
		UINT OID_ipMACEntAddr[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 6};
	
		AsnObjectIdentifier MIB_ifMACEntAddr = {sizeof(OID_ipMACEntAddr)/sizeof(UINT),
												OID_ipMACEntAddr};
	
		AsnObjectIdentifier MIB_ifEntryType = {sizeof(OID_ifEntryType)/sizeof(UINT),
											   OID_ifEntryType};
	
		AsnObjectIdentifier MIB_ifEntryNum = {sizeof(OID_ifEntryNum)/sizeof(UINT),
											  OID_ifEntryNum};
	
		RFC1157VarBindList  varBindList;
		RFC1157VarBind      varBind[2];
		AsnInteger          errorStatus;
		AsnInteger          errorIndex;
		AsnObjectIdentifier MIB_NULL = {0,0};
	
		int ret;
		int dtmp;
		int i = 0, j = 0;
		bool bFound = false;
	
		m_Init = NULL;
		m_InitEx = NULL;
		m_Query = NULL;
		m_Trap = NULL;

		/* Load the SNMP dll and get the addresses of the functions
		   necessary */
		m_hInst = LoadLibrary("inetmib1.dll");
		if(m_hInst < (HINSTANCE) HINSTANCE_ERROR)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress (win32): could not load inetmib1.dll\n"));
			m_hInst = NULL;
			goto  try_netbios;
		}
	
		m_Init =   (pSnmpExtensionInit)   GetProcAddress(m_hInst, "SnmpExtensionInit");
		m_InitEx = (pSnmpExtensionInitEx) GetProcAddress(m_hInst, "SnmpExtensionInitEx");
		m_Query =  (pSnmpExtensionQuery)  GetProcAddress(m_hInst, "SnmpExtensionQuery");
		m_Trap =   (pSnmpExtensionTrap)   GetProcAddress(m_hInst, "SnmpExtensionTrap");
	
		m_Init(GetTickCount(), &PollForTrapEvent, &SupportedView);

		/* Initialize the variable list to be retrieved by m_Query */
		varBindList.list = varBind;
		varBind[0].name  = MIB_NULL;
		varBind[1].name  = MIB_NULL;

		/* Copy in the OID to find the number of entries in the
		   Inteface table */
		varBindList.len = 1;        /* Only retrieving one item */
		SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryNum);
	
		ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
	
		UT_DEBUGMSG(("UT_getEthernetAddress (win32): adapters in this system : %i\n",
					 varBind[0].value.asnValue.number));
	
		varBindList.len = 2;

		/* Copy in the OID of ifType, the type of interface */
		SNMP_oidcpy(&varBind[0].name, &MIB_ifEntryType);

		/* Copy in the OID of ifPhysAddress, the address */
		SNMP_oidcpy(&varBind[1].name, &MIB_ifMACEntAddr);

		do {

			/* Submit the query.  Responses will be loaded into varBindList.
			   We can expect this call to succeed a # of times corresponding
			   to the # of adapters reported to be in the system */
			ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
		
			if(!ret)
			{
				ret = 1;
			}
			else
			{
				/* Confirm that the proper type has been returned */
				ret = SNMP_oidncmp(&varBind[0].name, &MIB_ifEntryType, MIB_ifEntryType.idLength);
			}
		
			if (!ret)
			{
				j++;
				dtmp = varBind[0].value.asnValue.number;
				UT_DEBUGMSG(("UT_getEthernetAddress (win32): Interface #%i type : %i\n", j, dtmp));

				/* Type 6 describes ethernet interfaces */
				if(dtmp == 6)
				{
					/* Confirm that we have an address here */
					ret = SNMP_oidncmp(&varBind[1].name,
									   &MIB_ifMACEntAddr,
									   MIB_ifMACEntAddr.idLength);
				
					if((!ret) && (varBind[1].value.asnValue.address.stream != NULL))
					{
						if(   (varBind[1].value.asnValue.address.stream[0] == 0x44)
						   && (varBind[1].value.asnValue.address.stream[1] == 0x45)
						   && (varBind[1].value.asnValue.address.stream[2] == 0x53)
						   && (varBind[1].value.asnValue.address.stream[3] == 0x54)
						   && (varBind[1].value.asnValue.address.stream[4] == 0x00))
						{

							/* Ignore all dial-up networking adapters */
							UT_DEBUGMSG(("UT_getEthernetAddress (win32): Interface #%i is a DUN adapter\n", j));
							continue;
						}
						if(   (varBind[1].value.asnValue.address.stream[0] == 0x00)
						   && (varBind[1].value.asnValue.address.stream[1] == 0x00)
						   && (varBind[1].value.asnValue.address.stream[2] == 0x00)
						   && (varBind[1].value.asnValue.address.stream[3] == 0x00)
						   && (varBind[1].value.asnValue.address.stream[4] == 0x00)
						   && (varBind[1].value.asnValue.address.stream[5] == 0x00))
						{

							/* Ignore NULL addresses returned by other network
							   interfaces */
							UT_DEBUGMSG(("UT_getEthernetAddress (win32): Interface #%i is "
										 "a NULL address\n", j));
							continue;
						}

						for(UT_uint32 k = 0; k < 6; ++k)
							A[k] = varBind[1].value.asnValue.address.stream[k];
					
						UT_DEBUGMSG(("UT_getEthernetAddress : MAC Address of interface #%i: "
									 "%02x.%02x.%02x.%02x.%02x.%02x\n",
									 j,A[0],A[1],A[2],A[3],A[4],A[5]));

						// we only need the first address we find ...
						bFound = true;
						break;
					} // if ((!ret) && (varBind[1].
				} // if (dtmp == 6)
			} //if (!ret) # 2
		}
		while (!ret);         /* Stop only on an error.  An error will occur
								 when we go exhaust the list of interfaces to
								 be examined */

		/* Free the bindings */
		SNMP_FreeVarBind(&varBind[0]);
		SNMP_FreeVarBind(&varBind[1]);

		if(bFound)
			return true;
	}
	
 try_netbios:
	// this is the method described in MS win32 docs; it has one
	// fundamental shortcoming: not everyone uses netbios
	{
		UT_DEBUGMSG(("UT_getEthernetAddress (win32): unable to use SNMP\n"));
		NCB Ncb;
		UT_uint32 iRet;
		bool bRet = true;
	
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = 0;

		iRet = Netbios(&Ncb);
		bRet = (NRC_GOODRET	== iRet);

		if(!bRet)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress (win32): unable to use NETBIOS either\n"));
			return false;
		}
		
		memset(&Ncb, 0, sizeof (Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = 0;

		char namebuf[] = "*               ";
		strcpy((char *)Ncb.ncb_callname, namebuf);

		ASTAT Adapter;
		Ncb.ncb_buffer = (unsigned char *) &Adapter;
		Ncb.ncb_length = sizeof(Adapter);

		iRet = Netbios(&Ncb);
		bRet &= (NRC_GOODRET	== iRet);
	
		if(!bRet)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress (win32): unable to use NETBIOS either\n"));
			return false;
		}
		
		for(UT_uint32 i = 0; i < 6; ++i)
			A[i] = Adapter.adapt.adapter_address[i];

		UT_DEBUGMSG(("MAC Address is %02x-%02x-%02x-%02x-%02x-%02x\n",
					 A[0],A[1],A[2],A[3],A[4],A[5]));

		return true;
	}
}

