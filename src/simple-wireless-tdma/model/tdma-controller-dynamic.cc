/*
 * tdma-controller-dynamic.cc
 *
 *  Created on: Apr 26, 2016
 *      Author: zh
 */
#include "ns3/assert.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "tdma-mac.h"
#include "tdma-controller-dynamic.h"
#include "tdma-mac.h"
#include "tdma-mac-low.h"
#include "tdma-route-table.h"
#include "tdma-slot.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("TdmaControllerDynamic");

#define MY_DEBUG(x) \
  NS_LOG_DEBUG (Simulator::Now () << " " << this << " " << x)

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (TdmaControllerDynamic);


Time
TdmaControllerDynamic::GetDefaultSlotTime (void)
{
  return MicroSeconds (1200);
}

Time
TdmaControllerDynamic::GetDefaultGaurdTime (void)
{
  return MicroSeconds (50);
}

DataRate
TdmaControllerDynamic::GetDefaultDataRate (void)
{
  NS_LOG_DEBUG ("Setting default");
  return DataRate ("800000b/s");
}


/*************************************************************
 * Tdma Controller Class Functions
 ************************************************************/
TypeId
TdmaControllerDynamic::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3:TdmaControllerDynamic")
    .SetParent<Object> ()
    .AddConstructor<TdmaControllerDynamic> ()
    .AddAttribute ("DataRate",
                   "The default data rate for point to point links",
                   DataRateValue (GetDefaultDataRate ()),
                   MakeDataRateAccessor (&TdmaControllerDynamic::SetDataRate,
                                         &TdmaControllerDynamic::GetDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("SlotTime", "The duration of a Slot in microseconds.",
                   TimeValue (GetDefaultSlotTime ()),
                   MakeTimeAccessor (&TdmaControllerDynamic::SetSlotTime,
                                     &TdmaControllerDynamic::GetSlotTime),
                   MakeTimeChecker ())
    .AddAttribute ("GaurdTime", "GaurdTime between TDMA slots in microseconds.",
                   TimeValue (GetDefaultGaurdTime ()),
                   MakeTimeAccessor (&TdmaControllerDynamic::SetGaurdTime,
                                     &TdmaControllerDynamic::GetGaurdTime),
                   MakeTimeChecker ())
    .AddAttribute ("InterFrameTime", "The wait time between consecutive tdma frames.",
                   TimeValue (MicroSeconds (0)),
                   MakeTimeAccessor (&TdmaControllerDynamic::SetInterFrameTimeInterval,
                                     &TdmaControllerDynamic::GetInterFrameTimeInterval),
                   MakeTimeChecker ());
  return tid;
}

TdmaControllerDynamic::TdmaControllerDynamic () : m_activeEpoch (false)
{
  NS_LOG_FUNCTION (this);
 // m_slotTime=1100;
 // m_gaurdTime=100;
  m_tdmaInterFrameTime=0;
  m_tdmaRouteTable=CreateObject<TdmaRouteTable>();
  //m_tdmaRouteTable->SetTdmaControllerDynamic(this);
  m_isTransmit=true;
  m_transmitPara=0.1;
  m_dynamicInterval=0;
  m_staticCounnt=0;
  m_data=0;
  m_dataRecv=0;
  m_sessionState=STATIC;
  m_reserveSlotTime=500;
}

TdmaControllerDynamic::~TdmaControllerDynamic ()
{

}
void
TdmaControllerDynamic::DoDispose(void)
{
	  m_bps = 0;
	  m_tdmaRouteTable=0;


	  if(m_data!=0)
	  {
		  delete [] m_data;
		  m_data = 0;
	  }
	  if(m_dataRecv!=0)
	  {
		  delete [] m_dataRecv;
		  m_dataRecv = 0;
	  }

}
void
TdmaControllerDynamic::Start (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_activeEpoch)
    {
	  //构建动态时隙表
	  uint32_t i=0;
	  TdmaSlot slot;
	  for(i=0;i<m_totalSlotsAllowed;i++)
	  {
		  m_slotTable.insert(std::pair<uint32_t,TdmaSlot >(i,slot));
	  }
	  m_dataSize=m_slotTable.size()*(sizeof(uint32_t)+sizeof(TdmaSlot));
	  m_data=new uint8_t[m_dataSize];
	  m_dataSizeRecv=m_dataSize;
	  m_dataRecv=new uint8_t[m_dataSizeRecv];
      m_activeEpoch = true;
      if(m_dynamicInterval>0)
    	  m_sessionState=RESERVE;
      Simulator::Schedule (NanoSeconds (10),&TdmaControllerDynamic::StartTdmaSessions, this);
    }
}

void
TdmaControllerDynamic::StartTdmaSessions (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  ScheduleTdmaSession (0);
}


void
TdmaControllerDynamic::SetSlotTime (Time slotTime)
{
  NS_LOG_FUNCTION (this << slotTime);
  m_slotTime = slotTime.GetMicroSeconds ();
}
void
TdmaControllerDynamic::SetReserveSlotTime(Time slotTime)
{
	  NS_LOG_FUNCTION (this << slotTime);
	  m_reserveSlotTime = slotTime.GetMicroSeconds ();
}
Time
TdmaControllerDynamic::GetSlotTime (void) const
{
  return MicroSeconds (m_slotTime);
}
Time
TdmaControllerDynamic::GetReserveSlotTime(void) const
{
	return MicroSeconds(m_reserveSlotTime);
}
void
TdmaControllerDynamic::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this << bps);
  m_bps = bps;
}

DataRate
TdmaControllerDynamic::GetDataRate (void) const
{
  return m_bps;
}

void
TdmaControllerDynamic::SetChannel (Ptr<SimpleWirelessChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}


Ptr<SimpleWirelessChannel>
TdmaControllerDynamic::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}
void
TdmaControllerDynamic::SetMacCentral(Ptr<TdmaMac> mac)
{
	NS_LOG_FUNCTION (this << mac);
	m_mac=mac;
}
Ptr<TdmaMac>
TdmaControllerDynamic::GetMacCentral(void) const
{
	return m_mac;
}
void
TdmaControllerDynamic::SetGaurdTime (Time gaurdTime)
{
  NS_LOG_FUNCTION (this << gaurdTime);
  //gaurdTime is based on the SimpleWirelessChannel's max range
  if (m_channel != 0)
    {
      m_gaurdTime = Seconds (m_channel->GetMaxRange () / 300000000.0).GetMicroSeconds ();
    }
  else
    {
      m_gaurdTime = gaurdTime.GetMicroSeconds ();
    }
}

Time
TdmaControllerDynamic::GetGaurdTime (void) const
{
  return MicroSeconds (m_gaurdTime);
}

void
TdmaControllerDynamic::SetInterFrameTimeInterval (Time interFrameTime)
{
  NS_LOG_FUNCTION (interFrameTime);
  m_tdmaInterFrameTime = interFrameTime.GetMicroSeconds ();
}

Time
TdmaControllerDynamic::GetInterFrameTimeInterval (void) const
{
  return MicroSeconds (m_tdmaInterFrameTime);
}

void
TdmaControllerDynamic::SetTotalSlotsAllowed (uint32_t slotsAllowed)
{
  m_totalSlotsAllowed = slotsAllowed;
}

uint32_t
TdmaControllerDynamic::GetTotalSlotsAllowed (void) const
{
  return m_totalSlotsAllowed;
}

void
TdmaControllerDynamic::ScheduleTdmaSession (const uint32_t slotNum)
{

	switch(m_sessionState)
	{
	case STATIC:
	{
		NS_LOG_FUNCTION (slotNum);
		  Time transmissionSlot = MicroSeconds(GetSlotTime ().GetMicroSeconds ());
		  Time totalTransmissionTimeUs = GetGaurdTime () + transmissionSlot;
		  //是该节点的的时隙，进行操作
		  if(slotNum==m_mac->GetDevice()->GetNode()->GetId())
		  {
			  //调用tdma central mac
			  m_mac->StartTransmission (transmissionSlot.GetMicroSeconds ());
		  }
		  //操作完成或者不是该节点时隙进入下一个时隙或者下一帧
		  if (slotNum +1== GetTotalSlotsAllowed ())
		    {
		      NS_LOG_DEBUG ("Starting over static sessions ");
		      if(m_dynamicInterval>0)
		      {
			      m_staticCounnt++;
		    	  if(m_staticCounnt>m_dynamicInterval)
		    	  {
		    		  m_sessionState=DYNAMIC;
		    		  CheckSlotTable();
		    		  m_staticCounnt=0;
		    	  }
		      }
		      Simulator::Schedule ((totalTransmissionTimeUs + GetInterFrameTimeInterval ()), &TdmaControllerDynamic::StartTdmaSessions, this);
		    }
		  else
		    {
		      NS_LOG_DEBUG ("Scheduling next session");
		      Simulator::Schedule (totalTransmissionTimeUs, &TdmaControllerDynamic::ScheduleTdmaSession, this, (slotNum+1));
		    }
		  break;
	}
	case RESERVE:
	{
		  Time transmissionSlot = MicroSeconds(GetReserveSlotTime().GetMicroSeconds ());
		  Time totalTransmissionTimeUs = GetGaurdTime () + transmissionSlot;
		  if(slotNum==m_mac->GetDevice()->GetNode()->GetId())
		  {
			  //判断自己是否需要预约时隙
			  ReserveSlot();
			  Ptr<Packet> packet=SetSlotPacket();
			  //调用tdma central mac
			 m_mac->StartRouteTransmission (transmissionSlot.GetMicroSeconds (),packet);
			 NS_LOG_DEBUG("mac="<<m_mac->GetAddress());
			std::map<uint32_t,TdmaSlot>::iterator it;
			it=m_slotTable.begin();
			for(;it!=m_slotTable.end();it++)
			{
			NS_LOG_DEBUG("my slot to send index="<<it->first<<"id(prority)="<<it->second.GetSlotIndex()<<"mac="<<it->second.GetOwner()<<
							"state="<<it->second.GetState());
			}
		  }
		  if (slotNum +1== GetTotalSlotsAllowed ())
		    {

		      NS_LOG_DEBUG ("Starting over reserve sessions ");
		  	  m_sessionState=STATIC;
		      Simulator::Schedule ((totalTransmissionTimeUs + GetInterFrameTimeInterval ()), &TdmaControllerDynamic::StartTdmaSessions, this);
		    }
		  else
		    {
		      NS_LOG_DEBUG ("Scheduling next session");
		      Simulator::Schedule (totalTransmissionTimeUs, &TdmaControllerDynamic::ScheduleTdmaSession, this, (slotNum+1));
		    }
		  break;
	}
	case DYNAMIC:
	{

		 NS_LOG_DEBUG("mac="<<m_mac->GetAddress());
		std::map<uint32_t,TdmaSlot>::iterator it;
		it=m_slotTable.begin();
		for(;it!=m_slotTable.end();it++)
		{
		NS_LOG_DEBUG("my last index="<<it->first<<"id(prority)="<<it->second.GetSlotIndex()<<"mac="<<it->second.GetOwner()<<
						"state="<<it->second.GetState());
		}
		  Time transmissionSlot = MicroSeconds(GetSlotTime ().GetMicroSeconds ());
		  Time totalTransmissionTimeUs = GetGaurdTime () + transmissionSlot;
		  it=m_slotTable.find(slotNum);
		  if(it->second.GetOwner()==m_mac->GetAddress())
		  {
			  m_mac->StartTransmission (transmissionSlot.GetMicroSeconds ());
		  }
		  if (slotNum +1== GetTotalSlotsAllowed ())
		  {

				  NS_LOG_DEBUG ("Starting over dynamic sessions ");
				 m_sessionState=RESERVE;
				 std::map<uint32_t,TdmaSlot>::iterator it;
				 it=m_slotTable.begin();
				for(;it!=m_slotTable.end();it++)
					it->second.MarkAsFree();
		      Simulator::Schedule ((totalTransmissionTimeUs + GetInterFrameTimeInterval ()), &TdmaControllerDynamic::StartTdmaSessions, this);
		   }
		  else
		  {
		      NS_LOG_DEBUG ("Scheduling next session");
		      Simulator::Schedule (totalTransmissionTimeUs, &TdmaControllerDynamic::ScheduleTdmaSession, this, (slotNum+1));
		  }
		  break;
	}
	}
}
Ptr<Packet>
TdmaControllerDynamic::SetSlotPacket()
{
	std::memset(m_data,0,m_dataSize);
	NS_LOG_DEBUG ("mac="<<m_mac->GetAddress());
	 std::map<uint32_t,TdmaSlot>::iterator it;
	 it=m_slotTable.begin();
	 uint8_t * ptr=m_data;
	 uint32_t i=0;
	 TdmaSlot::State state;
	 uint32_t index;
	 Mac48Address owner;
	//ptr=ptr+sizeof(uint32_t);
	for(;it!=m_slotTable.end();it++)
	{
		if(it->second.GetState()==TdmaSlot::ALLOCATED)
		{
			state=it->second.GetState();
			index=it->second.GetSlotIndex();
			owner=it->second.GetOwner();
			std::memcpy(ptr,&(it->first),sizeof(uint32_t));
			ptr=ptr+sizeof(uint32_t);
			std::memcpy(ptr,&index,sizeof(uint32_t));
			ptr=ptr+sizeof(uint32_t);//std::memcpy(ptr,&(it->second.GetSlotIndex()),sizeof(uint32_t));  返回值不可以取地址
			std::memcpy(ptr,&state,sizeof(TdmaSlot::State));
			ptr=ptr+sizeof(TdmaSlot::State);
			std::memcpy(ptr,&owner,sizeof(Mac48Address));
			ptr=ptr+sizeof(Mac48Address);
			i++;
			NS_LOG_DEBUG ("send:index="<<it->first<<" slot state="<<it->second.GetState()<<" slot mac="<<it->second.GetOwner());
		}

	}
	//ptr=m_data;
	//std::memcpy(ptr,&i,sizeof(uint32_t));
	NS_LOG_DEBUG ("send:slot num="<<i);
	uint32_t sendSize=1;
	if(i!=0)
		 sendSize=i*(sizeof(uint32_t)+sizeof(uint32_t)+sizeof(TdmaSlot::State)+sizeof(Mac48Address));
	Ptr<Packet> packet=new Packet(m_data,sendSize);
	return packet;
}
void
TdmaControllerDynamic::ReceiveSlotMsg(Ptr<Packet>packet)
{

	std::memset(m_dataRecv,0,m_dataSizeRecv);

	NS_LOG_DEBUG ("mac="<<m_mac->GetAddress());
	uint32_t readBytes=packet->CopyData(m_dataRecv,m_dataSizeRecv);
	if(readBytes<=1)
		return;
	 std::map<uint32_t,TdmaSlot>::iterator it;
	 uint32_t slotNum=0;
	 TdmaSlot::State state;
	 uint32_t index;
	 Mac48Address owner;
	 uint8_t *ptrRecv=m_dataRecv;
	 uint32_t unitSize=sizeof(uint32_t)+sizeof(uint32_t)+sizeof(TdmaSlot::State)+sizeof(Mac48Address);
	 uint32_t numSlot=readBytes/unitSize;
	 //uint32_t numSlot;
	 //std::memcpy(&numSlot,ptrRecv,sizeof(uint32_t));
	 //ptrRecv=ptrRecv+sizeof(uint32_t);
	 NS_LOG_DEBUG("Receive databuf count="<<numSlot);
	 for(uint32_t i=0;i<numSlot;i++)
	 {
			std::memcpy(&slotNum,ptrRecv,sizeof(uint32_t));
			ptrRecv=ptrRecv+sizeof(uint32_t);
			std::memcpy(&index,ptrRecv,sizeof(uint32_t));
			ptrRecv=ptrRecv+sizeof(uint32_t);
			std::memcpy(&state,ptrRecv,sizeof(TdmaSlot::State));
			ptrRecv=ptrRecv+sizeof(TdmaSlot::State);
			std::memcpy(&owner,ptrRecv,sizeof(Mac48Address));
			ptrRecv=ptrRecv+sizeof(Mac48Address);

			it=m_slotTable.find(slotNum);
			if(it==m_slotTable.end())
			{
				NS_LOG_DEBUG("Receive databuf slotNum="<<slotNum<<" error");
			}
			else
			{
				it->second.SetIndex(index);
				it->second.MarkAsAllocated(owner);
				NS_LOG_DEBUG ("Receive databuf:slotNum="<<index<<" slot state="<<state<<" slot mac="<<owner);
			}


	 }

}
void
TdmaControllerDynamic::CheckSlotTable()
{

	 std::map<uint32_t,TdmaSlot>::iterator it;
	 it=m_slotTable.find(m_mac->GetDevice()->GetNode()->GetId());
	 if(it==m_slotTable.end())
		 NS_LOG_DEBUG ("CheckSlotTable:find error");
	 else if(it->second.GetState()==TdmaSlot::FREE)
	 {
		 it->second.MarkAsAllocated(m_mac->GetAddress());
		 it->second.SetIndex(m_mac->GetDevice()->GetNode()->GetId());
		 NS_LOG_DEBUG ("CheckSlotTable:index="<<it->first<<" slot state="<<it->second.GetState()<<" slot mac="<<it->second.GetOwner());
	 }

}
void
TdmaControllerDynamic::ReserveSlot()
{

	double percent=0;
	uint32_t counter=m_tdmaRouteTable->GetQueueSizeLessNum(m_mac->GetQueueSize());
	uint32_t tableSize=m_tdmaRouteTable->GetRouteTableSize();
	if(tableSize==0)
		percent=0.4;
	else
		percent=(double)counter/tableSize;
	if(percent>0.6)//分配两个时隙
	{
		int slotnum=m_mac->GetDevice()->GetNode()->GetId();
		for(uint32_t i=0;i<2;i++)
		{
		 uint32_t index=0;
		 std::map<uint32_t,TdmaSlot>::iterator it;
		 index=GetFreeSlot((slotnum+i)%m_totalSlotsAllowed);
		 if(index==200)
		 {
			 NS_LOG_DEBUG("slot is over!");
			 return ;
		 }
		 it=m_slotTable.find(index);
		 if(it==m_slotTable.end())
			 NS_LOG_DEBUG ("ReserveSlot:find two error"<<i);
		 else if (it->first==index)
		 {
			 it->second.MarkAsAllocated(m_mac->GetAddress());
			 it->second.SetIndex(m_mac->GetDevice()->GetNode()->GetId());
		 }
		}

	}
	else if(percent>0.3)  //分配一个时隙
	{

		 uint32_t index=0;
		 std::map<uint32_t,TdmaSlot>::iterator it;
		 index=GetFreeSlot(m_mac->GetDevice()->GetNode()->GetId());
		 if(index==200)
		 {
			 NS_LOG_DEBUG("slot is over!");
			 return ;
		 }
		 it=m_slotTable.find(index);
		 if(it==m_slotTable.end())
			 NS_LOG_DEBUG ("ReserveSlot:find one error");
		 else if (it->first==index)
		 {
			 it->second.MarkAsAllocated(m_mac->GetAddress());
			 it->second.SetIndex(m_mac->GetDevice()->GetNode()->GetId());
		 }

	}

}
uint32_t
TdmaControllerDynamic::GetFreeSlot(uint32_t index)
{

	uint32_t ret=100;
	std::map<uint32_t,TdmaSlot>::iterator it;
	it=m_slotTable.find(index);
	if(it==m_slotTable.end())
	{
		NS_LOG_DEBUG("slot num=!"<<index<<"is error");
	}

	 //NS_ASSERT_MSG(it==m_slotTable.end(),"GetFreeSlot: slotnum error");
	for(;it!=m_slotTable.end();it++)
	{
		if(it->second.GetState()==TdmaSlot::FREE)
			return ret=it->first;
	}
	if(it==m_slotTable.end())
	{
		it=m_slotTable.begin();
		for(;it->first!=index;it++)
		{
			if(it->second.GetState()==TdmaSlot::FREE)
				return ret=it->first;
		}
	}
	return ret;
}
Time
TdmaControllerDynamic::CalculateTxTime (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (*packet);
  NS_ASSERT_MSG (packet->GetSize () < 1500,"PacketSize must be less than 1500B, it is: " << packet->GetSize ());
  uint32_t size=packet->GetSize () ;
  return Seconds (m_bps.CalculateTxTime (size));
}
void
TdmaControllerDynamic::RecvRouterMsg(TdmaHeader hdr)
{

	TdmaRouteTableEntry entry;
	entry.SetPriority(hdr.GetSrcId());
	entry.SetAddress(hdr.GetTransmit());
	entry.SetWaitQueueSize(hdr.GetSrcWaitSize());
	entry.SetTime();
	m_tdmaRouteTable->UpdateTable(entry);
}
bool
TdmaControllerDynamic::IsTransmit()
{
	if(m_transmitPara==0||m_tdmaRouteTable->GetRouteTableSize()<=2)
		m_isTransmit=true;
	else
	{

		uint32_t counter=m_tdmaRouteTable->GetQueueSizeLessNum(m_mac->GetQueueSize());
		uint32_t tableSize=m_tdmaRouteTable->GetRouteTableSize();
		double percent=(double)counter/tableSize;
		NS_LOG_DEBUG ("my table size= "<<tableSize<<"  "<<"my queue less than="<<counter<<" percent="<<percent);
		if(percent>m_transmitPara)
			m_isTransmit=true;
		else
			m_isTransmit=false;

	}
	return m_isTransmit;
}
void
TdmaControllerDynamic::PrintSlotTable(std::ostream &os)
{
	std::map<uint32_t,TdmaSlot>::iterator it;
	it=m_slotTable.begin();
	for(;it!=m_slotTable.end();it++)
	{
		os<<"index="<<it->first<<"id(prority)="<<it->second.GetSlotIndex()<<"mac="<<it->second.GetOwner()<<
				"state="<<it->second.GetState();
	}
}
} // namespace ns3
