#ifndef __INC_MESSAGE_QUEUE_HH__
#define __INC_MESSAGE_QUEUE_HH__

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @author Kyle May (kylemay@umich.edu)
 * 
 * @brief The interface to the Message Queue wrapper
 * that provides a templated version of mqueues to work
 * with
 * Currently, the implementation is limited to types that
 * do not contain any pointers to the actual data wanting to be transmitted
 */
template <typename T>
class MessageQueue
{
  public:
    MessageQueue(std::string identifier, std::size_t maxQueueSize,
                  bool read_enable, bool write_enable);
    ~MessageQueue();

    bool receive(T* destination);
    void send(const T* source, unsigned pri);

  private:
    const std::string id; 
    mqd_t descriptor;
    struct mq_attr queue_attributes;
    
    bool active;
    const bool rd_enable;
    const bool wr_enable;
    static const char endChar = '~';
};

/**
 * Implementation
 */
template <typename T>
MessageQueue<T>::MessageQueue(std::string identifier, std::size_t maxQueueSize,
  bool read_enable, bool write_enable)
: id(identifier), active(false), rd_enable(read_enable), wr_enable(write_enable)
{
  queue_attributes.mq_flags = 0;
  queue_attributes.mq_curmsgs = 0;
  queue_attributes.mq_maxmsg = maxQueueSize;
  queue_attributes.mq_msgsize = sizeof(T);
  
  int flag = 0;
  if (read_enable && write_enable) {
    flag |= O_RDWR;
  } else if (read_enable && !write_enable) {
    flag |= O_RDONLY;
  } else if (!read_enable && write_enable) {
    flag |= O_WRONLY;
  } else {
    throw std::runtime_error("MessageQueue Error: need rd or wr to be enabled");
  }
  flag |= O_CREAT;
  
  // Set it so we, other users in our group have read write permissions
  // Should not set the executable bit for each of these modes, hence the and not
  mode_t createMode = (S_IRWXU & ~S_IXUSR) |
                      (S_IRWXG & ~S_IXGRP) | 
                      (S_IRWXO & ~S_IXOTH);

  // Open the message queue
  descriptor = mq_open(id.c_str(), flag, createMode, &queue_attributes);

  if (descriptor == -1) {
    throw std::runtime_error("Error when opening message queue; ERRNO:" + 
                              std::to_string(errno));
  }

  active = true;
}

template <typename T>
bool
MessageQueue<T>::receive(T* destination)
{
  if (!rd_enable) {
    throw std::runtime_error("Error: not initialized with permissions to read");
  }

  if(active) {
    auto temporary_destination = new T();
    ssize_t retval = mq_receive(descriptor, (char*) temporary_destination,
                                sizeof(T), NULL);
    if (retval == -1) {
      throw std::runtime_error("Error while reading from message queue; ERRNO:" + 
                                std::to_string(errno));
    }

    if ( ((char*)temporary_destination)[0] == endChar ) {
      active = false;
    } else {
      *destination = *temporary_destination;
    }
  }

  return active;
}

template <typename T>
void MessageQueue<T>::send(const T* source, unsigned pri)
{
  if (!wr_enable) {
    throw std::runtime_error("Error: not initialized with permissions to write");
  }

  if (int ret = mq_send(descriptor, (char*) source, sizeof(T), pri) == -1) {
    throw std::runtime_error("Error while sending message; ERRNO:" +
                              std::to_string(errno));
  }
}

template <typename T>
MessageQueue<T>::~MessageQueue()
{
  if (wr_enable) {
    char end[sizeof(T)];
    end[0] = endChar;

    if (int ret = mq_send(descriptor, end, sizeof(T), 0) == -1) {
      throw std::runtime_error("Error while sending message; ERRNO:" +
                                std::to_string(errno));
    }
  }

  mq_close(descriptor);
  mq_unlink(id.c_str());
}

#endif // __INC_MESSAGE_QUEUE_HH__
