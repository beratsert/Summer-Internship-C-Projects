#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

/* destination mac has been chosen as broadcast mac.*/
#define DEST_MAC0	0xff
#define DEST_MAC1	0xff
#define DEST_MAC2	0xff
#define DEST_MAC3	0xff
#define DEST_MAC4	0xff
#define DEST_MAC5	0xff
#define ETHER_TYPE	0x1234
#define DEFAULT_IF	"enp0s17" /* assigned name to ethernet port.*/
#define BUF_SIZ		1400
#define MAX_NAME_SIZE 10
#define MAX_FILE_NAME_SIZE 32

/* broadcast packets has been defined.*/
struct file_bcast {
    struct ether_header eth;
    uint8_t type;
    char name[MAX_NAME_SIZE];
    char surname[MAX_NAME_SIZE];
    char file_name[MAX_FILE_NAME_SIZE];
    uint32_t file_size;
    uint16_t file_fragment_count;
    uint16_t fragment_index;
    uint32_t fragment_size;
    char fragment_data[0];
}__attribute__((packed));

int main(int argc, char *argv[])
{
	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
  char file[5402][998];
	char ifName[IFNAMSIZ];

	/* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;
	struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));
	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");
		return -1;
	}

	/* Set interface to promiscuous mode */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	int cnt=0;

	repeat:	printf("listener: Waiting to recvfrom...\n");

        numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	    rintf("listener: got packet %lu bytes\n", numbytes);

        struct file_bcast *file_packet=&buf;

	    printf("%d",file_packet->file_fragment_count);
	    printf("  %d",file_packet->fragment_index);
	    memcpy(file[file_packet->fragment_index],file_packet->fragment_data, file_packet->fragment_size);
	    cnt++;
	    printf("\n%d\n",cnt);

		if(cnt==file_packet->file_fragment_count){

            FILE * chosen_file;
		    chosen_file = fopen("music.mp3","w"); /* you need to give path to the file desired to transfer*/

            if(chosen_file== NULL)
		    {
		        printf("Unable to create file.\n");
		        exit(EXIT_FAILURE);
		    }

            for(i=1;i<=5402;i++) {
		    	fwrite(file[i], sizeof(char),sizeof(file[i]),chosen_file);
		    }

            fclose(chosen_file);
		}
	done:	goto repeat;

	close(sockfd);
	return ret;
}
