#include "server.hh"
using namespace std;
void err_quit(const char *err_str){
    cout << err_str << endl;
    exit(0);
}

int min(int a,int b){ 
    return a>b?b:a;
}
int epfd;

int agent_close_flag = 0;

map<int,Agent_connect_trans*> register_table;

void setnoblock(int fd)
{
	int i_flag;	
	if((i_flag=fcntl(fd,F_GETFL,0))<0)
	    err_quit("fcntl fd F_SETFL wrong");
	if(fcntl(fd,F_SETFL,i_flag|O_NONBLOCK)<0)
		err_quit("fcntl fd F_SETFL wrong");
}
void epoll_login(Agent *ptr, int sockfd,uint32_t events, int op)
{
	struct epoll_event ev;
	ev.data.ptr = ptr;
	ev.events = events;
	epoll_ctl(epfd, op, sockfd, &ev);
	//cout <<"sockfd is " << sockfd << " client is connect and add in|out to epollfd" << endl;
}
void epoll_del(Agent *ptr, int sockfd,uint32_t events, int op)
{
	struct epoll_event ev;
	delete ptr;
	ev.events = events;
    epoll_ctl(epfd,op,sockfd,NULL);
	cout <<"sockfd is " << sockfd << " client is closed and del in|out from epollfd" << endl;
    close(sockfd);  
	agent_close_flag = 1;
}