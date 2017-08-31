/*
 * upgrade.h
 *
 *  Created on: 2017年4月24日
 *      Author: root
 */

#ifndef UPGRADE_H_
#define UPGRADE_H_
#include "types.h"

struct resp_header//保持相应头信息
{
    int status_code;//HTTP/1.1 '200' OK
    char content_type[128];//Content-Type: application/gzip
    long content_length;//Content-Length: 11683079
    //char file_name[256];
};

void parse_url(const char *url, char *domain, int *port, char *file_name);
struct resp_header get_resp_header(const char *response);
//void get_ip_addr(char *domain, char *ip_addr);
void progressBar(long cur_size, long total_size);
void * download(void * socket_d);
int UpgradeApp();


#endif /* UPGRADE_H_ */
