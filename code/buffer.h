#pragma once

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>
#include <sys/socket.h>
#include "log.h"

namespace MindbniM
{

    class Buffer
    {
    public:
        // 构造函数：初始化缓冲区，默认大小为 1024 字节
        Buffer(int Size = 1024):_buffer(Size),_rPos(0),_wPos(0)
        {}

        ~Buffer() = default;

        // 返回当前可写字节数
        size_t Write_ableBytes() const {return _buffer.size()-_wPos;}

        // 返回当前可读字节数
        size_t Read_ableBytes() const{ return _wPos-_rPos;}

        // 返回当前可插入字节数（即读位置之前的空间）
        size_t Insert_ableBytes() const{return _rPos;}

        // 返回指向可读数据开始位置的常量指针
        const char *Peek() const{ return Begin_Ptr()+_rPos;}

        // 确保缓冲区有足够的可写空间，如果不足则调整缓冲区大小
        void Ensure_Writeable(size_t len);

        // 更新缓冲区的写位置，表示 `len` 字节的数据已被写入
        void ReWritten(size_t len);

        // 更新读位置, 表示已经读完len字节
        void Retrieve(size_t len);

        // 更新读位置, 表示已经读到end
        void Retrieve_Ptr(const char *end);

        // 清空缓冲区，重置读写位置
        void Retrieve_All();

        // 将所有可读数据转换为字符串，并清空缓冲区
        std::string Retrieve_AllToStr();

        // 返回指向可写区域开始位置的常量指针
        const char *Begin_Write() const{return Begin_Ptr()+_wPos;}

        // 返回指向可写区域开始位置的指针
        char *Begin_Write(){return Begin_Ptr()+_wPos;}

        // 将字符串追加到缓冲区
        void Append(const std::string &str);

        // 将C风格字符串追加到缓冲区
        void Append(const char *str, size_t len);

        // 将原始数据追加到缓冲区
        void Append(const void *data, size_t len);

        // 将另一个缓冲区的数据追加到当前缓冲区
        void Append(const Buffer &buff);

        // 从文件描述符 `fd` 读取数据到缓冲区
        ssize_t ReadFd(int fd, int *Errno);

        // 将缓冲区的数据写入到文件描述符 `fd`
        ssize_t WriteFd(int fd, int *Errno);

        ssize_t Send(int sock,int flags, int *Errno);
        ssize_t Recv(int sock,int flags, int *Errno);

    private:
        // 返回缓冲区起始位置的非const版本指针
        char *Begin_Ptr(){return &(*_buffer.begin());}

        // 返回缓冲区起始位置的常量指针

        const char *Begin_Ptr() const{return &(*_buffer.begin());}

        // 调整缓冲区, 如果空间不足就扩容, 如果空间足够就调整数据到开头位置
        void MakeSpace_(size_t len);

        std::vector<char> _buffer; 
        std::atomic<std::size_t> _rPos;
        std::atomic<std::size_t> _wPos; 
    };
}
