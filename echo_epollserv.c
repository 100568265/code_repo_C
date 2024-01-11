#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50
void error_handling(char *buf);

int main()
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if(argc != 2){
        printf("Usage : %s <port>\n",argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(9190);

    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr)) == -1)
        error_handling("bind() error!");
    if(listen(serv_sock,5) == -1)
        error_handling("listen() error!");

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1)
        {
            puts("epoll_wait() error!");
            break;
        }

        for(i=0; i<event_cnt; i++)
        {
            if(ep_events[i].data.fd == serv_sock)   //如果有事件发生在这个文件描述符上
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&adr_sz);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd,EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client: %d \n",clnt_sock);
            }
            else
            {
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if(str_len == 0)    //close request
                {
                    epoll_ctl(epfd,EPOLL_CTL_DEL,ep_events[i].data.fd,NULL);
                    close(ep_events[i].data.fd);
                    printf("closed client: %d \n",ep_eventsp[i].data.fd);
                }
                else
                    write(ep_events[i].data.fd, buf, str_len);  //echo!
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}

/*
这段代码是在使用 `epoll` 进行 I/O 多路复用的一个例子。`epoll` 是 Linux 系统中用于替代传统的 `select` 和 `poll` 的高效文件描述符监听机制。我将逐行解释这段代码的意义：

```c
event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
```

- `epoll_wait` 函数等待 `epfd` 指向的 epoll 实例监控的文件描述符上的事件发生。
- `ep_events` 是用来从 `epoll_wait` 接收事件通知的数组。
- `EPOLL_SIZE` 是 `ep_events` 数组的大小，表示最多可以处理的事件数量。
- 最后的 `-1` 表示 `epoll_wait` 的超时时间。在这里，`-1` 意味着 `epoll_wait` 将会无限期阻塞，直到至少有一个事件发生。
- `event_cnt` 是 `epoll_wait` 返回的值，表示有多少个事件发生了。如果 `epoll_wait` 出错，它会返回 `-1`。

接下来的代码块检查 `epoll_wait` 是否出错：

```c
if(event_cnt == -1)
{
    puts("epoll_wait() error!");
    break;
}
```

- 如果 `event_cnt` 等于 `-1`，则打印错误信息 `"epoll_wait() error!"` 并退出当前循环。

下面的循环遍历所有发生的事件：

```c
for(i=0; i<event_cnt; i++)
{
    if(ep_events[i].data.fd == serv_sock)
    {
        
    }
}
```

- 循环通过 `event_cnt` 来控制，确保遍历所有发生的事件。
- 在循环体内部，`ep_events[i].data.fd == serv_sock` 是一个条件判断语句，用来检查当前事件是否发生在 `serv_sock` 描述的文件描述符上。

`serv_sock` 通常是一个服务端 socket 的文件描述符，用来监听客户端的连接请求。如果条件成立，说明有一个新的连接请求到达了服务端。在这种情况下，通常会调用 `accept` 函数来接受这个连接请求，然后可能将返回的新 socket 文件描述符加入到 `epoll` 实例中，以便监控这个新连接上的 I/O 事件。

空的条件块 `{}` 表示这段代码是一个代码片段的一部分，你提供的部分没有包含处理新连接的代码。在实际应用中，这个条件块内部应该包含接受新连接并进行相应处理的代码。
*/
