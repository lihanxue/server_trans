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
    buff->readin(buff->from_out_flag,buff->to_in_flag,num);
    return 0;
}

int Task_listen_trans::task_run(Buff *buff,int fd){
    //cout << "into Task_listen_trans task_run()" << endl;
    int connfd = 0;
	if((connfd = accept(fd,NULL,NULL)) < 0)
		return 0;
    setnoblock(connfd);
    Task_trans *task_trans_temp = new Task_trans;
    Agent_connect_trans *agent_connect_pr = new Agent_connect_trans(connfd,task_trans_temp);
    epoll_login(agent_connect_pr, connfd, EPOLLIN, EPOLL_CTL_ADD);
    //agent_close_flag = 1;
    return 0;
}




int Task_trans::task_run(Buff *buff,int fd){
    if(buff->from_in_flag == buff->from_out_flag){
        return 0;
    }
    if(from_buff_head.data_len == 0){
        head_read_flag = 0;
        num_neq_len = 0;
    }
    if(head_read_flag == 0){
        if((buff->from_in_flag - buff->from_out_flag) < HEADLEN)
            return 0;//必须每个报文有完整的头部才开始读
        read_head(buff);
        if(from_buff_head.data_len == -1){
            return -1;
        }//收到结束报文头部信息
        head_read_flag = 1;
        if(from_buff_head.data_len == 0){
            buff->from_out_flag = buff->from_out_flag + HEADLEN;
            //cout << "login data neednot trans" << endl;
        }//登录报文不用转发
    }//登录或读取头报文过程
    if(login_flag == 0){
        login_flag = 1;
        return 1;//第一次登陆
    }
    int num;
    map<int,Agent_connect_trans*>::iterator iter = register_table.find(from_buff_head.decid);
    if(iter == register_table.end()){
        /*if(num_neq_len == 0){
            min_t num_t = min_two(buff->from_in_flag - buff->from_out_flag,from_buff_head.data_len + HEADLEN);
            num = num_t.min_n;
            if(num_t.id == 1){
                cout << "没找到用户" << from_buff_head.decid << "并丢弃部分报文" << num_t.min_n << endl;
                from_buff_head.data_len = from_buff_head.data_len - (num_t.min_n - HEADLEN);
                num_neq_len = 1;
            }//发送用户没有接受完全部报文
            else{
                cout << "没找到用户" << from_buff_head.decid << "并丢弃部分报文" << num_t.min_n << endl;
                from_buff_head.data_len = from_buff_head.data_len - (num_t.min_n - HEADLEN);
            }
        }
        else{
            min_t num_t = min_two(buff->from_in_flag - buff->from_out_flag,from_buff_head.data_len);
            num = num_t.min_n;
            if(num_t.id == 1){
                cout << "没找到用户" << from_buff_head.decid << "并丢弃部分报文" << num_t.min_n <<endl;
                from_buff_head.data_len = from_buff_head.data_len - num_t.min_n;
            }//发送用户没有接受完全部报文
            else{
                cout << "没找到用户" << from_buff_head.decid << "并丢弃部分报文" << num_t.min_n <<endl;
                from_buff_head.data_len = 0;
            }//发送用户接受完全部的报文
        }*/
        min_t num_t = min_two(buff->from_in_flag - buff->from_out_flag,from_buff_head.data_len + HEADLEN);
        //cout << "没找到用户" << from_buff_head.decid << "但并没有丢弃部分报文" << num_t.min_n <<endl;
        return 0;
    }
    if(iter != register_table.end()){
        Buff* temp_buff = iter->second->get_buff();
        if(num_neq_len == 0){
            min_t num_t = min_three(buff->from_in_flag - buff->from_out_flag,from_buff_head.data_len + HEADLEN
            ,&(temp_buff->to_socket_buff[BUFF_SIZE]) - temp_buff->to_in_flag);
            num = num_t.min_n;
            if(num_t.id == 1){
                memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                from_buff_head.data_len = from_buff_head.data_len - (num_t.min_n - HEADLEN);
                num_neq_len = 1;
            }//发送用户没有接受完全部报文
            else if(num_t.id == 2){
                memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                from_buff_head.data_len = 0;
            }//发送用户接受完全部的报文
            else{
                if(num_t.min_n < HEADLEN)
                    return 0;
                else{
                    memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                    temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                    //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                    from_buff_head.data_len = from_buff_head.data_len - (num_t.min_n - HEADLEN);
                    num_neq_len = 1;
                } 
            }
        }//找到中继用户
        else{
            min_t num_t = min_three(buff->from_in_flag - buff->from_out_flag,from_buff_head.data_len
            ,&(temp_buff->to_socket_buff[BUFF_SIZE]) - temp_buff->to_in_flag);
            num = num_t.min_n;
            if(num_t.id == 1){
                memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                from_buff_head.data_len = from_buff_head.data_len - num_t.min_n;
            }//发送用户没有接受完全部报文
            else if(num_t.id == 2){
                memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                from_buff_head.data_len = 0;
            }//发送用户接受完全部的报文
            else{ 
                memcpy(temp_buff->to_in_flag,buff->from_out_flag,num_t.min_n);
                temp_buff->to_in_flag = temp_buff->to_in_flag + num_t.min_n;
                //cout << "找到用户" << from_buff_head.decid << "并转发" << num_t.min_n <<endl;
                from_buff_head.data_len = from_buff_head.data_len - (num_t.min_n - HEADLEN);
                num_neq_len = 1;
            }
        }
        if(temp_buff->to_out_flag != temp_buff->to_in_flag){
            struct epoll_event ev;
	        ev.data.ptr = iter->second;
	        ev.events = EPOLLOUT | EPOLLIN;
            epoll_ctl(epfd, EPOLL_CTL_MOD, iter->second->get_fd(), &ev);
            //cout << "用户" << iter->second->task->get_from_head().srcid << "加入可以写出" << endl;
        }
    }
    
    buff->from_out_flag = buff->from_out_flag + num;

    /*找到转发对象就把报文放到它buff发送区,没找到就丢弃该报文*/
    return 0;
}

head Task_trans::get_from_head(){
    return from_buff_head;
}


void Task_trans::read_head(Buff *buff){
    //cout << "into task_trans read_head() "; 
    char *temp = buff->from_out_flag;
    
    this->from_buff_head.srcid = *(int*)temp;
    temp = temp + 4;

    
    this->from_buff_head.data_len = *(int*)temp;
    temp = temp + 4;

    
    this->from_buff_head.decid = *(int*)temp;
    //cout << "read head " << 12 << " finish" << endl;
}