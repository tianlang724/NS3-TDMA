/*
 * tdma-packet-tag.cc
 *
 *  Created on: Apr 25, 2016
 *      Author: zh
 */

#include "tdma-packet-tag.h"

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (TdmaTag);
TdmaTag::TdmaTag()
{
}
TdmaTag:: ~TdmaTag()
{
}
TypeId
TdmaTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TdmaTag")
    .SetParent<Tag> ()
    .AddConstructor<TdmaTag> ();
  /*
    .AddAttribute ("m_timestamp",
                   "time stamp",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&TdmaTag::GetSTimeStamp),
                   TimeChecker<time> ());*/

  return tid;
}
TypeId
TdmaTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
TdmaTag::GetSerializedSize (void) const
{
  return 8;
}
void
TdmaTag::Serialize (TagBuffer i) const
{
	uint64_t time=m_timestamp.GetNanoSeconds();
	i.WriteU64 (time);
}
void
TdmaTag::Deserialize (TagBuffer i)
{
	uint64_t time = i.ReadU64();
	m_timestamp=Time(time);
}
void
TdmaTag::Print (std::ostream &os) const
{
  os << "time=" <<m_timestamp;
}
void
TdmaTag::SetTimeStamp (Time value)
{
	m_timestamp = value;
}
Time
TdmaTag::GetSTimeStamp (void) const
{
  return m_timestamp;
}

}



