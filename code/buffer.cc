#include "buffer.h"
namespace MindbniM
{
    void Buffer::MakeSpace_(size_t len)
    {
        if (Write_ableBytes() + Insert_ableBytes() < len)
        {
            _buffer.resize(_wPos + len + 1);
        }
        else
        {
            int RAB = Read_ableBytes();
            std::copy(_buffer.begin() + _rPos, _buffer.begin() + _wPos, _buffer.begin());
            _rPos = 0;
            _wPos = RAB;
            assert(RAB == Read_ableBytes());
        }
    }
    void Buffer::Ensure_Writeable(size_t len)
    {
        if (_buffer.size() - _wPos < len)
        {
            MakeSpace_(len);
        }
        assert(Write_ableBytes() >= len);
    }
    void Buffer::ReWritten(size_t len)
    {
        assert(_wPos + len <= _buffer.size());
        _wPos += len;
    }
    void Buffer::Retrieve(size_t len)
    {
        assert(_rPos + len <= Read_ableBytes());
        _rPos += len;
    }
    void Buffer::Retrieve_Ptr(const char *end)
    {
        assert(end >= Peek());
        Retrieve(end - Peek());
    }
    void Buffer::Retrieve_All()
    {
        std::fill(_buffer.begin(), _buffer.end(), 0);
        _wPos = 0;
        _rPos = 0;
    }
    std::string Buffer::Retrieve_AllToStr()
    {
        std::string str(Peek(), Read_ableBytes());
        Retrieve_All();
        return str;
    }
    void Buffer::Append(const std::string &str)
    {
        Append(str.c_str(), str.size());
    }
    void Buffer::Append(const char *str, size_t len)
    {
        assert(str);
        Ensure_Writeable(len);
        std::copy(str, str + len, Begin_Write());
        ReWritten(len);
    }
    void Buffer::Append(const void *data, size_t len)
    {
        assert(data);
        Append(static_cast<const char *>(data), len);
    }
    void Buffer::Append(const Buffer &buff)
    {
        Append(buff.Peek(), buff.Read_ableBytes());
    }
    ssize_t Buffer::ReadFd(int fd, int *Errno)
    {
        char buff[1024 * 64] = {0};
        struct iovec io[2];
        int wsize = Write_ableBytes();
        io[0].iov_base = Begin_Write();
        io[0].iov_len = wsize;
        io[1].iov_base = buff;
        io[1].iov_len = sizeof(buff);
        ssize_t n = readv(fd, io, 2);
        if (n < 0)
        {
            *Errno = errno;
        }
        else if (static_cast<size_t>(n) < wsize)
        {
            _wPos += n;
        }
        else
        {
            _wPos = _buffer.size();
            Append(buff, n - wsize);
        }
        return n;
    }
    ssize_t Buffer::WriteFd(int fd, int *Errno)
    {
        ssize_t n = write(fd, Peek(), Read_ableBytes());
        if (n < 0)
        {
            *Errno = errno;
            return n;
        }
        _rPos += n;
        return n;
    }
}