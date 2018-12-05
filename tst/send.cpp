/**
 * @brief Test that will act as the sending process 
 */

#include "message-queue.hh"
#include "mem_ref.h"

int
main(int argc, char** argv)
{
  MessageQueue<MemPkt> mq("/test_queue", 5, false, true);

  for (int i = 0; i < 20; ++i) {
    MemPkt tmp;
    tmp.type = 'r';
    tmp.pc = i;
    tmp.addr = i*0x100 + 0x2000;

    mq.send(&tmp, 0);
  }

  return 0;
}
