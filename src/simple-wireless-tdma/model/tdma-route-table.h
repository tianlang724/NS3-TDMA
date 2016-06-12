/*
 * tdma-route-table.h

 *
 *  Created on: May 24, 2016
 *      Author: zh
 */
#ifndef TDMA_ROUTE_TABLE_H
#define TDMA_ROUTE_TABLE_H
#include <map>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "tdma-slot.h"
#include "tdma-controller-dynamic.h"
namespace ns3
{
class TdmaControllerDynamic;
class TdmaRouteTableEntry:public Object
{
public:
	TdmaRouteTableEntry();
	~TdmaRouteTableEntry();
	void SetPriority(uint32_t pri);
	uint32_t GetPriority();
	void SetWaitQueueSize(uint32_t size);
	uint32_t GetWaitQueueSize();
	void SetAddress(Mac48Address address);
	Mac48Address GetAddress();
	void SetTime();
	Time GetTime();
	bool IsTimeout(int64_t timeout);
	void Print (std::ostream &os) const;
protected:
  virtual void DoDispose (void);
private:
	uint32_t m_priority;
	uint32_t m_waitQueueSize;
	Mac48Address m_address;
	Time m_time;
};
class TdmaRouteTable:public Object
{
public:
	TdmaRouteTable();
	~TdmaRouteTable();
	void SetTimeout(int64_t time);
	//void SetTdmaControllerDynamic(Ptr<TdmaControllerDynamic> controllerDynamic);
	void Print (std::ostream &os);
	//收到新的信息包之后进行处理
	void UpdateTable(TdmaRouteTableEntry &entry);
	//添加新表项
	void AddNewEntry(TdmaRouteTableEntry &entry);
	//更新旧表项
	bool  UpdateTableEntry(TdmaRouteTableEntry &entry);
	//删除超时表项
	void DeleteTimeoutEntry();
	uint32_t GetQueueSizeLessNum(uint32_t size);
	uint32_t GetRouteTableSize();
protected:
  virtual void DoDispose (void);
private:
	int64_t m_timeout;
	std::map<uint32_t,TdmaRouteTableEntry> m_routeTable;
	//Ptr<TdmaControllerDynamic> m_tdmaControllerDynamic;
};
}
#endif
