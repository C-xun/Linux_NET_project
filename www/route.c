#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <pthread.h>
#include "get_interface.c"
#include <string.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

unsigned char dst_mac[]={};

int fire_wall(char *cmp_ip,char *cmp_mac)
{
    FILE *filter_fp=fopen("./filter.conf","r");
    char msg_buf[1024]="";
    fread(msg_buf,sizeof(msg_buf),1,filter_fp);
    if(strstr(msg_buf,cmp_ip) != NULL)
    {
        printf("IP:%s已被过滤\n",cmp_ip);
        return 1;
    }
    else if(strstr(msg_buf,cmp_mac) != NULL)
    {
        printf("MAC:%s已被过滤\n",cmp_mac);
        return 1;
    }
    return 0;
}

int find_arp(unsigned char *cmp_ip)
{
    
    FILE *arp_fp = fopen("./arp.conf","r");
    unsigned char arp_buf[1024] = "";
    fread(arp_buf,sizeof(arp_buf),1,arp_fp);
    

    char *dst_mac_p = strstr(arp_buf,cmp_ip);

    if (dst_mac_p != NULL)
    {
        char arp_mac[17]="";
        memcpy(arp_mac,dst_mac_p+strlen(cmp_ip)+strlen("_"),17);
        printf("arp_mac = %s\n",arp_mac);

        sscanf(arp_mac,"%02x:%02x:%02x:%02x:%02x:%02x",dst_mac,dst_mac+1,dst_mac+2,\
        dst_mac+3,dst_mac+4,dst_mac+5);  

    }
    fclose(arp_fp);
    if(dst_mac_p == NULL)
        return 1;
}

void *msg_fun(void *arg)
{

    while (1)
    {
        char msg_buf[128]="";
        fgets(msg_buf,sizeof(msg_buf),stdin);
        msg_buf[strlen(msg_buf)-1]=0;
        if(memcmp(msg_buf,"hello_config",sizeof("hello_config")) == 0)
        {
            printf("%s\t\t%s\t\t\t%s\n","接口名","IP地址","MAC地址");
            int i = 0;
            for(i=0;i<interface_num;i++)
            {
                char msg_mac[18]="";
                sprintf(msg_mac,"%02x:%02x:%02x:%02x:%02x:%02x",\
                net_interface[i].mac[0], net_interface[i].mac[1],net_interface[i].mac[2],net_interface[i].mac[3],\
                net_interface[i].mac[4],net_interface[i].mac[5]);

                char msg_ip[16]="";
                inet_ntop(AF_INET,net_interface[i].ip,msg_ip,16);
                printf("%s\t\t%s\t\t%s\n",net_interface[i].name,msg_ip,msg_mac);
            }
        }
        else if(memcmp(msg_buf,"filter_ip",strlen("filter_ip")) == 0)
        {
            FILE *filter_fp=fopen("./filter.conf","a");
            char filter_ip[16]="";
            sscanf(msg_buf,"filter_ip %s",filter_ip);


            fwrite("ip_",strlen("ip_"),1,filter_fp);
            fwrite(filter_ip,strlen(filter_ip),1,filter_fp);
            fwrite("\n",strlen("\n"),1,filter_fp);
            fclose(filter_fp);
        }
        else if(memcmp(msg_buf,"filter_mac",strlen("filter_mac")) == 0)
        {

            FILE *filter_fp=fopen("./filter.conf","a");
            char filter_mac[18]="";
            sscanf(msg_buf,"filter_mac %s",filter_mac);


            fwrite("mac_",strlen("mac_"),1,filter_fp);
            fwrite(filter_mac,strlen(filter_mac),1,filter_fp);
            fwrite("\n",strlen("\n"),1,filter_fp);
            fclose(filter_fp);
        }
        else if(memcmp(msg_buf,"route_add",strlen("route_add")) == 0)
        {
            printf("route_add\n");
            FILE *route_fp=fopen("./route.conf","a");
            char route_sem_ip[128]="";
            char route_next_ip[128]="";
            sscanf(msg_buf,"route_add %s %s",route_sem_ip,route_next_ip);


            fwrite(route_sem_ip,strlen(route_sem_ip),1,route_fp);
            fwrite(":",strlen(":"),1,route_fp);
            fwrite(route_next_ip,strlen(route_next_ip),1,route_fp);
            fwrite("\n",strlen("\n"),1,route_fp);
            fclose(route_fp);
        }
        else if(memcmp(msg_buf,"hello_route",strlen("hello_route")) == 0)
        {

            FILE *route_fp=fopen("./route.conf","r");

            fclose(route_fp);
        }
        else if(memcmp(msg_buf,"hello_arp",strlen("hello_arp")) == 0)
        {
            printf("hello_arp\n");
        }
    }
    
}

void *rcv_fun(void *arg)
{
    int sockfd = *(int *)arg;
    while (1)
    {
        char ip[16]="";

        unsigned char buf[1500]="";
        int len = recvfrom(sockfd,buf,sizeof(buf),0,NULL,NULL);
        
        //判断为ip报文时转发
        unsigned short mac_type = ntohs(*(unsigned short *)(buf+12));

        //必须arp报文  arp应答报文
        if(mac_type == 0x0806)//arp报文
        {
            if( ntohs(*(unsigned short *)(buf+20)) == 2)//arp应答
            {
                //对方的ip
                inet_ntop(AF_INET, buf+28,ip, 16);
                //回应报文的源mac就是我们需要的mac
                char mac[18]="";
                sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",\
                buf[6], buf[7],buf[8],buf[9],buf[10],buf[11]);


                char eth0_ip[16]="";
                inet_ntop(AF_INET,net_interface[1].ip,eth0_ip,16);
                char eth1_ip[16]="";
                inet_ntop(AF_INET,net_interface[2].ip,eth1_ip,16);


                if(memcmp(ip,eth0_ip,sizeof(eth0_ip)) != 0 && memcmp(ip,eth1_ip,sizeof(eth1_ip)) != 0)
                {
                    printf("%s的mac：%s\n", ip, mac);
                    memcpy(dst_mac,buf+6,6);


                    
                    char unsigned arp_str[1024] = "";
                    //打开arp.conf文件
                    FILE *arp_fp = fopen("./arp.conf","r");
                    fread(arp_str,sizeof(arp_str),1,arp_fp);
                    fclose(arp_fp);


                    if (strstr(arp_str, ip) == NULL)
                    {
                        FILE *arp_fp = fopen("./arp.conf","a");
                        fwrite(ip,strlen(ip),1,arp_fp);
                        fwrite("_",strlen("_"),1,arp_fp);
                        fwrite(mac,strlen(mac),1,arp_fp);
                        fwrite("\n",strlen("\n"),1,arp_fp);
                        fclose(arp_fp);
                    }
                }
            }
        }

        else if(mac_type == 0x0800)//IP报文
        {
            unsigned char *dst_ip = buf+14+16;
            unsigned char *src_ip = buf+14+12;
            char cmp_ip[16]="";
            inet_ntop(AF_INET,dst_ip,cmp_ip,16);

            int i = 0;
            for (i = 0; i < get_interface_num(); i++)
            {
                char ban_ip[16]="";
                char temp_ip[16]="";
                inet_ntop(AF_INET,net_interface[i].ip,temp_ip,16);
                memcpy(ban_ip,temp_ip,strlen(temp_ip)-2);
                memcpy(ban_ip+strlen(temp_ip)-2,"255",3);


                if (memcmp(dst_ip, net_interface[i].ip, 3) == 0 && memcmp(dst_ip, net_interface[i].ip, 4) != 0\
                && memcmp(src_ip,net_interface[i].ip,4) != 0 && memcmp(cmp_ip,ban_ip,sizeof(ban_ip)) != 0)
                {
                    memcpy(buf+6,net_interface[i].mac,6);
                    char test_ip[16]="";
                    inet_ntop(AF_INET, src_ip,test_ip, 16);
                    printf("-----------------%s-------------\n",test_ip);
                    inet_ntop(AF_INET, dst_ip,test_ip, 16);
                    printf("-----------------%s\n",test_ip);


                    struct ifreq ethreq;
                    strncpy(ethreq.ifr_name,net_interface[i].name,IFNAMSIZ);
                    ioctl(sockfd,SIOCGIFINDEX,&ethreq);

                    struct sockaddr_ll sll;
                    bzero(&sll,sizeof(sll));
                    sll.sll_ifindex = ethreq.ifr_ifindex;
                                      
                    if(find_arp(cmp_ip))
                    {
                        //发送arp请求
                        unsigned char msg[42] = "";
                        unsigned char arp_dst_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
                        memcpy(msg, arp_dst_mac, 6);
                        memcpy(msg + 6, net_interface[i].mac, 6);
                        msg[12] = 0x08;
                        msg[13] = 0x06;
                        msg[14] = 0x00;
                        msg[15] = 0x01;
                        msg[16] = 0x08;
                        msg[17] = 0x00;
                        msg[18] = 6;
                        msg[19] = 4;
                        msg[20] = 0;
                        msg[21] = 1;
                        memcpy(msg + 22, net_interface[i].mac, 6);
                        memcpy(msg + 28, net_interface[i].ip, 4);
                        memset(msg + 32, 0, 6);
                        memcpy(msg + 38, dst_ip, 4);

                        // 发送arp请求
                        printf("发送arp请求1\n");
                        sendto(sockfd, msg, 42, 0,  (struct sockaddr *)&sll, sizeof(sll));
                        break;
                    }

                    printf("----------发送\n");
                    memcpy(buf,dst_mac,6);
                    //防火墙过滤
                    char cmp_mac[18] = "";
                    sprintf(cmp_mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

                    if (fire_wall(cmp_ip, cmp_mac))
                    {
                        break;
                    }
                    sendto(sockfd, buf, len, 0, (struct sockaddr *)&sll, sizeof(sll));
             
                    break;
                }
            }

            if(i == interface_num)
            {

                    char test_ip[16]="";
                    inet_ntop(AF_INET, dst_ip,test_ip, 16);


                    FILE *route_fp = fopen("./route.conf", "r");
                    char route_buf[1024] = "";
                    fread(route_buf, sizeof(route_buf), 1, route_fp);


                    char sem_temp[16] = "";
                    char next_temp[16] = "";
                    char sem_ip[16] = ""; //网段ip
                    char next_ip[4]="";//下一跳ip

                    inet_ntop(AF_INET, dst_ip, sem_temp, 16);

                    memcpy(sem_ip, sem_temp, strlen(sem_temp) - 2);
                    memcpy(sem_ip + strlen(sem_temp) - 2, "0", 1);

                    char *sem_fp = strstr(route_buf,sem_ip);

                    if (sem_fp != NULL)
                    {
                        printf("匹配成功\n");
                        sscanf(sem_fp,"%[^:]%*c%s", sem_temp, next_temp);

                        inet_pton(AF_INET,next_temp,next_ip);

                        int i = 0;
                        for(i=0;i<interface_num;i++)
                        {
                            if(memcmp(next_ip, net_interface[i].ip, 3) == 0 &&memcmp(dst_ip, net_interface[i].ip, 4) != 0 \
                            && memcmp(src_ip,net_interface[i].ip,4) != 0)
                            {
                                struct ifreq ethreq;
                                strncpy(ethreq.ifr_name, net_interface[i].name, IFNAMSIZ);
                                ioctl(sockfd, SIOCGIFINDEX, &ethreq);

                                struct sockaddr_ll sll;
                                bzero(&sll, sizeof(sll));
                                sll.sll_ifindex = ethreq.ifr_ifindex;
                                //发送arp请求
                                if(find_arp(next_temp))
                                {
                                    unsigned char msg[42] = "";
                                    unsigned char arp_dst_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
                                    memcpy(msg, arp_dst_mac, 6);
                                    memcpy(msg + 6, net_interface[i].mac, 6);
                                    msg[12] = 0x08;
                                    msg[13] = 0x06;
                                    msg[14] = 0x00;
                                    msg[15] = 0x01;
                                    msg[16] = 0x08;
                                    msg[17] = 0x00;
                                    msg[18] = 6;
                                    msg[19] = 4;
                                    msg[20] = 0;
                                    msg[21] = 1;
                                    memcpy(msg + 22, net_interface[i].mac, 6);
                                    memcpy(msg + 28, net_interface[i].ip, 4);
                                    memset(msg + 32, 0, 6);
                                    memcpy(msg + 38, next_ip, 4);

                                    // 发送arp请求
                                    printf("正在发送arp请求2\n");
                                    sendto(sockfd, msg, 42, 0, (struct sockaddr *)&sll, sizeof(sll));
                                    
                                    break;
                                }
                                
                                //发送到下一跳
                                memcpy(buf+6,net_interface[i].mac,6);
                                memcpy(buf,dst_mac,6);
                                sendto(sockfd, buf, len, 0, (struct sockaddr *)&sll, sizeof(sll));
                                memset(dst_mac,0,6);
                                break;
                            }
                        }
                    }
                
                    fclose(route_fp);
            }
        }
    }
    
}
int main(int argc, char const *argv[])
{
    //创建原始套接字 
    int sockfd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(sockfd < 0)
    {
        perror("socket");
        return 0;
    }

    //调用获取接口函数
    getinterface();

    //创建接收线程
    pthread_t rcv_tid,msg_tid;
    pthread_create(&rcv_tid,NULL,rcv_fun,(void *)&sockfd);
    pthread_create(&msg_tid, NULL, msg_fun , (void*)&sockfd);

    pthread_detach(msg_tid);
    pthread_detach(rcv_tid);

    int i = 0;
    while(1);
    close(sockfd);
    return 0;
}
