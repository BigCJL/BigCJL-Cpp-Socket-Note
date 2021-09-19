#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
        //1.创建监听的套接字
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd == -1){
                perror("socket");
                return -1;
        }
        //2. 绑定本地的IP Port
        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(9999);
        saddr.sin_addr.s_addr = INADDR_ANY;   //0=0.0.0.0 读取本地实际网卡地址
        int ret = bind(fd,(struct sockaddr*)&saddr, sizeof(saddr));
        if(ret == -1){
                perror("bind");
                return -1;
        }
        //3. 设置监听
        ret = listen(fd, 128);
        if(ret == -1){
                perror("listen");
                return -1;
        }
        //4. 阻塞并等待客户端连接
        struct sockaddr_in caddr;
        int addrlen = sizeof(caddr);
        int cfd = accept(fd, (struct sockaddr*)&caddr, (socklen_t*)&addrlen);
        if(cfd == -1){
                perror("accept");
                return -1;
	}
	//5. 连接建立成功，打印客户端的ip和端口信K息
	char ip[32];
        printf("客户端的IP：%s, 端口：%d\n",inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip)),
			ntohs(caddr.sin_port));//大端转换为小端的函数
	//6. 通信
	while(1){
		//接收数据
	char buff[1024];
	int len = recv(cfd, buff, sizeof(buff), 0);
	if(len>0){
		printf("client say: %s\n",buff);
		send(cfd, buff, len, 0);
        sleep(1);
	}
	else if(len == 0){
		printf("客户端已经断开...\n");
		break;
	}
	else{
		perror("recv");
		break;
	}
	}
	// 关闭文件描述符
	close(fd); // 这个是用来监听的
	close(cfd);// 这个是用来通信的
       	return 0;
}       
