//
//  Echo_server.h
//  echo_server_epoll
//
//  Created by 李寒雪 on 2019/8/10.
//  Copyright © 2019 lhx.uestc.cd. All rights reserved.
//

#ifndef ECHO_SERVER_HH
#define ECHO_SERVER_HH


#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>

#include <fcntl.h>
#include <sys/epoll.h>

#define port 9877
#define MAX_EVENT 1000
#define MAX_CLINUM 1000
#define BUFF_SIZE 20480
#define LISTENQ 100
#define TIMEOUT 500

#define SERVER_TASK 1//trans
#define HEADLEN 12

using namespace std;

struct head{
    int srcid;
    int data_len;
    int decid;
};//12byte

struct min_t{
    int min_n;
    int id;
};

class Task;
class Agent;
class Buff{
    friend Task;
    friend Agent;
    public:
        Buff(){
            from_socket_buff = new char[BUFF_SIZE+1];
            to_socket_buff = new char[BUFF_SIZE+1];
            from_in_flag = from_out_flag = from_socket_buff;
            to_in_flag = to_out_flag = to_socket_buff;
        }
        ~Buff(){
            delete[] from_socket_buff;
            delete[] to_socket_buff;
        }
        //void clear_buff();//清空当前buff的所有数据
        int readin(int from,char *co_buff);//从from读到buff的对应缓冲区
        int readout(char *co_buff,int to);//从buff对应缓冲区读出到to
        int readin(char *from_co_buff,char *co_buff,int num);
        int readout(char *co_buff,char *to_co_buff);
        void buff_init();
        char *from_socket_buff;
        char *to_socket_buff;
        char *from_in_flag;//套接字接收缓冲区接下来接受指针
        char *from_out_flag;//套接字接收缓冲区接下来发送指针
        char *to_out_flag;//套接字发送缓冲区接下来发送指针
        char *to_in_flag;//套接字发送缓冲区接下来接受指针
};

class Task {
    friend Agent;
    public:
        Task(){}
        virtual int task_run(Buff *buff,int fd) = 0;
        virtual ~Task(){}
};
class Task_listen_echo: public Task {
    public:
        Task_listen_echo(){}
        int task_run(Buff *buff,int fd);
};

class Task_listen_trans: public Task {
    public:
        Task_listen_trans(){}
        int task_run(Buff *buff,int fd);
};

class Task_echo: public Task {
    public:
        Task_echo(){}
        int task_run(Buff *buff,int fd);
};

class Task_trans: public Task {
    public:
        Task_trans():head_read_flag(0),login_flag(0){
            from_buff_head.data_len = 0;
            to_buff_head.data_len = 0;
            num_neq_len = 0;
        }
        int task_run(Buff *buff,int fd);
        head get_from_head();
        void read_head(Buff *buff);
        ~Task_trans(){}
    private:
        head from_buff_head;
        head to_buff_head;
        int head_read_flag;
        int login_flag;
        int num_neq_len;
};


class Agent {
    friend Task;
    public:
        Agent(){}
        virtual void agent_read() = 0;//把数据从socker读到buff
        virtual void agent_write() = 0;//往socket写buff的数据
        //virtual void agent_run() = 0;
        virtual ~Agent(){}
};
class Agent_listen: public Agent {
    public:
        Agent_listen(int fd,Task *temp_task):fd(fd){
            buff = new Buff;
            this->task = temp_task;
        }
        void agent_read();//监听用户开始运行
        void agent_write(){}
        ~Agent_listen(){
            if(buff != NULL)
                delete buff;
            if(task != NULL)
                delete task;
        }
        Task *task;
    private:
        int fd;
        Buff *buff;
};
class Agent_connect_echo: public Agent {
    public:
        Agent_connect_echo(int fd,Task_echo *temp_task):fd(fd){
            buff = NULL;
            task = temp_task;
            close_flag = 0;
            first_flag = 1;
        }
        //void agent_run();
        void agent_read();//监听用户开始运行
        void agent_write();
        ~Agent_connect_echo(){
            if(buff != NULL)
                delete buff;
            if(task != NULL)
                delete task;
        }
        Task_echo *task;
    private:
        int fd;
        //char task[10];
        Buff *buff;
        int close_flag;
        int first_flag;
};

class Agent_connect_trans: public Agent {
    public:
        Agent_connect_trans(int fd,Task_trans *temp_task):fd(fd){
            buff = NULL;
            task = temp_task;
            close_flag = 0;
            first_flag = 1;
            //head_read_flag = 0;
            //from_buff_head.data_len = 0;
            //to_buff_head.data_len = 0;
            login_flag =0;
        }
        void agent_read();
        void agent_write();
        Buff* get_buff();
        //int read_head();
        int get_fd();
        ~Agent_connect_trans(){
            if(buff != NULL)
                delete buff;
            if(task != NULL)
                delete task;
        }
        Task_trans *task;
        private:
            int fd;
            Buff *buff;
            int close_flag;
            int first_flag;
            //head from_buff_head;
            //head to_buff_head;
            //int head_read_flag;
            int login_flag;
};

class Epoll {
    public:
        Epoll(int p);//创建全局句柄
        //void epoll_login(void *ptr, int sockfd,uint32_t events, int op);
        //int Epoll_ctl(char *op,int fd);//根据操作来选择对套接字的epoll操作
        void Epoll_bind_listen();//与套接字绑定并开始监听
        void Epoll_run();
    private:
        //int epfd;
        //struct epoll_event ev;
        int listen_fd;
        int listen_port;
        struct sockaddr_in servaddr;
        
        
};

extern void err_quit(const char *err_str);

extern int min(int a,int b);
extern struct min_t min_two(int a,int b);
extern struct min_t min_three(int a,int b,int c);
extern int epfd;
extern int agent_close_flag; 
extern map<int,Agent_connect_trans*> register_table;

extern void setnoblock(int fd);
extern void epoll_login(Agent *ptr, int sockfd,uint32_t events, int op);
extern void epoll_del(Agent *ptr, int sockfd,uint32_t events, int op);





#endif /* ECHO_SERVER_HH */
