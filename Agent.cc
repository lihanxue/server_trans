#include "server.hh"

void Agent_listen::agent_read() {
    cout << "into Agent_listen agent_read()" << endl; 
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
    /*if(from_buff_head.data_len == 0){
        head_read_flag = 0;
    }
    if(head_read_flag = 0){
        if((n = read_head()) <= 0){
            return;
        }
        else
            head_read_flag = 1;
            if(login_flag == 0){
                register_table.insert(pair<int,Agent_connect_trans*>(fd,this));
                login_flag = 1;
            }
        return;
    }//登录和读取头报文过程*/
    cout << "into Agent_connect_trans agent_read()" << endl;
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
            cout << "fd " << fd << "is erase from table" << endl;
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
        cout << "用户" << this->task->get_from_head().srcid << "注册表" << endl;
    }
}

void Agent_connect_trans::agent_write(){
    if((this->buff == NULL) || (this->buff->to_in_flag == this->buff->to_out_flag)){
        //cout << "nothing write back to client" << endl;
        return;
    }
    cout << "into Agent_connect agent_write()" << endl;
    int n;
    this->buff->buff_init();
    if((n = this->buff->readout(this->buff->to_out_flag,this->fd)) == 0){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
    }

}//暂时先不存储to_buff_head的服务器

Buff* Agent_connect_trans::get_buff(){
    return this->buff;
}


/*int Agent_connect_trans::read_head(){
    int n;
    if((n = read(this->fd,&(this->from_buff_head),HEADLEN)) == 0){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        agent_close_flag = 1;
        return n;
    }
    else if(n < 0 && (errno != EAGAIN | EWOULDBLOCK)){
        this->close_flag = 1;
        epoll_del(this,this->fd,EPOLLIN | EPOLLOUT,EPOLL_CTL_DEL);
        agent_close_flag = 1;
        return 0;
    }
    else
        return n;
}*/