/**
 * A packet that represents a memory reference
 */

#ifndef _MEM_REF_H_
#define _MEM_REF_H_
#include <cstdint>

class MemPkt
{
 public:
  char type;
  uint64_t addr;
  uint64_t pc;

  void pack(char* out) const
  {
    *((uint64_t*) out) = addr; 
    out[8] = type;
    *((uint64_t*) (out+9)) = pc;
  }

  void unpack(char* in)
  {
    addr = *((uint64_t*) in);
    type = in[8];
    pc = *((uint64_t*) (in+9));
  }

  static unsigned sizeOfBuffer(void) { return 17; }

  MemPkt() = default;
  ~MemPkt() = default;
};

#endif // _MEM_REF_H_
