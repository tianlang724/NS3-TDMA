/*
 * tdma-mac-header.cc
 *
 *  Created on: Mar 30, 2016
 *      Author: zh
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "tdma-mac-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaHeader);

TdmaHeader::TdmaHeader ()
:m_seq(0),
 m_ttl(0),
 m_srcId(0),
 m_srcWaitSize(),
 m_calcChecksum(false),
  m_goodChecksum(true)
{}

TdmaHeader::  ~TdmaHeader()
  {}
TypeId
TdmaHeader::GetTypeId (void)
{
	 static TypeId tid = TypeId ("ns3::TdmaHeader")
	    .SetParent<Header> ()
	    .AddConstructor<TdmaHeader> ()
	    ;
	  return tid;
}
TypeId
TdmaHeader::GetInstanceTypeId (void) const
{
	  return GetTypeId ();
}
void
TdmaHeader::Print (std::ostream &os) const
{
	  os << "length: " <<  GetSerializedSize ()
	     << " "
	     << m_src << " > " << m_seq
	  ;
}
uint32_t
TdmaHeader::GetSerializedSize (void) const
{
	return GetSize();
}

uint16_t
TdmaHeader::CalculateHeaderChecksum (uint16_t size) const
{

	/*Buffer buf = Buffer (10);
	buf.AddAtStart (10);
	Buffer::Iterator it = buf.Begin ();
	it.WriteU16(m_src);
	it.WriteU16(m_seq);
	it.WriteU16(m_type);
	it.WriteU16(m_payloadSize);
	it=buf.Begin();

	return ~(it.CalculateIpChecksum(8));
	*/
	return 0;
}
uint32_t
TdmaHeader::GetFrameControl(void)const
{
	uint32_t val=0;
	val|=m_ttl&0xf;
	val|=(m_type<<4)&(0xf<<4);
	val|=(m_seq<<8)&(0xff<<8);
	val|=(m_srcId<<16)&(0xff<<16);
	val|=(m_srcWaitSize<<24)&(0xff<<24);
	return val;
}
void
TdmaHeader::SetFrameControl(uint32_t frameControl)
{
	m_ttl=frameControl&0x0f;
	m_type=(frameControl>>4)&0x0f;
	m_seq=(frameControl>>8)&0xff;
	m_srcId=(frameControl>>16)&0xff;
	m_srcWaitSize=(frameControl>>24)&0xff;
}
void
TdmaHeader::Serialize (Buffer::Iterator start) const
{
	Buffer::Iterator i=start;
	WriteTo(i,m_src);
	WriteTo(i,m_dest);
	WriteTo(i,m_transmit);
	i.WriteHtonU32(GetFrameControl());
	//i.WriteHtolsbU16(m_seq);
   /*
	  if (m_checksum == 0)
	    {
	      i.WriteU16 (0);
	      if (m_calcChecksum)
	        {
	          uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
	          i.WriteU16 (headerChecksum);
	        }
	    }
	  else
	    {
	      i.WriteU16 (m_checksum);
	    }
	   */

}
uint32_t
TdmaHeader::Deserialize (Buffer::Iterator start)
{
	  Buffer::Iterator i = start;
	  ReadFrom(i,m_src);
	  ReadFrom(i,m_dest);
	  ReadFrom(i,m_transmit);
	  uint32_t frameControl=i.ReadNtohU32();
	  SetFrameControl(frameControl);
	  //test!
	  //m_seq=i.ReadNtohU16();
	  return GetSerializedSize ();
}
//设置源端口
void
TdmaHeader:: SetSrc(Mac48Address src)
  {
   m_src=src;
  }
Mac48Address
TdmaHeader::GetSrc(void)const
{
	 return m_src;
}
void
TdmaHeader:: SetDest(Mac48Address dest)
  {
   m_dest=dest;
  }
Mac48Address
TdmaHeader::GetDest(void)const
{
	 return m_dest;
}
//设置序列号
void
TdmaHeader::SetSeq(uint8_t seq)
  {
	  m_seq=seq;
  }

uint8_t
TdmaHeader::GetSeq(void)const
{
	 return m_seq;
}
void
TdmaHeader::SetTransmit(Mac48Address tran)
{
	m_transmit=tran;
}
Mac48Address
TdmaHeader::GetTransmit()
{
	return m_transmit;
}
//设置类型

void
TdmaHeader::SetTypeRoute(void)
{
	m_type=ROUTE;
}
void
TdmaHeader::SetTypeData(void)
{
	m_type=DATA;
}

bool
TdmaHeader::IsTypeRoute(void)const
{
	return (m_type==ROUTE);
}
bool
TdmaHeader::IsTypeData(void)const
{
	return (m_type==DATA);
}
void
TdmaHeader::SetSrcId(uint32_t id)
{
	m_srcId=id&0x0f;
}
uint8_t
TdmaHeader::GetSrcId()
{
	return m_srcId;
}
void
TdmaHeader::SetSrcWaitSize(uint32_t size)
{
	m_srcWaitSize=size&0x0f;
}
uint8_t
TdmaHeader::GetSrcWaitSize()
{
	return m_srcWaitSize;
}
void
TdmaHeader::SetTTL(void)
{
	m_ttl++;
}
void
TdmaHeader::SetTTL(uint8_t ttl)
{
	m_ttl=ttl+1;
}
uint8_t
TdmaHeader::GetTTL(void) const
{
	return m_ttl;
}
bool
TdmaHeader::IsTTLEnd(void)const
{
	if(m_ttl>=3)
		return true;
	else
		return false;
}

uint16_t
TdmaHeader::GetSize(void)const
{
	 return 22;
}
//发送包时候计算校验和
void
TdmaHeader::CalcChecksum()
{

}
 //初始化校验
void
TdmaHeader::InitializeChecksum()
{
	/*
	Buffer buf = Buffer (10);
	buf.AddAtStart (10);
	Buffer::Iterator it = buf.Begin ();
	it.WriteU16(m_src);
	it.WriteU16(m_seq);
	it.WriteU16(m_type);
	it.WriteU16(m_payloadSize);
	it=buf.Begin();  */
}



  //返回校验和是否正确
bool
TdmaHeader::IsChecksumOk (void) const
{
	return m_goodChecksum;
}

uint16_t
TdmaHeader::GetChecksum ()
{
	return m_checksum;
}



//测试
/*
int main (int argc, char *argv[])
{
  // Enable the packet printing through Packet::Print command.
  //Packet::EnablePrinting ();

  // instantiate a header.
  TdmaHeader sourceHeader;
  sourceHeader.SetSrc (2);
  sourceHeader.SetSeq(1);
  sourceHeader.SetTpye(1);
  sourceHeader.SetPayloadSize(10);

  // instantiate a packet
  Ptr<Packet> p = Create<Packet> ();

  // and store my header into the packet.
  p->AddHeader (sourceHeader);

  // print the content of my packet on the standard output.
  p->Print (std::cout);
  std::cout << std::endl;

  // you can now remove the header from the packet:
  TdmaHeader destinationHeader;
  p->RemoveHeader (destinationHeader);

  // and check that the destination and source
  // headers contain the same values.
  NS_ASSERT (destinationHeader.IsChecksumOk ());

  return 0;
}   */


} // namespace ns3

