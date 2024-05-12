#include "defines.h"
#include "network.h"
#include "packet.h"

#include "string.h"
#include "stdio.h"


char* get_ipv4_address(struct sockaddr_in* sockaddr) {
  char ip_address[INET_ADDRSTRLEN]; // Buffer to store IPv4 string representation

  const char* converted_ip = inet_ntop(AF_INET, &(sockaddr->sin_addr), ip_address, INET_ADDRSTRLEN);

  if (converted_ip != NULL) {
    return strdup(ip_address); // Allocate and return a copy of the string
  } else {
    perror("inet_ntop");
    return NULL; // Indicate error (conversion failed)
  }
}

// intended to be used for a new successfull connection
PlayerConnectionInfo make_new_player_info(int index)
{
	PlayerConnectionInfo info = {0};
	info.client_index = index;
	info.connected = true;
	return info;
}

int send_to(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
#ifdef _WIN32
	return sendto(sockfd, buf, len, flags, (const struct sockaddr *)addr, addrlen);
	// WSABUF wsaBuf;
	// wsaBuf.len = len;
	// wsaBuf.buf = (CHAR *)buf; // Cast to match WSABUF definition

	// DWORD bytes_sent;
	// return sendto(sockfd, &wsaBuf, len, &bytes_sent, flags, addr, addrlen, NULL, NULL);
#else
	return sendto(sockfd, buf, len, flags, (const struct sockaddr *)addr, addrlen);
#endif
}

int recv_from(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen) {
#ifdef _WIN32
	WSABUF wsaBuf;
	wsaBuf.len = len;
	wsaBuf.buf = (CHAR *)buf; // Cast to match WSABUF definition

	DWORD bytesRecv;
	int result = WSARecvFrom(sockfd, &wsaBuf, 1, &bytesRecv, &flags, addr, addrlen, NULL, NULL);
	if (result == 0)
	{
		return 0;
	}
	return bytesRecv;
#else
	return recvfrom(sockfd, buf, len, flags, addr, addrlen);
#endif
}
