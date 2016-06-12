/*
 * tdma-controller-dynamic.h
 *
 *  Created on: Apr 26, 2016
 *      Author: zh
 */

#ifndef TDMA_CONTROLLER_DYNAMIC_H
#define TDMA_CONTROLLER_DYNAMIC_H

#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/data-rate.h"
#include "ns3/packet.h"
#include "tdma-slot.h"
#include "ns3/timer.h"
#include "simple-wireless-channel.h"
#include "tdma-mac-header.h"
#include "tdma-route-table.h"
#include <vector>
#include <map>

namespace ns3 {


class TdmaMac;
class TdmaMacLow;
class TdmaRouteTable;

class TdmaControllerDynamic : public Object
{
public:
    enum SessionState {
      STATIC,
      DYNAMIC,
      RESERVE,
    };
  static TypeId GetTypeId (void);
  TdmaControllerDynamic ();
  ~TdmaControllerDynamic ();


  /**
   * \param slotTime the duration of a slot.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSlotTime (Time slotTime);
  void SetReserveSlotTime (Time slotTime);
  /**
   */
  void SetGaurdTime (Time gaurdTime);
  /**
   */
  void SetDataRate (DataRate bps);
  /**
   */
  void SetInterFrameTimeInterval (Time interFrameTime);
  /**
   */
  void SetTotalSlotsAllowed (uint32_t slotsAllowed);
  /**
   */
  void SetChannel (Ptr<SimpleWirelessChannel> c);
  void SetMacCentral(Ptr<TdmaMac> mac);

  Time GetSlotTime (void) const;
  Time GetReserveSlotTime(void) const;
  /**
   */
  Time GetGaurdTime (void) const;
  /**
   */
  DataRate GetDataRate (void) const;
  /**
   */
  Time GetInterFrameTimeInterval (void) const;
  /**
   */
  uint32_t GetTotalSlotsAllowed (void) const;
  /**
   * \param duration expected duration of reception
   *
   * Notify the DCF that a packet reception started
   * for the expected duration.
   */
  void NotifyRxStartNow (Time duration);
  /**
   * Notify the DCF that a packet reception was just
   * completed successfully.
   */
  void NotifyRxEndOkNow (void);
  /**
   * Notify the DCF that a packet reception was just
   * completed unsuccessfully.
   */
  void NotifyRxEndErrorNow (void);
  /**
   * \param duration expected duration of transmission
   *
   * Notify the DCF that a packet transmission was
   * just started and is expected to last for the specified
   * duration.
   */
  void NotifyTxStartNow (Time duration);
  Time CalculateTxTime (Ptr<const Packet> packet);
  void StartTdmaSessions (void);
  virtual void Start (void);
  //优化1
  bool IsTransmit();
  void RecvRouterMsg(TdmaHeader hdr);

  //优化2
  void ReceiveSlotMsg(Ptr<Packet>);
  void CheckSlotTable();
  Ptr<Packet> SetSlotPacket();
  void ReserveSlot();
  uint32_t GetFreeSlot(uint32_t index);
  void PrintSlotTable(std::ostream &os);
protected:
  virtual void DoDispose (void);

private:
  static Time GetDefaultSlotTime (void);
  static Time GetDefaultGaurdTime (void);
  static DataRate GetDefaultDataRate (void);
  void DoRestartAccessTimeoutIfNeeded (void);
  void AccessTimeout (void);
  void DoGrantAccess (void);
  bool IsBusy (void) const;
  void UpdateFrameLength (void);
  void ScheduleTdmaSession (const uint32_t slotNum);
  Ptr<SimpleWirelessChannel> GetChannel (void) const;
  Ptr<TdmaMac> GetMacCentral (void) const;
//  Time m_lastRxStart;
//  Time m_lastRxDuration;
//  bool m_lastRxReceivedOk;
//  Time m_lastRxEnd;
//  Time m_lastTxStart;
//  Time m_lastTxDuration;
//  EventId m_accessTimeout;
  DataRate m_bps;
  uint32_t m_slotTime;
  uint32_t m_gaurdTime;
  uint32_t m_tdmaFrameLength; //total frameLength in microseconds
  uint32_t m_tdmaInterFrameTime;
  uint32_t m_totalSlotsAllowed;
  bool m_activeEpoch;
  //TdmaMode m_tdmaMode;
  Ptr<SimpleWirelessChannel> m_channel;
  Ptr<TdmaMac> m_mac;
  //优化1  减少广播次数
  bool m_isTransmit;   //本节点是否转发
  double m_transmitPara;  //转发判断的参数
  Ptr<TdmaRouteTable> m_tdmaRouteTable;


  //优化2  增加动态时隙划分，均衡网络
  uint32_t m_dynamicInterval; //几个静态帧后面插播一个动态帧
  uint32_t m_staticCounnt;//静态帧计数
  std::map<uint32_t,TdmaSlot> m_slotTable;   //时隙表，动态的时候才需要
  uint8_t *m_data;
  uint32_t m_dataSize;
  uint8_t *m_dataRecv;
  uint32_t m_dataSizeRecv;
  SessionState m_sessionState;
  uint32_t m_reserveSlotTime;


};

} // namespace ns3




#endif /* TDMA_CONTROLLER_DYNAMIC_H_ */
