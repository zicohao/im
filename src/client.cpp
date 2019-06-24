#include <signal.h>
#include <pthread.h>
#include <vector>

#include "info.h"
#include "client.h"
#include "version.h"

volatile int is_quit = 0;

typedef struct {
    Client *clientp;
    Version *winp;
    std::string nick_name;
    std::string school;
}clientInfo;

clientInfo ci;

std::vector<std::string> online;

static void AddUser(std::string &f) {
    std::vector<std::string>::iterator it = online.begin();
    
    for(; it != online.end() ; it++) {
        if(*it == f) {
            return;
        }
    }
    
    online.push_back(f);
}

static void DelUser(std::string &f) {
    std::vector<std::string>::iterator it = online.begin();
    
    for (; it != online.end(); it++) {
        if(*it == f) {
            online.erase(it);
            break;
        }
    }
}

//标题窗口绘制线程
void *RunHeader(void* arg) {

    clientInfo *cip = (clientInfo*)arg;
    Version *v = cip->winp;
    v->drawHeader();
    std::string title = "Welcome To My ChatRoom";
    
    //绘制标题动态效果
    size_t i = 1;
    int y,x;
    int dir = 0;
    while(1) {
        v->drawHeader();
        getmaxyx(v->getHeader(), y, x);
        v->putStringOfVersion(v->getHeader(), y/2, i, title);
        
        if(i >= x - title.size() - 3) {
            dir = 1;
        }
        if(i <= 4) {
            dir = 0;
        }
        if(dir == 0)
            i++;
        else 
            i--;
        usleep(100000);
    }
}

// 输入窗口绘制线程
void *RunInput(void* arg) {
    clientInfo *cip = (clientInfo*)arg;
    Version *v = cip->winp;
    Client *c = cip->clientp;
    
    v->drawInput();
    std::string message = "Please Enter$ ";
    std::string str;
    Info d;
    std::string out_string;
    
    while(1) {
        v->putStringOfVersion(v->getInput(), 1, 2, message);
        //从窗口获得数据，发送到服务器
        v->getStringOfVersion(v->getInput(), str);
        //序列化
        d._userName = cip->nick_name;
        d._school = cip->school;
        d._token = "None";
        d._message = str;
        d.Serialize(out_string);
        c->sendData(out_string);//->server
        v->drawInput();
    }
}

//输出和在线列表绘制线程
void *RunOutputOnline(void* arg) {
    clientInfo *cip = (clientInfo*)arg;
    Version *v = cip->winp;
    Client *c = cip->clientp;
    v->drawOutput();
    v->drawOnline();
    int y,x;
    int i = 1;
    std::string out_string;
    std::string show_string;
    Info d;
    while(1) {
        c->recvData(out_string);

        //将收到的数据反序列化打印到 Output 上
        d.DeSerialize(out_string);
        
        show_string = d._userName;
        show_string += "-";
        show_string += d._school;
    
        if(d._token == "quit") {
            DelUser(show_string);
        } else {
            AddUser(show_string);
            show_string += ": ";
            show_string += d._message;
        
            //避免超出矩形框
            if(i > y -2) {
                i = 1;
                v->drawOutput();
            }
            getmaxyx(v->getOutput(),y,x);
            
            //y change x no change
            v->putStringOfVersion(v->getOutput(), i++, 2, show_string);
        }
        //online
        v->drawOnline();
        size_t j = 0;
        
        for(; j < online.size(); j++) {
            v->putStringOfVersion(v->getOnline(), j+1, 2, online[j]);
        }
    }
}

void SendQuit(int sig) {
    (void)sig;
    
    Info d;
    d._userName = ci.nick_name;
    d._school = ci.school;
    d._message = "None";
    d._token = "quit";
  
    std::string out_string;
    d.Serialize(out_string);
    ci.clientp->sendData(out_string);
    is_quit = 1;
}

int main(int argc, char* argv[])
{
    if(argc != 3) {
        std::cout << "Usage:" << " [server_ip][server_port]" << std::endl << argv[1]; 
        return 1;
    }
    std::cout << "Please Enter You NickName$ ";
    std::cin >> ci.nick_name;
    std::cout << "Please Enter You School$ ";
    std::cin >> ci.school;
    signal(SIGINT, SendQuit);
    Client client(argv[1], atoi(argv[2]));
    client.initClient();
    Version w;
    ci.clientp = &client;
    ci.winp = &w;
    pthread_t header,output_online,input;
    pthread_create(&header, NULL, RunHeader, (void *)&ci);
    pthread_create(&output_online, NULL, RunOutputOnline, (void *)&ci);
    pthread_create(&input, NULL, RunInput, (void*)&ci);
    while(!is_quit){
        sleep(1);
    }
    pthread_cancel(header);
    pthread_cancel(output_online);
    pthread_cancel(input);
    return 0;
}
