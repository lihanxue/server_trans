#include "server.hh"

int Task_listen_echo::task_run(Buff *buff,int fd) {
    //cout << "into Task_listen task_run()" << endl;
    int connfd = 0;
	if((connfd = accept(fd,NULL,NULL)) < 0)
		return 0;
    setnoblock(connfd);
    Task_echo *task_echo_temp = new Task_echo;
    Agent_connect_echo *agent_connect_pr = new Agent_connect_echo(connfd,task_echo_temp);
    epoll_login(agent_connect_pr, connfd, EPOLLIN | EPOLLOUT, EPOLL_CTL_ADD);
    return 0;
}

int Task_echo::task_run(Buff *buff,int fd) {
    int num = min((buff->from_in_flag - buff->from_out_flag),(&(buff->to_socket_buff[BUFF_SIZE]) 
    - buff->to_in_flag));
    
    //cout << buff->from_in_flag - buff->from_out_flag << "and" << (&(buff->to_socket_buff[BUFF_SIZE]) 
    //- buff->to_in_flag) << " min is:" << num << endl;
    
    //cout << "into Task_echo task_run()" << endl;
    buff->readin(buff->from_out_flag,buff->to_in_flag,num);
    return 0;
}

int Task_listen_trans::task_run(Buff *buff,int fd){
    cout << "into Task_listen_trans task_run()" << endl;
    int connfd = 0;
	if((connfd = accept(fd,NULL,NULL)) < 0)
		return 0;
    setnoblock(connfd);
    Task_trans *task_trans_temp = new Task_trans;
    Agent_connect_trans *agent_connect_pr = new Agent_connect_trans(connfd,task_trans_temp);
    epoll_login(agent_connect_pr, connfd, EPOLLIN | EPOLLOUT, EPOLL_CTL_ADD);
    //agent_close_flag = 1;
    return 0;
}




int Task_trans::task_run(Buff *buff,int fd){
    if(from_buff_head.data_len == 0){
        head_read_flag = 0;
        num_neq_len = 0;
    }
    if(head_read_flag == 0){
        read_head(buff);
        head_read_flag = 1;
        if(from_buff_head.data_len == 0){
            buff->from_out_flag = buff->from_out_flag + HEADLEN;
            cout << "login data neednot trans" << endl;
        }//登录报文不用转发
    }//登录或读取头报文过程
    if(login_flag == 0){
        login_flag = 1;
        return 1;//第一次登陆
    }
    int num;
    map<int,Agent_connect_trans*>::iterator iter = register_table.find(from_buff_head.decid);
    if(buff->from_in_flag-buff->from_out_flag > (from_buff_head.data_len+HEADLEN)){
        if(num_neq_len == 0)
            num = from_buff_head.data_len+HEADLEN;
        else
            num = from_buff_head.data_len;
    }
    else
    {
        num = buff->from_in_flag-buff->from_out_flag;
        num_neq_len = 1;
    }
    Buff* temp_buff = iter->second->get_buff();
    if(iter != register_table.end()){
        memcpy(temp_buff->to_in_flag,buff->from_out_flag,num);
        temp_buff->to_in_flag = temp_buff->to_in_flag + num;
        cout << "找到用户" << from_buff_head.decid << "并转发" << num <<endl;
    }
    buff->from_out_flag = buff->from_out_flag + num;
    from_buff_head.data_len = from_buff_head.data_len - num;
    /*找到转发对象就把报文放到它buff发送区,没找到就丢弃该报文*/
    return 0;
}

head Task_trans::get_from_head(){
    return from_buff_head;
}


void Task_trans::read_head(Buff *buff){
    cout << "into task_trans read_head() "; 
    char *temp;
    temp = new char[4];
    strncpy(temp,buff->from_out_flag,4);
    this->from_buff_head.srcid = atoi(temp);

    strncpy(temp,buff->from_out_flag+4,4);
    this->from_buff_head.data_len = atoi(temp);

    strncpy(temp,buff->from_out_flag+8,4);
    this->from_buff_head.decid = atoi(temp);
    cout << "read head " << 12 << " finish" << endl;
    //buff->from_out_flag = buff->from_out_flag + HEADLEN;
    delete temp;
}