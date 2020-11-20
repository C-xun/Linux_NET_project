#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char const *argv[])
{
    printf("Content-type:text/html\n\n");

    char *data = getenv("QUERY_STRING");

    char msg_buf[128] = "";

    if (memcmp(data, "hello_config", sizeof("hello_config")) == 0)
    {
        printf("%s\t%s\t\t%s\n", "接口名", "IP地址", "MAC地址");
        FILE *config_fp = fopen("../config.conf","r");
        char config_buf[1024]="";
        fread(config_buf,sizeof(config_buf),1,config_fp);
        printf("%s\n",config_buf);

        fclose(config_fp);
    }
    else if (memcmp(data, "filter_ip", strlen("filter_ip")) == 0)
    {
        FILE *filter_fp = fopen("../filter.conf", "a");
        char filter_ip[128] = "";
        sscanf(data, "filter_ip %s", filter_ip);

        fwrite(filter_ip, strlen(filter_ip), 1, filter_fp);
        fwrite("\n", strlen("\n"), 1, filter_fp);
        fclose(filter_fp);
    }
    else if (memcmp(data, "filter_mac", strlen("filter_mac")) == 0)
    {
        FILE *filter_fp = fopen("../filter.conf", "a");
        char filter_mac[128] = "";
        sscanf(data, "filter_mac%s", filter_mac);

        fwrite(filter_mac, strlen(filter_mac), 1, filter_fp);
        fwrite("\n", strlen("\n"), 1, filter_fp);
        fclose(filter_fp);
    }
    else if (memcmp(data, "route_add", strlen("route_add")) == 0)
    {
        FILE *route_fp = fopen("../route.conf", "a");

        char route_ip[128] = "";
        sscanf(data, "route_add:%s", route_ip);

        fwrite(route_ip, strlen(route_ip), 1, route_fp);
        fwrite("\n", strlen("\n"), 1, route_fp);
        fclose(route_fp);
    }
    else if (memcmp(data, "hello_route", strlen("hello_route")) == 0)
    {
        FILE *route_fp = fopen("../route.conf", "r");
        printf("%s\t\t%s\t%s\n", "网段IP", "子网掩码", "下一跳IP");
        char route_buf[1024] = "";
        fread(route_buf, sizeof(route_buf), 1, route_fp);

        char sem_temp[16] = "";
        char next_temp[16] = "";
        sscanf(route_buf, "%[^:]%*c%s", sem_temp, next_temp);

        printf("%s\t%s\t%s\n", sem_temp, "255.255.255.0", next_temp);
        //??????????????????????????????????????????????????????????
        fclose(route_fp);
    }
    else if (memcmp(data, "hello_arp", strlen("hello_arp")) == 0)
    {
        FILE *arp_fp = fopen("../arp.conf", "r");
        printf("%s\t     %s\n", "IP地址", "MAC地址");
        char arp_buf[1024] = "";
        fread(arp_buf,sizeof(arp_buf),1,arp_fp);
        printf("%s\n",arp_buf);

        fclose(arp_fp);
    }

return 0;
}
