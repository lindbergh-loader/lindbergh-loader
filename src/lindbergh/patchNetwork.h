#pragma once

#include <stdio.h>
#include <unistd.h>

void hummerSetIPs(size_t firstAddress);
int rtunedGetCabs();
void rtunedGetLinkIdandCabs();
void rtunedSetIPs();
int rtunedBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);