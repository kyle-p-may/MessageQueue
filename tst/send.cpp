/**
 * @brief Test that will act as the sending process 
 */

#include "message-queue.hh"

struct Pkt
{
  char type;
  uint64_t addr;
};

int
main(int argc, char** argv)
{
  MessageQueue<Pkt> mq("/test_queue", 5, false, true);

  for (int i = 0; i < 20; ++i) {
    Pkt tmp;
    if (i%2) { tmp.type = 'r'; }
    else { tmp.type = 'w'; }
    tmp.addr = 0x1000 + i*0x10;
    mq.send(&tmp, 0);
  }

  return 0;
}
