/*
 * tdma-slot.cc
 *
 *  Created on: May 24, 2016
 *      Author: zh
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/stats-module.h"
#include "tdma-slot.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TdmaSlot");
TdmaSlot::TdmaSlot()
:m_state(TdmaSlot::FREE),
 m_index(0)
{

}

TdmaSlot::~TdmaSlot()
{

}
void
TdmaSlot::DoDispose (void)
{

}
TdmaSlot::State
TdmaSlot::GetState()
{
	return m_state;
}

void
TdmaSlot::MarkAsFree()
{
	m_state=TdmaSlot::FREE;
	m_index=0;
	NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext()
	<< " StdmaSlot:MarkAsFree() marked slot " << m_index << " as free )");
}
bool
TdmaSlot::IsFree()
{
	return(m_state==TdmaSlot::FREE);
}
void
TdmaSlot::MarkAsAllocated(Mac48Address owner)
{
	m_state=TdmaSlot::ALLOCATED;
	m_owner=owner;
	NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext()
	<< " StdmaSlot:MarkAsAllocated() marked slot " << m_index << " as allocated )");
}
void
TdmaSlot::SetIndex(uint32_t index)
{
	m_index=index;
}
bool
TdmaSlot::IsAllocated()
{
	return (m_state==TdmaSlot::ALLOCATED);
}
uint32_t
TdmaSlot::GetSlotIndex()
{
	return m_index;
}
Mac48Address
TdmaSlot::GetOwner()
{
	return m_owner;
}
