#include "server.hh"
using namespace std;
void err_quit(const char *err_str){
    cout << err_str << endl;
    exit(0);
}

int min(int a,int b){ 
    return a>b?b:a;
}
struct min_t min_two(int a,int b){
	min_t ret = {0,0};
	if(a >= b){
		ret.id = 2;
		ret.min_n = b;
	}
	else
	{
		ret.id = 1;
		ret.min_n = a;
	}
	return ret;
}
 struct min_t min_three(int a,int b,int c){
	min_t ret = {0,0};
	 
	 if(a >= b){
		ret.id = 2;
		ret.min_n = b;
		if(b > c){
			ret.id = 3;
			ret.min_n = c;
		}
	}
	else
	{
		ret.id = 1;
		ret.min_n = a;
		if(a > c){
			ret.min_n = c;
			ret.id = 3;
		}
	}
	return ret;
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
	//cout <<"sockfd is " << sockfd << " client is closed and del in|out from epollfd" << endl;
    close(sockfd);  
	agent_close_flag = 1;
}
