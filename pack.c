while(1){
        n = read(client_fd,buf,1);
        if(n == 0){
            cancel_epoll(epoll_fd, client_fd, EPOLLIN);
            close(client_fd);
            break;
        }
        i++;
        //封包
        pack[i-1] = buf[0];
        if(i == 4){
            length = pack;
            data_size = *length;
            printf("接受到的音频字节数:%d\n",*length);
        }
        if(i == data_size+8){
            printf("接受到的总的数据包大小为:%d\n", data_size+8);
            //截取音频信息写入xx.wav
            fp = fopen("./xx.wav","wb");
            fwrite(&pack[8],1, *length,fp);
            //响应音频信息返回给客户端
            ...
            break;
        }