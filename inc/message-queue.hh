#ifndef __INC_MESSAGE_QUEUE_HH__
#define __INC_MESSAGE_QUEUE_HH__

#include <errno.h>
#include <fcntl.h>
#include <iostream>
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

/**
 * @brief on Pkt
 * This type must overload three functions
 * static unsigned sizeOfBuffer(void)
 *  - this should return the size in bytes that the struct needs to send there information
 * void unpack(char*)
 *  - this should take in a buffer that was used for mqueue communication and move the data
 *    into the struct's members
 * void pack(char*)
 *  - this will take the members of the class and place them into the provided buffer
 *    for mqueue communication
 *
 * The use of the pack and unpack will allow for the mqueue code to stay relatively the same
 * throughout
 * An example for a simple type is provided
 */
class IntPkt
{
 public:
  static unsigned sizeOfBuffer(void) { return 4; }
  void unpack(char* in) { m = *((uint32_t*) in); }
  void pack(char* out) const { *((uint32_t*) out) = m; }

  uint32_t m;

  IntPkt() = default;
  ~IntPkt() = default;
};

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
    static const char contChar = 1;
    static const char endChar = 0;
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
  queue_attributes.mq_msgsize = T::sizeOfBuffer()+1;
  
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
    char* tmp = new char[T::sizeOfBuffer()+1];
    ssize_t retval = mq_receive(descriptor, tmp,
                                T::sizeOfBuffer()+1, NULL);
    if (retval == -1) {
      throw std::runtime_error("Error while reading from message queue; ERRNO:" + 
                                std::to_string(errno));
    }

    if ( tmp[0] == endChar ) {
      active = false;
    } else {
      destination->unpack(tmp+1);
    }

    delete[] tmp;
  }

  return active;
}

template <typename T>
void MessageQueue<T>::send(const T* source, unsigned pri)
{
  if (!wr_enable) {
    throw std::runtime_error("Error: not initialized with permissions to write");
  }

  /** Set up the message for the queue */
  char* msg = new char[T::sizeOfBuffer()+1];
  msg[0] = contChar;
  source->pack(msg+1);
  
  if (int ret = mq_send(descriptor, msg, T::sizeOfBuffer()+1, pri) == -1) {
    throw std::runtime_error("Error while sending message; ERRNO:" +
                              std::to_string(errno));
  }
  delete[] msg;
}

template <typename T>
MessageQueue<T>::~MessageQueue()
{
  if (wr_enable) {
    char* end_msg = new char[T::sizeOfBuffer()+1];

    end_msg[0] = endChar;

    mq_send(descriptor, end_msg, T::sizeOfBuffer()+1, 0);
  }

  mq_close(descriptor);
  mq_unlink(id.c_str());
}

#endif // __INC_MESSAGE_QUEUE_HH__
