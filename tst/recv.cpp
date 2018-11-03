/**
 * @brief Test that will act as the recv'ing process
 */
#include <iomanip>
#include <iostream>

#include "message-queue.hh"

struct Pkt
{
  char type;
  uint64_t addr;
};

int
main(int argc, char** argv)
{
  MessageQueue<Pkt> mq("/test_queue", 5, true, false);

  Pkt dest;
  while (mq.receive(&dest)) {
    std::cout << dest.type << ": 0x" << std::hex << dest.addr << std::endl;
  }

  return 0;
}
