/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Hemanth Narra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hemanth Narra <hemanthnarra222@gmail.com>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-mac-header.h"
#include "tdma-central-mac.h"
#include "ns3/error-model.h"
#include <fstream>



NS_LOG_COMPONENT_DEFINE ("TdmaCentralMac");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (TdmaCentralMac);

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) {std::clog << "[TdmaMac=" << m_low->GetAddress () << "] "; }

TypeId
TdmaCentralMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TdmaCentralMac")
    .SetParent<TdmaMac> ()
    .AddConstructor<TdmaCentralMac> ()
    .AddTraceSource ("MacTx",
                     "A packet has been received from higher layers and is being processed in preparation for "
                     "queueing for transmission.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macTxTrace))
    .AddTraceSource ("MacTxDrop",
                     "A packet has been dropped in the MAC layer before being queued for transmission.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macRxTrace))
    .AddTraceSource ("MacRxDrop",
                     "A packet has been dropped in the MAC layer after it has been passed up from the physical "
                     "layer.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macRxDropTrace))
  ;
  return tid;
}

TdmaCentralMac::TdmaCentralMac ()
{
  NS_LOG_FUNCTION (this);
  m_isTdmaRunning = false;
  m_low = CreateObject<TdmaMacLow> ();
  m_queue = CreateObject<TdmaMacQueue> ();
  m_queue->SetTdmaMacTxDropCallback (MakeCallback (&TdmaCentralMac::NotifyTxDrop, this));
  m_recvQueueMaxSize=50;
  m_seq=0;
  m_recvPackets=0;
  m_bytesTotal=0;
  m_delayTotal=0;
  m_ownPackets=0;
  m_ttlPackets=0;
  m_repeatePackets=0;
  m_sendPackets=0;

  /*
  m_rem = CreateObject<RateErrorModel> ();
  m_uv = CreateObject< UniformRandomVariable > ();
  m_rem->SetRandomVariable(m_uv);
  m_rem->SetRate (0.05);  */
  /*std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "delay,"<<
  std::endl;
  out.close ();  */
}
TdmaCentralMac::~TdmaCentralMac ()
{
}

void
TdmaCentralMac::DoDispose (void)
{
  m_low->Dispose ();
  m_low = 0;
  m_device = 0;
  m_queue = 0;
  //m_tdmaController = 0;
  m_tdmaControllerDynamic = 0;
  TdmaMac::DoDispose ();
}

void
TdmaCentralMac::NotifyTx (Ptr<const Packet> packet)
{
  m_macTxTrace (packet);
}

void
TdmaCentralMac::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_macTxDropTrace (packet);
}

void
TdmaCentralMac::NotifyRx (Ptr<const Packet> packet)
{
  m_macRxTrace (packet);
}

void
TdmaCentralMac::NotifyPromiscRx (Ptr<const Packet> packet)
{
  m_macPromiscRxTrace (packet);
}

void
TdmaCentralMac::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_macRxDropTrace (packet);
}

void
TdmaCentralMac::SetChannel (Ptr<SimpleWirelessChannel> channel)
{
  if (channel != 0)
    {
      m_channel = channel;
      //m_tdmaController->SetChannel (channel);
      m_tdmaControllerDynamic->SetChannel (channel);
      m_low->SetChannel (m_channel);
    }
}

Ptr<SimpleWirelessChannel>
TdmaCentralMac::GetChannel (void) const
{
  return m_channel;
}

void
TdmaCentralMac::SetTdmaController (Ptr<TdmaController> controller)
{
  m_tdmaController = controller;
}

Ptr<TdmaController>
TdmaCentralMac::GetTdmaController (void) const
{
  return m_tdmaController;
}
void
TdmaCentralMac::SetTdmaControllerDynamic (Ptr<TdmaControllerDynamic> controller)
{
  m_tdmaControllerDynamic = controller;
}

Ptr<TdmaControllerDynamic>
TdmaCentralMac::GetTdmaControllerDynamic (void) const
{
  return m_tdmaControllerDynamic;
}

void
TdmaCentralMac::SetDevice (Ptr<TdmaNetDevice> device)
{
  m_device = device;
  m_low->SetDevice (m_device);
}

uint8_t
TdmaCentralMac::GetNewSeq()
{
	m_seq++;
	if(m_seq==0xff)
		m_seq=0;
	return m_seq;
}
Ptr<TdmaNetDevice>
TdmaCentralMac::GetDevice (void) const
{
  return m_device;
}

Ptr<TdmaMacLow>
TdmaCentralMac::GetTdmaMacLow (void) const
{
  return m_low;
}
void
TdmaCentralMac::SetId (uint32_t id)
{
	  std::stringstream ss;
	  ss<<"mac"<<id<<std::endl;
	  std::string CSVfileName=ss.str()+".csv";
	  std::ofstream out (CSVfileName.c_str ());
	  out << "SimulationSecond," <<
			 "ReceiveRate,"<<
	  "PacketsReceived," <<
	  "ttlPackets,"<<
	  "ownPackets,"<<
	  "repeatPacket,"<<
	  "delay,"<<
	  "use,"<<
	  "sendPackets"<<
	  std::endl;
	  out.close ();
	  m_CSVfileName=CSVfileName;
	  m_nodeId=id;
	  CheckData();
}
void
TdmaCentralMac::SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback)
{
  NS_LOG_FUNCTION (this);
  m_upCallback = upCallback;
}

void
TdmaCentralMac::SetLinkUpCallback (Callback<void> linkUp)
{
  linkUp ();
}

void
TdmaCentralMac::SetTxQueueStartCallback (Callback<bool,uint32_t> queueStart)
{
  NS_LOG_FUNCTION (this);
  m_queueStart = queueStart;
}

void
TdmaCentralMac::SetTxQueueStopCallback (Callback<bool,uint32_t> queueStop)
{
  NS_LOG_FUNCTION (this);
  m_queueStop = queueStop;
}

uint32_t
TdmaCentralMac::GetQueueState (uint32_t index)
{
  if (m_queue->GetMaxSize () == m_queue->GetSize ())
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

uint32_t
TdmaCentralMac::GetNQueues (void)
{
  //TDMA currently has only one queue
  return 1;
}
uint32_t
TdmaCentralMac::GetQueueSize()
{
	return m_queue->GetSize();
}
void
TdmaCentralMac::SetLinkDownCallback (Callback<void> linkDown)
{
}

void
TdmaCentralMac::SetMaxQueueSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_queue->SetMaxSize (size);
}
void
TdmaCentralMac::SetMaxQueueDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_queue->SetMaxDelay (delay);
}


Mac48Address
TdmaCentralMac::GetAddress (void) const
{
  return m_low->GetAddress ();
}
Ssid
TdmaCentralMac::GetSsid (void) const
{
  return m_ssid;
}
void
TdmaCentralMac::SetAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (address);
  m_low->SetAddress (address);
  m_low->SetBssid (address);
}
void
TdmaCentralMac::SetSsid (Ssid ssid)
{
  NS_LOG_FUNCTION (ssid);
  m_ssid = ssid;
}
Mac48Address
TdmaCentralMac::GetBssid (void) const
{
  return m_low->GetBssid ();
}

void
TdmaCentralMac::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from);
  //NotifyRx(packet);
  m_upCallback (packet, from, to);
}

void
TdmaCentralMac::Queue (Ptr<const Packet> packet, const TdmaHeader &hdr)
{
  NS_LOG_FUNCTION (this << packet << &hdr);
  if (!m_queue->Enqueue (packet, hdr))
    {
      NotifyTxDrop (packet);
    }
  //else
  //NS_LOG_DEBUG ("Packet Queue success in m_queue");

  //Cannot request for channel access in tdma. Tdma schedules every node in round robin manner
  //RequestForChannelAccess();
}

void
TdmaCentralMac::StartTransmission (uint64_t transmissionTimeUs)
{
 // NS_LOG_DEBUG (transmissionTimeUs << " usec");
  Time totalTransmissionSlot = MicroSeconds (transmissionTimeUs);
  if (m_queue->IsEmpty ())
    {
      NS_LOG_DEBUG ("queue empty");
      return;
    }
  TdmaHeader header;
  Ptr< const Packet> peekPacket = m_queue->Peek (&header);
  Time packetTransmissionTime = m_tdmaControllerDynamic->CalculateTxTime (peekPacket);
  NS_LOG_DEBUG ("Packet TransmissionTime(microSeconds): " << packetTransmissionTime.GetMicroSeconds () << "usec");
  if (packetTransmissionTime < totalTransmissionSlot)
    {
      totalTransmissionSlot -= packetTransmissionTime;
      Simulator::Schedule (packetTransmissionTime, &TdmaCentralMac::SendPacketDown, this,totalTransmissionSlot);
    }
  else
    {
      NS_LOG_DEBUG ("Packet takes more time to transmit than the slot allotted. Will send in next slot");
    }
}
void
TdmaCentralMac::StartRouteTransmission (uint64_t transmissionTime,Ptr<Packet> packet)
{
	  Time totalTransmissionSlot = MicroSeconds (transmissionTime);
	  //设置报头
	  TdmaHeader newHdr;
	  newHdr.SetDest (Mac48Address::GetBroadcast());
	  newHdr.SetSrc (m_low->GetAddress());
	  newHdr.SetSeq(GetNewSeq());
	  newHdr.SetSrcId(m_nodeId);
	  newHdr.SetSrcWaitSize(m_queue->GetSize());
	  newHdr.SetTransmit(m_low->GetAddress());
	  newHdr.SetTypeRoute();
	  //设置时间标签
	  TdmaTag newTag;
	  Time time=Simulator::Now();
	  newTag.SetTimeStamp(time);
	  packet->AddPacketTag(newTag);

	  Time packetTransmissionTime = m_tdmaControllerDynamic->CalculateTxTime (packet);
	  NS_LOG_DEBUG ("Packet TransmissionTime(microSeconds): " << packetTransmissionTime.GetMicroSeconds () << "usec");
	  if (packetTransmissionTime < totalTransmissionSlot)
	    {
	      totalTransmissionSlot -= packetTransmissionTime;
	      Simulator::Schedule (packetTransmissionTime, &TdmaCentralMac::SendRouteDown, this,totalTransmissionSlot,packet,&newHdr);
	    }
	  else
	    {
	      NS_LOG_DEBUG ("Packet takes more time to transmit than the slot allotted. Will send in next slot");
	    }

}
void
TdmaCentralMac::SendRouteDown (Time remainingTime,Ptr<Packet>packet,TdmaHeader *hdr)
{
	  m_low->StartTransmission (packet, hdr);
}
void
TdmaCentralMac::SendPacketDown (Time remainingTime)
{
  TdmaHeader header;
  Ptr<const Packet> packet = m_queue->Dequeue (&header);
  m_low->StartTransmission (packet, &header);
  m_sendPackets+=1;
  TxQueueStart (0);
  NotifyTx (packet);
  TxQueueStart (0);
  StartTransmission (remainingTime.GetMicroSeconds ());
}

void
TdmaCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (this << packet << to << from);
  TdmaHeader hdr;
  hdr.SetDest (to);
  hdr.SetSrc (from);
  //增加序列号和类型字段
  hdr.SetSeq(GetNewSeq());

 /* TdmaTag tag;
  Time now=Simulator::Now();
  tag.SetTimeStamp(now);
  packet->AddPacketTag(tag);
  NS_LOG_DEBUG("Packet" <<packet->GetUid()<<"add timestamp" <<now);*/
  Queue (packet, hdr);
}
void
TdmaCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);

  TdmaHeader hdr;
  hdr.SetDest (to);
  hdr.SetSrc (m_low->GetAddress ());
  hdr.SetSeq(GetNewSeq());
  //设置路由信息
  hdr.SetSrcId(m_nodeId);
  hdr.SetSrcWaitSize(m_queue->GetSize());
  hdr.SetTransmit(m_low->GetAddress ());
  hdr.SetTypeData();

  TdmaTag tag;
  Time now=Simulator::Now();
  tag.SetTimeStamp(now);
  packet->AddPacketTag(tag);
  NS_LOG_DEBUG("Packet" <<packet->GetUid()<<"add timestamp" <<now);
  Queue (packet, hdr);
  NS_LOG_FUNCTION (this << packet << to);
}
void
TdmaCentralMac::Enqueue (Ptr<const Packet> packet, TdmaHeader hdr,TdmaTag tag)
{

  TdmaHeader newHdr;
  newHdr.SetDest (hdr.GetDest());
  newHdr.SetSrc (hdr.GetSrc());
  newHdr.SetSeq(hdr.GetSeq());
  newHdr.SetTTL(hdr.GetTTL());
  //携带转发节点信息
  newHdr.SetSrcId(m_nodeId);
  newHdr.SetSrcWaitSize(m_queue->GetSize());
  newHdr.SetTransmit(m_low->GetAddress());
  newHdr.SetTypeData();
  TdmaTag newTag;
  newTag.SetTimeStamp(tag.GetSTimeStamp());
  packet->AddPacketTag(newTag);
  Queue (packet, newHdr);
}
bool
TdmaCentralMac::SupportsSendFrom (void) const
{
  return true;
}

void
TdmaCentralMac::TxOk (const TdmaHeader &hdr)
{
}
void
TdmaCentralMac::TxFailed (const TdmaHeader &hdr)
{
}

void
TdmaCentralMac::TxQueueStart (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStart (index);
}
void
TdmaCentralMac::TxQueueStop (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStop (index);
}

void
TdmaCentralMac::Receive (Ptr<Packet> packet, const TdmaHeader *hdr)
{

	/*if (m_rem->IsCorrupt (packet))
			  {
			   return ;
			  }  */

	Time delay;
	Time now=Simulator::Now();
	TdmaTag tag;
	packet->RemovePacketTag(tag);
	delay=now-tag.GetSTimeStamp();
	NS_LOG_DEBUG("time:"<<now <<"Receive Packet "<<packet->GetUid()<<"from "<<hdr->GetSrc() <<"seq="<<(uint16_t)hdr->GetSeq()
			<<"ttl="<<(uint16_t)hdr->GetTTL()<<"delay= " <<delay);
	//ForwardUp (packet, hdr->GetSrc (), hdr->GetDest ());
	//即使时重复包或者超时的包，都会有转发节点的信息，所以要进行处理；
	m_tdmaControllerDynamic->RecvRouterMsg(*hdr);
	//数据包进行处理
	if(hdr->IsTypeData())
	{
	    m_recvPackets += 1;
		//自己发出的包直接丢掉
		if(hdr->GetSrc()==m_low->GetAddress())
		{
			NS_LOG_DEBUG("from this node ,throw packet");
			m_ownPackets+=1;
			return ;
		}
		//重复包判定
		std::deque<TdmaHeader >::iterator iter;
		iter=m_recvPacketsQueue.begin();
		for(;iter!=m_recvPacketsQueue.end();iter++)
		{
			if((iter->GetSeq()==hdr->GetSeq())&&(iter->GetSrc()==hdr->GetSrc()))
			{
				//重复包
				m_repeatePackets++;
				m_recvPacketsQueue.erase(iter);
				m_recvPacketsQueue.push_back(*hdr);
				return ;
			}
		}
		//判断缓冲队列是否已经满
		if(m_recvPacketsQueue.size()==m_recvQueueMaxSize)
			m_recvPacketsQueue.pop_front();
		m_recvPacketsQueue.push_back(*hdr);


		Ptr<const Packet> p=packet->Copy();  //必须在向上提交 之前复制！！！！！

		//广播直接提交上一层
		ForwardUp (packet, hdr->GetSrc (), hdr->GetDest ());
		m_bytesTotal+= packet->GetSize ();
		m_delayTotal+=delay.GetNanoSeconds();
		//判断是否需要转发
		if(hdr->IsTTLEnd())
		{
			NS_LOG_DEBUG("ttl over,throw packet");
			m_ttlPackets+=1;
			return ;
		}
		//转发参数判断
		if(m_tdmaControllerDynamic->IsTransmit())
		{
			Enqueue (p,*hdr,tag);
		}
	}
	else  //信息包提交给controller
	{
		m_tdmaControllerDynamic->ReceiveSlotMsg(packet);
	}

}

void
TdmaCentralMac::DoInitialize (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_isTdmaRunning = true;
  m_queue->SetMacPtr (this);
  //m_tdmaController->Start ();
  m_tdmaControllerDynamic->Start ();
  m_low->SetRxCallback (MakeCallback (&TdmaCentralMac::Receive, this));
  TdmaMac::DoInitialize ();
}


void
TdmaCentralMac::CheckData()//CSV data
{
	double kbs = (m_bytesTotal * 8.0) / 1000;
	  uint64_t delayone=0;
	  m_bytesTotal = 0;
	  if(m_recvPackets>0)
		  delayone=m_delayTotal/m_recvPackets;
	  else
		  delayone=0;
	  double use=0;
	  uint64_t  sum;
	  sum=m_recvPackets-m_repeatePackets-m_ownPackets;
	  if(sum>0)
		  use=(double)m_recvPackets/sum;
	  else
		  sum=0;
	  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

	  out << (Simulator::Now ()).GetSeconds () <<","<< kbs<<","<< m_recvPackets << ","<<m_ttlPackets<<","<<m_ownPackets<<","
			  <<m_repeatePackets<<","<<delayone<<","<<use<<","<<m_sendPackets<<std::endl;

	  out.close ();
	  m_recvPackets = 0;
	  m_delayTotal=0;
	  m_ttlPackets=0;
	  m_ownPackets=0;
	  m_repeatePackets=0;
	  m_sendPackets=0;
	  Simulator::Schedule (Seconds (150.0), &TdmaCentralMac::CheckData, this);
}

} // namespace ns3
