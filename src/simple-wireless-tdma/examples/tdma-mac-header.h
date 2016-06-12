/*
 * tdma-mac-header.h
 *
 *  Created on: Mar 30, 2016
 *      Author: zh
 */

#ifndef TDMA_HEADER_H
#define TDMA_HEADER_H

#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/packet.h"

namespace ns3 {

uint16_t DATA=0;
uint16_t ACK=1;
class TdmaHeader : public Header
{
public:
  TdmaHeader ();
  ~TdmaHeader ();
//设置源端口
  void SetSrc(uint16_t src);
//设置序列号
  void SetSeq(uint16_t seq);
//设置类型
  void SetTpye(uint16_t type);
  void SetPayloadSize(uint16_t size);
//收到包时候计算校验和
  void CalcChecksum();
 //初始化校验和，发送包的时候用
  void InitializeChecksum();
  uint16_t GetChecksum ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  //返回校验和是否正确
  bool IsChecksumOk (void) const;
private:

  uint16_t CalculateHeaderChecksum (uint16_t size) const;     //!< Source port

  //报文格式
  uint16_t m_src;
  uint16_t m_seq;
  uint16_t m_type;
  uint16_t m_payloadSize;
  uint16_t m_checksum;

  bool m_calcChecksum;        //!< Flag to calculate checksum
  bool m_goodChecksum;        //!< Flag to indicate that checksum is correct

};

} // namespace ns3

#endif
