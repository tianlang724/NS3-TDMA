/*
 * tdma-route-table.cc
 *
 *  Created on: May 24, 2016
 *      Author: zh
 */

#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/stats-module.h"
#include "tdma-slot.h"
#include "tdma-route-table.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TdmaRouteTable");
/*************************************
 * TdmaRouteTableEntry
 ************************************/

TdmaRouteTableEntry::TdmaRouteTableEntry()
:m_priority(0),
 m_waitQueueSize(0),
 m_time(0)
{

}
TdmaRouteTableEntry::~TdmaRouteTableEntry()
{
}
void
TdmaRouteTableEntry::DoDispose (void)
{

}
void
TdmaRouteTableEntry::SetPriority(uint32_t pri)
{
	m_priority=pri;
}
uint32_t
TdmaRouteTableEntry::GetPriority()
{
	return m_priority;
}
void
TdmaRouteTableEntry::SetWaitQueueSize(uint32_t size)
{
	m_waitQueueSize=size;
}
uint32_t
TdmaRouteTableEntry::GetWaitQueueSize()
{
	return m_waitQueueSize;
}
void
TdmaRouteTableEntry::SetAddress(Mac48Address address)
{
	m_address=address;
}
Mac48Address
TdmaRouteTableEntry::GetAddress()
{
	return m_address;
}
void
TdmaRouteTableEntry::SetTime()
{
	m_time=Simulator::Now();
}
Time
TdmaRouteTableEntry::GetTime()
{
	return m_time;
}
bool
TdmaRouteTableEntry::IsTimeout(int64_t timeout)
{
	Time time=Simulator::Now()-GetTime();
	return (time.GetMicroSeconds()>timeout);
}
void
TdmaRouteTableEntry::Print(std::ostream &os) const
{
	os<<"Prrority"<<m_priority<<" "<< "WaitQueue"<<m_waitQueueSize<<" "
			<<"Address"<<m_address;
}

/****************
void
TdmaRouteTableEntry::SetSlotEntry(std::map<uint32_t,Ptr<TdmaSlot>> &slotEntry)
{
	std::map<uint32_t,Ptr<TdmaSlot>>::iterator iter;
	iter=slotEntry.begin();
	for(;iter++;iter!=slotEntry.end())
	{
		m_slotEntry.insert(std::pair<uint32_t,Ptr<TdmaSlot>>(iter->first,iter->second));
	}
}
***************/


/*************************************
 * TdmaRouteTable
 ************************************/
TdmaRouteTable::TdmaRouteTable()
:m_timeout(20000)
{

}

TdmaRouteTable::~TdmaRouteTable()
{

}
void
TdmaRouteTable::DoDispose (void)
{

}
void
TdmaRouteTable::SetTimeout(int64_t timeout)
{
	m_timeout=timeout;
}

/*
void
TdmaRouteTable::SetTdmaControllerDynamic(Ptr<TdmaControllerDynamic> controllerDynamic)
{
	m_tdmaControllerDynamic=controllerDynamic;
}*/

void
TdmaRouteTable::UpdateTable(TdmaRouteTableEntry &entry)
{
	if(!UpdateTableEntry(entry))
		AddNewEntry(entry);
	DeleteTimeoutEntry();
}
void
TdmaRouteTable::AddNewEntry(TdmaRouteTableEntry &entry)
{
	m_routeTable.insert(std::pair<uint32_t,TdmaRouteTableEntry>(entry.GetPriority(),entry));
}
bool
TdmaRouteTable::UpdateTableEntry(TdmaRouteTableEntry &entry)
{
	std::map<uint32_t,TdmaRouteTableEntry>::iterator iter;
	iter=m_routeTable.find(entry.GetPriority());
	if(iter!=m_routeTable.end())
	{
		iter->second.SetWaitQueueSize(entry.GetWaitQueueSize());
		iter->second.SetTime();
		return true;
	}
	else
		return false;
}
void
TdmaRouteTable::DeleteTimeoutEntry()
{
	std::map<uint32_t,TdmaRouteTableEntry>::iterator iter;
	iter=m_routeTable.begin();
	for(;iter!=m_routeTable.end();iter++)
	{
		if(iter->second.IsTimeout(m_timeout))
		{
			m_routeTable.erase(iter);
		}
	}
}
void
TdmaRouteTable::Print(std::ostream &os)
{
	std::map<uint32_t,TdmaRouteTableEntry>::const_iterator iter;
	iter=m_routeTable.begin();
	for(;iter!=m_routeTable.end();iter++)
	{
	 //os<<"Index"<<iter->second.GetPriority() <<" "
		//<<"Address"<<iter->second.GetAddress()<< " "<<"WaitQueue"<<iter->second.GetWaitQueueSize();
	}

}
uint32_t
TdmaRouteTable::GetQueueSizeLessNum(uint32_t size)
{
	if(m_routeTable.empty())
		return 0;
	uint32_t counter=0;
	std::map<uint32_t,TdmaRouteTableEntry > ::iterator it;
	it=m_routeTable.begin();
	uint32_t entryQueueSize=0;
	for(;it!=m_routeTable.end();it++)
	{
		entryQueueSize=it->second.GetWaitQueueSize();
		if(size<=entryQueueSize)
			counter++;
		NS_LOG_DEBUG ("my queue size= "<<size<<"  "<<" route size="<<entryQueueSize);
	}
	NS_LOG_DEBUG ("my queue size= "<<size<<"  "<<" queue less than="<<counter<<" table size="<<m_routeTable.size() );
	return counter;

}
uint32_t
TdmaRouteTable::GetRouteTableSize()
{
	return m_routeTable.size();
}
