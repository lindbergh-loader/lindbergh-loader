#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"
#include "log.h"
#include "flowControl.h"

extern uint32_t gId;
char *rtunedIPs[4];
char *rtunedSortedIPs[4];
u_int8_t rtunedLinkId = 0;
uint8_t rtunedCabs = 0;

void hummerSetIPs(size_t firstAddress)
{
    if (!getConfig()->enableNetworkPatches)
        return;

    if (strcmp(getConfig()->IpCab1, "") != 0)
        setVariable(firstAddress, (size_t)(void *)getConfig()->IpCab1);
    if (strcmp(getConfig()->IpCab2, "") != 0)
        setVariable(firstAddress + 4, (size_t)(void *)getConfig()->IpCab2);
    if (strcmp(getConfig()->IpCab3, "") != 0)
        setVariable(firstAddress + 8, (size_t)(void *)getConfig()->IpCab3);
    if (strcmp(getConfig()->IpCab4, "") != 0)
        setVariable(firstAddress + 12, (size_t)(void *)getConfig()->IpCab4);
}

int rtunedGetCabs()
{
    return rtunedCabs;
}

void rtunedGetLinkIdandCabs()
{
    FILE *f = fopen(getConfig()->sramPath, "rb");
    if (f == NULL)
        return;
    fseek(f, 0xc011, SEEK_SET);
    fread(&rtunedLinkId, 1, 1, f);
    fseek(f, 0xc019, SEEK_SET);
    fread(&rtunedCabs, 1, 1, f);
    fclose(f);
}

void rtunedRotateIps(char *ori[], char *dst[], int total, int pos)
{
    for (int i = 0; i < total; i++)
    {
        dst[i] = ori[(pos + i) % total];
    }
    dst[total - 1] = ori[pos - 1];
}

void rtunedSetIPs()
{
    if (!getConfig()->enableNetworkPatches)
        return;

    for (int x = 0; x < 4; x++)
    {
        rtunedIPs[x] = "0.0.0.0";
        rtunedSortedIPs[x] = "0.0.0.0";
    }

    uint8_t setIps = 0;
    if (strcmp(getConfig()->IpCab1, "") != 0)
    {
        rtunedIPs[0] = getConfig()->IpCab1;
        setIps++;
    }
    if (strcmp(getConfig()->IpCab2, "") != 0)
    {
        rtunedIPs[1] = getConfig()->IpCab2;
        setIps++;
    }
    if (strcmp(getConfig()->IpCab3, "") != 0)
    {
        rtunedIPs[2] = getConfig()->IpCab3;
        setIps++;
    }
    if (strcmp(getConfig()->IpCab4, "") != 0)
    {
        rtunedIPs[3] = getConfig()->IpCab4;
        setIps++;
    }
    if (setIps != rtunedCabs)
        log_warn("Warning: You have %d IPs set in the config file and %d Cabinets set in the TestMenu.", setIps, rtunedCabs);

    rtunedRotateIps(rtunedIPs, rtunedSortedIPs, rtunedCabs, rtunedLinkId);
}

int rtunedBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (rtunedCabs > 1)
    {
        int idx1 = 0;
        int idx2 = 0;
        for (int i = 0; i < rtunedCabs; i++)
        {
            setVariable(0x0851e890 + idx1, inet_addr(rtunedSortedIPs[i]));
            setVariable(0x0851d7c0 + idx2, inet_addr(rtunedIPs[i]));
            idx1 += 16;
            idx2 += 4;
        }
        uint32_t ip = inet_addr(rtunedIPs[rtunedLinkId - 1]);
        uint32_t invIp = ((ip & 0xFF) << 24) | ((ip & 0xFF00) << 8) | ((ip & 0xFF0000) >> 8) | ((ip & 0xFF000000) >> 24);
        uint32_t invIpBcast = ((ip & 0xFF) << 24) | ((ip & 0xFF00) << 8) | ((ip & 0xFF0000) >> 8) | 0x000000FF;
        setVariable(0x0851bf90, invIp);
        setVariable(0x0851bf98, invIpBcast);
    }
    return bind(sockfd, addr, addrlen);
}

int inet_aton(const char *cp, struct in_addr *inp)
{
    int (*_inet_aton)(const char *cp, struct in_addr *inp) = dlsym(RTLD_NEXT, "inet_aton");

    if (gId == HARLEY_DAVIDSON)
    {
        if (getConfig()->enableNetworkPatches)
        {
            if ((strcmp(cp, "192.168.1.1") == 0 || strcmp(cp, "192.168.1.5") == 0 || strcmp(cp, "192.168.1.9") == 0))
            {
                if (strcmp(getConfig()->IpCab1, "") != 0)
                    return _inet_aton(getConfig()->IpCab1, inp);
                else
                    return _inet_aton("127.0.0.1", inp);
            }
            if ((strcmp(cp, "192.168.1.2") == 0 || strcmp(cp, "192.168.1.7") == 0 || strcmp(cp, "192.168.1.10") == 0) &&
                strcmp(getConfig()->IpCab2, "") != 0)
            {
                return _inet_aton(getConfig()->IpCab2, inp);
            }
            if ((strcmp(cp, "192.168.1.3") == 0 || strcmp(cp, "192.168.1.7") == 0 || strcmp(cp, "192.168.1.11") == 0) &&
                strcmp(getConfig()->IpCab3, "") != 0)
            {
                return _inet_aton(getConfig()->IpCab4, inp);
            }
            if ((strcmp(cp, "192.168.1.4") == 0 || strcmp(cp, "192.168.1.8") == 0 || strcmp(cp, "192.168.1.12") == 0) &&
                strcmp(getConfig()->IpCab4, "") != 0)
            {
                return _inet_aton(getConfig()->IpCab4, inp);
            }
        }
        else
        {
            return _inet_aton("127.0.0.1", inp);
        }
    }
    return _inet_aton(cp, inp);
}

in_addr_t inet_addr(const char *cp)
{
    in_addr_t (*_inet_addr)(const char *cp) = dlsym(RTLD_NEXT, "inet_addr");

    if (getConfig()->enableNetworkPatches)
    {
        if (gId == TOO_SPICY)
        {
            if (strcmp("10.0.0.1", cp) == 0 && strcmp(getConfig()->tooSpicyIpCab1, "") != 0)
            {
                cp = getConfig()->tooSpicyIpCab1;
            }
            else if (strcmp("10.0.0.2", cp) == 0 && strcmp(getConfig()->tooSpicyIpCab2, "") != 0)
            {
                cp = getConfig()->tooSpicyIpCab2;
            }
        }
        else if (gId == SEGA_RACE_TV)
        {
            // Standalone = 4, Main = 0, 2-Sub = 1, 3-Sub = 2 and 4-Sub = 3
            if (strcmp("192.168.49.1", cp) == 0 || strcmp("192.168.49.2", cp) == 0 || strcmp("192.168.49.3", cp) == 0 ||
                strcmp("192.168.49.4", cp) == 0)
            {
                if (strcmp(getConfig()->srtvIP, "") != 0)
                    cp = getConfig()->srtvIP;
                else
                    // Prevents the game to crash if the IP does not match the PC's IP.
                    cp = "127.0.0.1";
            }
        }
    }
    return _inet_addr(cp);
}

char *strstr(const char *haystack, const char *needle)
{
    char *(*_strstr)(const char *__haystack, const char *__needle) = dlsym(RTLD_NEXT, "strstr");

    if (strcmp(needle, "eth0") == 0 && getConfig()->enableNetworkPatches && strcmp(getConfig()->nicName, "") != 0)
    {
        return _strstr(haystack, getConfig()->nicName);
    }
    return _strstr(haystack, needle);
}

int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop)
{
    int (*_gethostbyname_r)(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop) =
        dlsym(RTLD_NEXT, "gethostbyname_r");

    void *addr = __builtin_return_address(0);
    if (gId == QUIZ_AXA && addr == (void *)0x0832609d)
    {
        ret = 0;
        buf[0] = '\0';
        result = 0;
        h_errnop = 0;
        return -1;
    }
    return _gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}
