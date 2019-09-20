#include "server.hh"

void Agent_listen::agent_read() {
    //cout << "into Agent_listen agent_read()" << endl; 
    this->task->task_run(this->buff,this->fd);
}

void Agent_connect_echo::agent_read() {
    //cout << "into Agent_connect agent_read()" << endl;
    int n;
    if(first_flag == 1){
        buff = new Buff;
        first_flag = 0;
    }
    this->buff->buff_init();
    if((n = buff->readin(this->fd,this->buff->from_in_flag)) == 0){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        agent_close_flag = 1;
        return;
    }
    if(n < 0 && errno == EAGAIN | EWOULDBLOCK){
        return;
    }
    this->task->task_run(this->buff,this->fd);//agent的缓冲区read结束后，开始执行对应任务
}

void Agent_connect_echo::agent_write() {
    if((this->buff == NULL) || (this->buff->to_in_flag == this->buff->to_out_flag)){
        //cout << "nothing write back to client" << endl;
        return;
    }
    //cout << "into Agent_connect agent_write()" << endl;
    int n;
    this->buff->buff_init();
    if((n = this->buff->readout(this->buff->to_out_flag,this->fd)) == 0){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
    }
}



void Agent_connect_trans::agent_read(){
    int n;
    //cout << "into Agent_connect_trans agent_read()" << endl;
    if(first_flag == 1){
        buff = new Buff;
        first_flag = 0;
    }
    this->buff->buff_init();
    if((n = buff->readin(this->fd,this->buff->from_in_flag)) == 0){
        this->close_flag = 1;
        map<int,Agent_connect_trans*>::iterator iter = register_table.find(this->task->get_from_head().srcid);
        if(iter != register_table.end()){
            register_table.erase(iter);
            //cout << "fd " << fd << "is erase from table" << endl;
        }//在登记表里就把它删掉
        agent_close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        return;
    }//处理用户删除问题
    if(n < 0 && (errno == EAGAIN | EWOULDBLOCK)){
        return;
    }
    if(n < HEADLEN && login_flag == 0)
        return;
    
    n = this->task->task_run(buff,fd);//判断登录读取头部与转发报文
    if(n == 1){
        login_flag = 1;
        register_table.insert(pair<int,Agent_connect_trans*>(this->task->get_from_head().srcid,this));
        //cout << "用户" << this->task->get_from_head().srcid << "注册表" << endl;
        map<int,Agent_connect_trans*>::iterator iter = register_table.find(this->task->get_from_head().decid);
        if(iter != register_table.end()){
            iter->second->task->task_run(iter->second->buff,iter->second->fd);
            /*if(buff->to_out_flag != buff->to_in_flag){
                struct epoll_event ev;
	            ev.data.ptr = this;
	            ev.events = EPOLLOUT;
                epoll_ctl(epfd, EPOLL_CTL_ADD, this->fd, &ev);
                cout << "用户加入可写出" << endl;
            }*/
        }
        int a = this->task->task_run(buff,fd);
    }
    if(n == -1){
        map<int,Agent_connect_trans*>::iterator iter = register_table.find(this->task->get_from_head().srcid);
        if(iter != register_table.end()){
            register_table.erase(iter);
            //cout << "fd " << fd << "is erase from table" << endl;
        }//在登记表里就把它删掉
        agent_close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        return;
    }//收到退出报文
}

void Agent_connect_trans::agent_write(){
    if((this->buff == NULL) || (this->buff->to_in_flag == this->buff->to_out_flag)){
        //cout << "nothing write back to client" << endl;
        return;
    }
    //cout << "into Agent_connect agent_write()" << endl;
    int n;
    this->buff->buff_init();
    if((n = this->buff->readout(this->buff->to_out_flag,this->fd)) == 0){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        return;
    }
    /*写出之后缓冲区有空闲，需要通知会话agent继续往里边填数据*/
    if(n > 0){
        map<int,Agent_connect_trans*>::iterator iter = register_table.find(this->task->get_from_head().decid);
        if(iter != register_table.end()){
            int a = iter->second->task->task_run(iter->second->buff,iter->second->fd);
            if(a == -1){
                register_table.erase(iter);
                //cout << "fd " << fd << "is erase from table" << endl;
                epoll_del(iter->second,iter->second->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
            }//在登记表里就把它删掉
        }//收到退出报文
    }
    if(buff->to_in_flag == buff->to_out_flag){
        //cout << "nothing write back to client" << endl;
        struct epoll_event ev;
	    ev.data.ptr = this;
	    ev.events = EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_MOD, this->fd, &ev);
        return;
    }
}//暂时先不存储to_buff_head的服务器

Buff* Agent_connect_trans::get_buff(){
    return this->buff;
}

int Agent_connect_trans::get_fd(){
    return this->fd;
}
