//
//  main.cpp
//  echo_server_epoll
//
//  Created by 李寒雪 on 2019/8/10.
//  Copyright © 2019 lhx.uestc.cd. All rights reserved.
//

#include "server.hh"
using namespace std;
//extern int epfd;
int main(int argc, const char * argv[2]) {
    // insert code here...
    //int epfd;
    if(argc != 2){
        err_quit("<listen_port>\n");
    }
    if((epfd = epoll_create(MAX_CLINUM)) < 0)
        err_quit("epoll_create fail\n");
    Epoll epoll1(atoi(argv[1]));
    epoll1.Epoll_bind_listen();
    epoll1.Epoll_run();
    close(epfd);
    std::cout << "end\n";
    return 0;
}
