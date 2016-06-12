/*
 * TdmaSlot.h
 *
 *  Created on: May 24, 2016
 *      Author: zh
 */

#ifndef TDMASLOT_H
#define TDMASLOT_H

using namespace ns3;
class TdmaSlot:public Object{

  public:
    enum State {
      FREE,             // A slot is said to be free if it is neither used, neither busy
      ALLOCATED,        // A slot is reserved if it is allocated/reserved by another station/node
    };

    TdmaSlot();
    virtual ~TdmaSlot();

    TdmaSlot::State GetState();
    void MarkAsFree();
    void MarkAsAllocated(Mac48Address owner);
    bool IsFree();
    bool IsAllocated();
    uint32_t GetSlotIndex();
    void SetIndex(uint32_t index);
    ns3::Mac48Address GetOwner();
  protected:
    virtual void DoDispose (void);
  private:
    State m_state;
    uint32_t m_index;
    Mac48Address m_owner;
  };




#endif /* TDMASLOT_H_ */
