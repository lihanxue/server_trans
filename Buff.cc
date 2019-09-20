#include "server.hh"

int Buff::readin(int from,char *co_buff) {
	//cout << "into Buff readin(int,char *)" << endl;
    int n;
    if ((n = read(from,co_buff, &(this->from_socket_buff[BUFF_SIZE]) - co_buff)) < 0) {
		if (errno != EWOULDBLOCK | EAGAIN){
			perror("read error\n");
			return 0;
		}
	}//有一个用户发生了接收缓冲区读read socket错误就直接退出进程
	else if(n == 0){
		//cout << "reanin() back n = " << n << endl;
		return n;
	}//该用户关闭发送连接，那么直接,返回该用户的epoll主循环，把该用户对应的socket，用户类删除
	else
	{
		//cout << "readin() back n = " << n << endl;
        this->from_in_flag = this->from_in_flag + n;
	}
	return n;
}
int Buff::readout(char *co_buff,int to) {
	//cout << "into Buff readout(char*,int)" << endl;
	int n;
	if((n = write(to, co_buff,this->to_in_flag - co_buff)) < 0){
		if (errno != EAGAIN | EWOULDBLOCK){
			perror("write error\n");
			return 0;
		}
        /*else if(errno == EPIPE)
            return -1;*/
	}//有一个用户发生了write socket错误就直接退出进程
	else
	{
		//cout << "write " << n << "bytes to " << to << endl;
		this->to_out_flag = this->to_out_flag + n;
		//co_buff = co_buff + n;
	}
	return n;
}

int Buff::readin(char *from_co_buff,char *co_buff,int num) {
	memcpy(co_buff,from_co_buff,num);
	//cout << "into readin(char*,char*)() and read " << num <<
	//" char from from_out_flag to to_in_flag" <<endl; 
	this->from_out_flag = this->from_out_flag + num;
	this->to_in_flag = this->to_in_flag + num;
	return num;
}


void Buff::buff_init(){
	if(this->from_in_flag == this->from_out_flag){
    	 this->from_in_flag = this->from_out_flag = this->from_socket_buff;
    }
	if(this->to_in_flag == this->to_out_flag){
		this->to_in_flag = this->to_out_flag = this->to_socket_buff;
	}
}
