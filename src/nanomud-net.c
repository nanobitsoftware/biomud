/*
MudClient - Play muds!
Copyright (C) 2003  Michael D. Hayes.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*  NanoMudTelnet.c
*  This file handles all winsock/telnet connection routines
*  as well as any misc. networking routines that will be
*  needed.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include "NanoMud.h"

int last_sent;
BOOL congestion;
extern struct sockaddr_in address;
extern struct hostent* host;
extern bool connected;
extern unsigned long int bufcount;

const char echo_off_str[] =
{
    IAC, WILL, TELOPT_ECHO, '\0'
};
const char echo_on_str[] =
{
    IAC, WONT, TELOPT_ECHO, '\0'
};
const char go_ahead_str[] =
{
    IAC, GA, '\0'
};

void CheckPing()
{
    //return; //For some reason, this is not stable enough to include.
    DWORD     dwTimeSent;
    DWORD     dwElapsed;
    u_char    cTTL;

    int       nRet;

    return;

    //this_session->rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    dwTimeSent = SendEchoRequest(this_session->rawSocket, &this_session->saDest);
    nRet = WaitForEchoReply(this_session->rawSocket);

    if (nRet == SOCKET_ERROR)
        {
            this_session->ping = -1;

            return;
        }
    if (!nRet)
        {
            this_session->ping = -1;

            return;
        }
    dwElapsed = RecvEchoReply(this_session->rawSocket, &this_session->saSrc, &cTTL);
    //  cTTL = NULL;
    //dwElapsed = GetTickCount();
    LOG("Time sent: %d, Elapsed: %d. Total: %d", dwTimeSent, dwElapsed, dwElapsed - dwTimeSent);
    dwElapsed -= dwTimeSent;

    //nRet = closesocket(this_session->rawSocket);
    this_session->ping = dwElapsed;
    return;
}

int SendEchoRequest(SOCKET s, LPSOCKADDR_IN lpstToAddr)
{
    static ECHOREQUEST echoReq;
    static int nId = 1;
    static int nSeq = 1;
    int nRet;

    echoReq.icmpHdr.Type = ICMP_ECHOREQ;
    echoReq.icmpHdr.Code = 0;
    echoReq.icmpHdr.Checksum = 0;
    echoReq.icmpHdr.ID = GetCurrentThreadId(); //nId++;
    echoReq.icmpHdr.Seq = nSeq++;

    //if (congestion == TRUE)
    //  return FALSE;

    for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
        {
            echoReq.cData[nRet] = ' ' + nRet;
        }

    echoReq.dwTime = GetTickCount();

    echoReq.icmpHdr.Checksum = in_cksum((u_short*)&echoReq, sizeof(ECHOREQUEST));

    nRet = sendto(s, (LPSTR)&echoReq, sizeof(ECHOREQUEST), 0, (LPSOCKADDR)lpstToAddr, sizeof(SOCKADDR_IN));
    //  LOG("Sent time: %d", echoReq.dwTime);
    last_sent = echoReq.dwTime;

    //  if (nRet == SOCKET_ERROR)
    //      ReportError("sendto()");
    return (nRet);
}
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char* pTTL)
{
    ECHOREPLY echoReply;
    int nRet;
    int nAddrLen = sizeof(struct sockaddr_in);

    nRet = recvfrom(s, (LPSTR)&echoReply, sizeof(ECHOREPLY), 0, (LPSOCKADDR)lpsaFrom, &nAddrLen);

    //  if (nRet == SOCKET_ERROR)
    //      ReportError("recvfrom()");

    if (echoReply.echoRequest.icmpHdr.ID != GetCurrentThreadId())
        {
            return 0;
        }
    //LOG ("Received: %d", echoReply.echoRequest.dwTime);
    if (echoReply.echoRequest.dwTime != last_sent)
        {
            congestion = TRUE;
            //LOG("Congestion incountered. Skipping packets");
            return last_sent;
        }
    congestion = FALSE;
    *pTTL = echoReply.ipHdr.TTL;
    return(echoReply.echoRequest.dwTime);
}
int WaitForEchoReply(SOCKET s)
{
    struct timeval Timeout;
    fd_set readfds;

    readfds.fd_count = 1;
    readfds.fd_array[0] = s;
    Timeout.tv_sec = 5;
    Timeout.tv_usec = 0;

    return(select(1, &readfds, NULL, NULL, &Timeout));
}
//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *    Mike Muuss
// *    U. S. Army Ballistic Research Laboratory
// *    December, 1983
u_short in_cksum(u_short* addr, int len)
{
    register int nleft = len;
    register u_short* w = addr;
    register u_short answer;
    register int sum = 0;

    /*
    *  Our algorithm is simple, using a 32 bit accumulator (sum),
    *  we add sequential 16 bit words to it, and at the end, fold
    *  back all the carry bits from the top 16 bits into the lower
    *  16 bits.
    */
    while (nleft > 1)
        {
            sum += *w++;
            nleft -= 2;
        }

    /* mop up an odd byte, if necessary */
    if (nleft == 1)
        {
            u_short u = 0;

            *(u_char*)(&u) = *(u_char*)w;
            sum += u;
        }

    /*
    * add back carry outs from top 16 bits to low 16 bits
    */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}

int CloseWinsock(HWND hwnd, char* HostName)
{
    hwnd = 0;
    HostName = 0;
    give_term_echo("Disconnected from %s:%d...", this_session->host, this_session->port);
    update_term();
    connected = FALSE;
    return 0;
}

/* Handlewinsock will take all connection information and attempt
* to make the connection. If successful, it passes it along. If not
* it will attempt to fail gracefully. Or dis-gracefully, as seen in the
* past.
*/

int HandleWinsockConnection(HWND hwnd, char* HostName)
{
    char tt[100] = "";
    int err;

    LPHOSTENT lpHost;

    address.sin_family = AF_INET;
    address.sin_port = htons(this_session->port);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
            exit(0);
        }
    if ((host = gethostbyname(HostName)) == NULL)
        {
            give_term_error("Unable to find mud! %s", HostName);
            return 0;
        }

    address.sin_addr.s_addr = *((unsigned long*)host->h_addr);
    if ((this_session->desc = connect(sock, (struct sockaddr*) &address, sizeof(address))) != 0)
        {
            MessageBox(hwnd, "Unable to connect.", "Error", MB_OK);
            return 0;
        }
    this_session->rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    /*  if (this_session->rawSocket == SOCKET_ERROR)
        {
            GiveError("Unable to create raw socket in CheckPing().",FALSE);
            return 0;
        }
    */
    lpHost = gethostbyname(this_session->host);
    if (lpHost == NULL)
        {
            return 0;
        }
    this_session->saDest.sin_addr.s_addr = *((u_long FAR*) (lpHost->h_addr));
    this_session->saDest.sin_family = AF_INET;
    this_session->saDest.sin_port = 0;

    //WindowProcedure (MudMain, WM_TIMER,0,0); /* Cheap hack for lazyness */
    this_session->connected = TRUE;

    give_term_echo("Connected to %s:%d...", this_session->host, this_session->port);
    update_term();

    //    sprintf(tt,"%s[CIP;68.62.139.144\n", "\033");
    err = 0;
    //    err = send(sock,tt,strlen(tt),0);
    WSAAsyncSelect(sock, MudMain, WM_SOCKET, FD_READ | FD_CLOSE);
    connected = TRUE;

    return 0;
}

/* Main check ping code -- threaded.
* We keep this treaded so that a blocking on the ping code
* cannot, and will not, upset the updating and speed of the
* mud client. As much as I do not like threading when not
* necessary, I believe this one is necessary.
*/
DWORD WINAPI check_ping(void)
{
    char buf[1228] = "";
    char tt_char[1000] = "";
    char ping_string[100];
    char b_buf[100] = "";
    int b_type = 0;
    float t_total = 0.0;

    extern unsigned long long int total_alloc;
    struct timeval timeout;
    fd_set set;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    //this_session->rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    while (1000)
        {
            //update_term();
            sleep(1000);
            //timeout.tv_sec = 1000;
            //timeout.tv_usec = 0;

            //FD_ZERO(&set);
            //FD_SET(100, &set);

            //select(FD_SETSIZE, &set, NULL, NULL, &timeout);
            //sleep(2000);
            //        char temp[1000];

            t_total = (float)total_alloc;
            //update_term();

            if (t_total > 1000)
                {
                    b_type = 1; // KB
                    t_total = t_total / 1000;
                }
            if (t_total > 1000)
                {
                    b_type = 2; // MB
                    t_total = t_total / 1000;
                }
            if (t_total > 1000)
                {
                    b_type = 3; // GB
                    t_total = t_total / 1000;
                }

            CheckPing();
            //commaize(t_total,tt_char);
            commaize(bufcount, b_buf);
            sprintf(ping_string, "%d", this_session->ping);

            //sprintf(temp, "%u", (unsigned int)this_session->ping);
            sprintf(buf, "BioMud - (Ping:%s%s)(Buffer: %s)(Memory %3.6f %s)",
                    this_session->ping == -1 ? "N/A" : ping_string, this_session->ping != -1 ? "ms" : "", b_buf, t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes");

            SendMessage(MudMain, WM_SETTEXT, /*strlen(buf)-2*/0, (LPARAM)(LPCSTR)buf);

            update_status();
            TERM_READY = TRUE;
        }

    return 0;
}

/* Initialize the main winsock mode. In the current
* implementation, it will only handle one connection
* at a time. In future revisions, this will be changed
* with the addition of classes.
*/
void initwinsock()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0)
        {
            GiveError("Unable to find winsock.", 1);
        }

    return;
}