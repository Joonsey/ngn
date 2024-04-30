#include <minwindef.h>
#include <minwinbase.h>
#define SOCKET int
#define INADDR_ANY (unsigned long)0x00000000
#define INADDR_LOOPBACK 0x7f000001
#define INADDR_BROADCAST (u_long)0xffffffff
#define INADDR_NONE 0xffffffff
#define AF_INET 2
#define SOCK_DGRAM 2
#define WSADESCRIPTION_LEN	256
#define WSASYS_STATUS_LEN	128
typedef struct WSAData {
	WORD		wVersion;
	WORD		wHighVersion;
	unsigned short	iMaxSockets;
	unsigned short	iMaxUdpDg;
	char		*lpVendorInfo;
	char		szDescription[WSADESCRIPTION_LEN+1];
	char		szSystemStatus[WSASYS_STATUS_LEN+1];
} WSADATA, *LPWSADATA;
struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct sockaddr {
	unsigned short	sa_family;
	char	sa_data[14];
};

#define ADDR_ANY INADDR_ANY
typedef struct _WSABUF {
long len;
char *buf;
} WSABUF,*LPWSABUF;
typedef struct _OVERLAPPED *LPWSAOVERLAPPED;
typedef void (CALLBACK *LPWSAOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwError,DWORD cbTransferred,LPWSAOVERLAPPED lpOverlapped,DWORD dwFlags);
int sendto(SOCKET s,const char *buf,int len,int flags,const struct sockaddr *to,int tolen);
int closesocket(SOCKET s);
int bind(SOCKET s,const struct sockaddr *name,int namelen);
int WSACleanup(void);
int WSAStartup(WORD wVersionRequested,LPWSADATA lpWSAData);
int WSAGetLastError(void);
int WSARecvFrom(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags,struct sockaddr *lpFrom,LPINT lpFromlen,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
unsigned long inet_addr(const char *cp);
unsigned long htonl(unsigned long hostlong);
unsigned short htons(unsigned short hostshort);
unsigned long ntohl(unsigned long netlong);
unsigned short ntohs(unsigned short netshort);
#define socklen_t size_t
#define sleep _sleep