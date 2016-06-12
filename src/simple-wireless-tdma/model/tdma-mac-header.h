/*
 * tdma-mac-header.h
 *
 *  Created on: Mar 30, 2016
 *      Author: zh
 */

#ifndef TDMA_HEADER_H_
#define TDMA_HEADER_H_

#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"

namespace ns3 {

enum TdmaType{
	 ROUTE,
	 DATA,
};

class TdmaHeader : public Header
{
public:
  TdmaHeader ();
  ~TdmaHeader ();
//设置源端口
  void SetSrc(Mac48Address src);
  Mac48Address GetSrc(void)const;
  void SetDest(Mac48Address dest);
  Mac48Address GetDest(void)const;
  void SetTransmit(Mac48Address tran);
  Mac48Address GetTransmit();
//设置序列号
  void SetSeq(uint8_t seq);
  uint8_t GetSeq(void)const;
//设置类型
  void SetTypeRoute(void);
  void SetTypeData(void);
  bool IsTypeRoute(void)const;
  bool IsTypeData(void)const;
  void SetSrcId(uint32_t id);
  uint8_t GetSrcId();
  void SetSrcWaitSize(uint32_t size);
  uint8_t GetSrcWaitSize();
 //设置ttl
  void SetTTL(void);
  void SetTTL(uint8_t ttl);
  uint8_t GetTTL(void)const;
  bool IsTTLEnd(void)const;
  uint16_t GetSize(void)const;
  uint32_t GetFrameControl(void)const;
  void SetFrameControl(uint32_t frameControl);
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

  uint16_t CalculateHeaderChecksum (uint16_t size) const;


  //报文格式
  Mac48Address m_src;
  Mac48Address m_dest;
  Mac48Address m_transmit;
  uint8_t m_seq;
  uint8_t m_type;
  uint8_t m_ttl;
  uint8_t m_srcId;
  uint8_t m_srcWaitSize;

  uint16_t m_checksum;
  bool m_calcChecksum;        //!< Flag to calculate checksum
  bool m_goodChecksum;        //!< Flag to indicate that checksum is correct

};

} // namespace ns3

#endif
