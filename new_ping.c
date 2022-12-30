#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// command: make clean && make all && ./parta

#define IP4_HDRLEN 20
#define ICMP_HDRLEN 8

unsigned short calculate_checksum(unsigned short *paddress, int len);
int createPacket(char *packet, int seq); // create icmp packet return packet length

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Usage: ./parta <ip-address>\n");
        return 0;
    }

    char ip[INET_ADDRSTRLEN];
    strcpy(ip, argv[1]);
    struct in_addr addr;
    if (inet_pton(AF_INET, ip, &addr) != 1)
    {
        printf("Invalid ip-address\n");
        return 0;
    }

    int count = 0;
    char packet[IP_MAXPACKET];
    int packetlen = createPacket(packet, count);
    printf("PING %s: %d data bytes\n", ip, packetlen - ICMP_HDRLEN);

    struct sockaddr_in dest_in;
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    dest_in.sin_addr.s_addr = inet_addr(ip);

    // Create raw socket for IP-RAW-ICMP
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;

    while (1)
    {
        int pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);
        }
        else
        {
            int packetlen = createPacket(packet, count); // create icmp packet

            struct timeval start, end;
            gettimeofday(&start, 0);

            int bytes_sent = sendto(sock, packet, packetlen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
            if (bytes_sent == -1)
            {
                fprintf(stderr, "sendto() failed with error: %d", errno);
                return -1;
            }

            bzero(packet, IP_MAXPACKET);
            socklen_t len = sizeof(dest_in);
            ssize_t bytes_received = -1;

            while ((bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
            {
                if (bytes_received > 0)
                {
                    // Check the IP header
                    struct iphdr *iphdr = (struct iphdr *)packet;
                    struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
                    break;
                }
            }
            gettimeofday(&end, 0);
            kill(pid, SIGKILL);

            char reply[IP_MAXPACKET];
            memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, packetlen - ICMP_HDRLEN); // get reply data from packet

            float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
            unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
            float time = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
            printf("    %d bytes from %s: seq: %d time: %0.3fms\n", bytes_received, ip, count, time);

            count++;
            bzero(packet, IP_MAXPACKET);
            sleep(1); // wait 1 second to send next packet. looks better in terminal
        }
    }
    close(sock);
    return 0;
}

int createPacket(char *packet, int seq)
{
    struct icmp icmphdr; // ICMP-header

    icmphdr.icmp_type = ICMP_ECHO; // Message Type (8 bits): echo request
    icmphdr.icmp_code = 0;         // Message Code (8 bits): echo request
    icmphdr.icmp_cksum = 0;        // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_id = 24;          // Identifier (16 bits): some number to trace the response.
    icmphdr.icmp_seq = seq;        // Sequence Number (16 bits)

    memcpy((packet), &icmphdr, ICMP_HDRLEN); // add ICMP header to packet

    char data[IP_MAXPACKET] = "ping\n";
    int datalen = strlen(data) + 1;
    memcpy(packet + ICMP_HDRLEN, data, datalen); // add ICMP data to packet

    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen); // calculate checksum
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    return ICMP_HDRLEN + datalen;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}