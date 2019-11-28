#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

/* to broadcast the message, this mac is used. */
#define MY_DEST_MAC0	0xff
#define MY_DEST_MAC1	0xff
#define MY_DEST_MAC2	0xff
#define MY_DEST_MAC3	0xff
#define MY_DEST_MAC4	0xff
#define MY_DEST_MAC5	0xff

/* assigned name to ethernet port.*/
#define DEFAULT_IF	"enp0s17"

/* due to defined protocol, this buffersize has been chosen.*/
#define BUF_SIZ		1024

/* hello packet is defined.*/
struct pkt
{
	uint8_t type;
	uint16_t len;
	char nickname[64];

}__attribute__((packed));
/* message packet is defined.*/
struct msg_pkt
{
	uint8_t type;
	uint16_t length;
	char nickname[64];
}__attribute__((packed));


int main(int argc, char *argv[])
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];



	/* Get interface name */

	strcpy(ifName, DEFAULT_IF);

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");

	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	eh->ether_dhost[0] = MY_DEST_MAC0;
	eh->ether_dhost[1] = MY_DEST_MAC1;
	eh->ether_dhost[2] = MY_DEST_MAC2;
	eh->ether_dhost[3] = MY_DEST_MAC3;
	eh->ether_dhost[4] = MY_DEST_MAC4;
	eh->ether_dhost[5] = MY_DEST_MAC5;

	/* Ethertype field */
	eh->ether_type = htons(0xdeaf);
	tx_len += sizeof(struct ether_header);

	int tx_len_org=0;
	tx_len_org=tx_len;

	struct pkt hello_message;
	hello_message.type=atoi(argv[2]);
	hello_message.len=strlen(argv[1]);
	strcpy(hello_message.nickname, argv[1])	;

	/* Packet data */
	memcpy(&sendbuf[tx_len],&hello_message, sizeof(hello_message));
	tx_len+=sizeof(hello_message);


	struct msg_pkt newpacket;

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;

	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;

	/* Destination MAC */
	socket_address.sll_addr[0] = MY_DEST_MAC0;
	socket_address.sll_addr[1] = MY_DEST_MAC1;
	socket_address.sll_addr[2] = MY_DEST_MAC2;
	socket_address.sll_addr[3] = MY_DEST_MAC3;
	socket_address.sll_addr[4] = MY_DEST_MAC4;
	socket_address.sll_addr[5] = MY_DEST_MAC5;

	/* Send packet */
	if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

	/* while loop for taking message*/
	while (1) {

		tx_len=tx_len_org;
		printf("pls type your message\n");
		memset(&newpacket,0,sizeof(newpacket));
		fgets(newpacket.nickname,sizeof(newpacket.nickname),stdin);
		printf("%s",newpacket.nickname);
	    newpacket.type=02;
		uint16_t len = strlen(newpacket.nickname)-1;
		memcpy(&newpacket.length, &len, 2);
		memcpy(&sendbuf[tx_len],&newpacket,sizeof(newpacket));
		tx_len+=sizeof(newpacket);

		if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
		    printf("Send failed\n");

	};

	return 0;
}
