/*
 * tdma-mac-header.cc
 *
 *  Created on: Mar 30, 2016
 *      Author: zh
 */

#include "tdma-mac-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaHeader);
TdmaHeader::TdmaHeader ()
:m_src(0xfffd),
 m_seq(0),
 m_type(DATA),
 m_payloadSize(0),
 m_checksum(0xfffd),
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
	return 10;
}

uint16_t
TdmaHeader::CalculateHeaderChecksum (uint16_t size) const
{
	Buffer buf = Buffer (10);
	buf.AddAtStart (10);
	Buffer::Iterator it = buf.Begin ();
	it.WriteU16(m_src);
	it.WriteU16(m_seq);
	it.WriteU16(m_type);
	it.WriteU16(m_payloadSize);
	it=buf.Begin();

	return ~(it.CalculateIpChecksum(10));
}

void
TdmaHeader::Serialize (Buffer::Iterator start) const
{
	Buffer::Iterator i=start;
	i.WriteHtonU16(m_src);
	i.WriteHtonU16(m_seq);
	i.WriteHtonU16(m_type);
	//i.WriteHtonU16(m_checksum);
	 if (m_payloadSize == 0)
	    {
	      i.WriteHtonU16 (start.GetSize ());
	    }
	  else
	    {
	      i.WriteHtonU16 (m_payloadSize);
	    }
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

}
uint32_t
TdmaHeader::Deserialize (Buffer::Iterator start)
{
	  Buffer::Iterator i = start;
	  m_src = i.ReadNtohU16 ();
	  m_seq = i.ReadNtohU16 ();
	  m_type = i.ReadNtohU16 ();
	  m_payloadSize = i.ReadNtohU16 () - GetSerializedSize ();
	  m_checksum = i.ReadU16 ();
	  if (m_calcChecksum)
	    {
	      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
	      m_goodChecksum = (headerChecksum == 0);
	    }
	  return GetSerializedSize ();
}
//设置源端口
void
TdmaHeader:: SetSrc(uint16_t src)
  {
   m_src=src;
  }
//设置序列号
void
TdmaHeader::SetSeq(uint16_t seq)
  {
	  m_seq=seq;
  }
//设置类型
void
TdmaHeader::SetTpye(uint16_t type)
{
	m_type=type;
}

void
TdmaHeader::SetPayloadSize(uint16_t size)
{
	m_payloadSize=size;
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
	Buffer buf = Buffer (10);
	buf.AddAtStart (10);
	Buffer::Iterator it = buf.Begin ();
	it.WriteU16(m_src);
	it.WriteU16(m_seq);
	it.WriteU16(m_type);
	it.WriteU16(m_payloadSize);
	it=buf.Begin();
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
}


} // namespace ns3

