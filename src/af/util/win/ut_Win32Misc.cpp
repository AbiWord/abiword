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
#include "ut_debugmsg.h"

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


typedef struct _ASTAT
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER    NameBuff [30];
}ASTAT, * PASTAT;

typedef BOOL(WINAPI * pSnmpExtensionInit) (IN DWORD dwTimeZeroReference,
										   OUT HANDLE * hPollForTrapEvent,
										   OUT AsnObjectIdentifier * supportedView);

typedef BOOL(WINAPI * pSnmpExtensionQuery) (IN BYTE requestType,
											IN OUT RFC1157VarBindList * variableBindings,
											OUT AsnInteger * errorStatus,
											OUT AsnInteger * errorIndex);

typedef VOID (WINAPI * pSnmpUtilVarBindFree) (RFC1157VarBind *VarBind);

typedef SNMPAPI (WINAPI * pSnmpUtilOidNCmp )(AsnObjectIdentifier *ObjIdA,
											 AsnObjectIdentifier *ObjIdB,
											 UINT Len);

typedef SNMPAPI (WINAPI * pSnmpUtilOidCpy) (AsnObjectIdentifier *DestObjId,
											AsnObjectIdentifier *SrcObjId);

typedef int (WINAPI * pWSAStartup) (WORD wVersionRequested,LPWSADATA lpWSAData);

typedef UCHAR (WINAPI * pNetbios) (PNCB pncb);


/*!
    retrieve the 6-byte address of the network card; returns true on success
*/
bool UT_getEthernetAddress(UT_EthernetAddress &A)
{
	// the following code by James Marsh <James.Marsh@sandtechnology.com>
	// was found at http://tangentsoft.net/wskfaq/examples/getmac-snmp.html
	// I adjusted it, so all the libs are dynamically loaded and unloaded

	HINSTANCE m_hWSInst = NULL;
	m_hWSInst = LoadLibrary("ws2_32.dll");
	pWSAStartup m_WSAStartup = NULL;
	
	if(m_hWSInst < (HINSTANCE) HINSTANCE_ERROR)
	{
		UT_DEBUGMSG(("UT_getEthernetAddress: could not load ws2_32.dll\n"));
		m_hWSInst = NULL;
		goto  try_netbios;
	}
	else
	{
		m_WSAStartup = (pWSAStartup)   GetProcAddress(m_hWSInst, "WSAStartup");
		if(!m_WSAStartup)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: no WSAStartup\n"));
			FreeLibrary(m_hWSInst);
		}
	}
	
	{
		
		WSADATA WinsockData;
		if(m_WSAStartup(MAKEWORD(2, 0), &WinsockData) != 0)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: need Winsock 2.x!\n"));
			FreeLibrary(m_hWSInst);
			goto  try_netbios;
		}

		HINSTANCE            m_hSNMPInst = NULL;
		pSnmpUtilVarBindFree m_SnmpUtilVarBindFree = NULL;
		pSnmpUtilOidNCmp     m_SnmpUtilOidNCmp = NULL;
		pSnmpUtilOidCpy      m_SnmpUtilOidCpy = NULL;

		
		HINSTANCE            m_hInst = NULL;
		pSnmpExtensionInit   m_Init = NULL;
		pSnmpExtensionQuery  m_Query = NULL;
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

		/* Load the SNMP dll and get the addresses of the functions
		   necessary */
		m_hSNMPInst = LoadLibrary("snmpapi.dll");
		if(m_hSNMPInst < (HINSTANCE) HINSTANCE_ERROR)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: could not load snmpapi.dll\n"));
			m_hSNMPInst = NULL;
			goto  try_netbios;
		}

		m_SnmpUtilVarBindFree = (pSnmpUtilVarBindFree) GetProcAddress(m_hSNMPInst,
																	  "SnmpUtilVarBindFree");
		
		m_SnmpUtilOidNCmp = (pSnmpUtilOidNCmp) GetProcAddress(m_hSNMPInst,
																	  "SnmpUtilOidNCmp");
		
		m_SnmpUtilOidCpy = (pSnmpUtilOidCpy) GetProcAddress(m_hSNMPInst,
																	  "SnmpUtilOidCpy");
		
		
		if(!m_SnmpUtilVarBindFree || !m_SnmpUtilOidNCmp || !m_SnmpUtilOidCpy)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress (win32): could not load SNMP functions\n"));
			FreeLibrary(m_hSNMPInst);
			goto try_netbios;
		}


		// load the SNMP extension library
		m_hInst = LoadLibrary("inetmib1.dll");

		if(m_hInst < (HINSTANCE) HINSTANCE_ERROR)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: could not load inetmib1.dll\n"));
			m_hInst = NULL;
			FreeLibrary(m_hSNMPInst);
			goto  try_netbios;
		}
	
		m_Init =   (pSnmpExtensionInit)   GetProcAddress(m_hInst, "SnmpExtensionInit");
		m_Query =  (pSnmpExtensionQuery)  GetProcAddress(m_hInst, "SnmpExtensionQuery");

		if(!m_Init || !m_Query)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress (win32): could not load SNMP ext functions\n"));
			FreeLibrary(m_hInst);
			FreeLibrary(m_hSNMPInst);
			goto try_netbios;
		}
		
		m_Init(GetTickCount(), &PollForTrapEvent, &SupportedView);

		/* Initialize the variable list to be retrieved by m_Query */
		varBindList.list = varBind;
		varBind[0].name  = MIB_NULL;
		varBind[1].name  = MIB_NULL;

		/* Copy in the OID to find the number of entries in the
		   Inteface table */
		varBindList.len = 1;        /* Only retrieving one item */
		m_SnmpUtilOidCpy(&varBind[0].name, &MIB_ifEntryNum);
	
		ret = m_Query(ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex);
	
		UT_DEBUGMSG(("UT_getEthernetAddress: adapters in this system : %i\n",
					 varBind[0].value.asnValue.number));
	
		varBindList.len = 2;

		/* Copy in the OID of ifType, the type of interface */
		m_SnmpUtilOidCpy(&varBind[0].name, &MIB_ifEntryType);

		/* Copy in the OID of ifPhysAddress, the address */
		m_SnmpUtilOidCpy(&varBind[1].name, &MIB_ifMACEntAddr);

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
				ret = m_SnmpUtilOidNCmp(&varBind[0].name,
										&MIB_ifEntryType,
										MIB_ifEntryType.idLength);
			}
		
			if (!ret)
			{
				j++;
				dtmp = varBind[0].value.asnValue.number;
				UT_DEBUGMSG(("UT_getEthernetAddress (win32): Interface #%i type : %i\n"
							 , j, dtmp));

				/* Type 6 describes ethernet interfaces */
				if(dtmp == 6)
				{
					/* Confirm that we have an address here */
					ret = m_SnmpUtilOidNCmp(&varBind[1].name,
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
							UT_DEBUGMSG(("UT_getEthernetAddress: #%i is a DUN adapter\n", j));
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
		m_SnmpUtilVarBindFree(&varBind[0]);
		m_SnmpUtilVarBindFree(&varBind[1]);
		
		FreeLibrary(m_hInst);
		FreeLibrary(m_hSNMPInst);
		FreeLibrary(m_hWSInst);

		if(bFound)
			return true;
	}

 try_netbios:
	// this is the method described in MS win32 docs; it has one
	// fundamental shortcoming: not everyone uses netbios, so it is
	// here as a fall back in case the previous method does not work
	// I have adjusted the MS code so as to load and unload the dlls
	// dynamically
	{
		UT_DEBUGMSG(("UT_getEthernetAddress: unable to use SNMP\n"));

		HINSTANCE m_hInst;

		m_hInst = LoadLibrary("netapi32.dll");
		if(m_hInst < (HINSTANCE) HINSTANCE_ERROR)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: could not load netapi32.dll\n"));
			return false;
		}

		pNetbios m_Netbios = NULL;
		m_Netbios = (pNetbios) GetProcAddress(m_hInst, "Netbios");

		if(!m_Netbios)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: could load netbios functions\n"));
			FreeLibrary(m_hInst);
			return false;
		}
		
		NCB Ncb;
		UT_uint32 iRet;
		bool bRet = true;
	
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = 0;

		iRet = m_Netbios(&Ncb);
		bRet = (NRC_GOODRET	== iRet);

		if(!bRet)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: unable to use NETBIOS either\n"));
			FreeLibrary(m_hInst);
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

		iRet = m_Netbios(&Ncb);
		bRet &= (NRC_GOODRET	== iRet);
	
		if(!bRet)
		{
			UT_DEBUGMSG(("UT_getEthernetAddress: unable to use NETBIOS2 either\n"));
			FreeLibrary(m_hInst);
			return false;
		}
		
		for(UT_uint32 i = 0; i < 6; ++i)
			A[i] = Adapter.adapt.adapter_address[i];

		UT_DEBUGMSG(("MAC Address is %02x-%02x-%02x-%02x-%02x-%02x\n",
					 A[0],A[1],A[2],A[3],A[4],A[5]));

		FreeLibrary(m_hInst);
		return true;
	}
}

