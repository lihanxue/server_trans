#include "server.hh"

Epoll::Epoll(int p){
    this->listen_port = p;
    if((this->listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0)
        err_quit("epoll socket fail\n");
    setnoblock(listen_fd);
    cout<<listen_fd << " " << listen_port;
    bzero(&(this->servaddr),sizeof(this->servaddr));
	this->servaddr.sin_family=AF_INET;
	this->servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	this->servaddr.sin_port=htons(this->listen_port);
    Agent_listen *agent_listen_pr;
	if(SERVER_TASK == 1){
		Task_listen_trans *temp_trans_task = new Task_listen_trans;
		agent_listen_pr = new Agent_listen(this->listen_fd,temp_trans_task);
	}//中继服务
    else if(SERVER_TASK == 0){
		Task_listen_echo *temp_echo_task = new Task_listen_echo;
		agent_listen_pr = new Agent_listen(this->listen_fd,temp_echo_task);
	}
    struct epoll_event ev;
    ev.data.ptr = agent_listen_pr;
	//(Agent_listen *) ev.data.ptr;
    ev.events = EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&ev);	
}//创建全局句柄与监听套接字并加入epoll


//Epoll::Epoll_ctl(struct epoll_event &ev,)

void Epoll::Epoll_bind_listen() {
    if(bind(listen_fd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
        err_quit("bind fail\n");
	if(listen(listen_fd,LISTENQ)<0)
        err_quit("listen fail\n");
}
void Epoll::Epoll_run() {
	cout << "epoll_run" << endl;
    int i, i_ready;
	for(;;)
	{
		struct epoll_event events[MAX_EVENT];
		i_ready=epoll_wait(epfd,events,MAX_EVENT,TIMEOUT); 
		for(i=0;i<i_ready;i++)
		{
			if(events[i].events & EPOLLIN)
			{
				agent_close_flag = 0;
				((Agent*)events[i].data.ptr)->agent_read();
				if(agent_close_flag == 1){
					agent_close_flag = 0;
					continue;
				}
			}
			if(events[i].events & EPOLLOUT)
			{
				((Agent*)events[i].data.ptr)->agent_write();
			}
		}
	}	
}
