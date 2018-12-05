/**
 * @brief Test that will act as the recv'ing process
 */
#include <iomanip>
#include <iostream>

#include "message-queue.hh"
#include "mem_ref.h"


int
main(int argc, char** argv)
{
  MessageQueue<MemPkt> mq("/test_queue", 5, true, false);

  MemPkt dest;
  while (mq.receive(&dest)) {
    std::cout << "type: " << dest.type << " pc: " << dest.pc << " ea: ";
    std::cout << std::hex << dest.addr << std::endl;
    std::cout << std::dec;
  }

  return 0;
}
