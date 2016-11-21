/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "zmqCommunicator.h"
#include "zmqHelper.h"
#include "zmqContextManager.h"
#include "core/helperTemplates.h"
#include "zmqProxyHub.h"
#include "zmqReactor.h"
#include <cppzmq/zmq_addon.hpp>

#include "comms/commMessage.h"

using namespace zmq;

zmqCommunicator::zmqCommunicator():gridCommunicator()
{

}

zmqCommunicator::zmqCommunicator(std::string name):gridCommunicator(name),txDescriptor(name+"_tx"),rxDescriptor(name+"_rx")
{

}

zmqCommunicator::zmqCommunicator(std::string name, std::uint64_t id):gridCommunicator(name,id), txDescriptor(name + "_tx"), rxDescriptor(name + "_rx")
{

}

zmqCommunicator::zmqCommunicator(std::uint64_t id):gridCommunicator(id)
{

}

zmqCommunicator::~zmqCommunicator()
{

}


std::shared_ptr<gridCommunicator> zmqCommunicator::clone(std::shared_ptr<gridCommunicator> comm) const
{
	auto zmqComm = cloneBase<zmqCommunicator, gridCommunicator>(this, comm);
	if (!zmqComm)
	{
		return comm;
	}
	zmqComm->txDescriptor = txDescriptor;
	zmqComm->rxDescriptor=rxDescriptor;
	zmqComm->proxyName=proxyName;
	zmqComm->contextName = contextName;
	zmqComm->flags = flags;

	return zmqComm;
}

void zmqCommunicator::transmit(const std::string &destName, std::shared_ptr<commMessage> message)
{
	zmq::multipart_t txmsg;
	if (!flags[no_transmit_dest])
	{
		txmsg.addstr(destName);
	}
	addHeader(txmsg, message);
	addMessageBody(txmsg, message);
	txmsg.send(*txSocket);

}

void zmqCommunicator::transmit(std::uint64_t destID, std::shared_ptr<commMessage> message)
{
	zmq::multipart_t txmsg;
	if (!flags[no_transmit_dest])
	{
		txmsg.addmem(&destID, 8);
	}
	addHeader(txmsg, message);
	addMessageBody(txmsg, message);
	txmsg.send(*txSocket);
}

void zmqCommunicator::addHeader(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message)
{
	if (!flags[no_transmit_source])
	{
		msg.addstr(getName());
	}
}

void zmqCommunicator::addMessageBody(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message)
{
	msg.addstr(message->toString(commMessage::comm_modifiers::with_type));
}

void zmqCommunicator::initialize()
{
	//don't initialize twice if we already initialized
	if (txSocket)
	{
		return;
	}
	
	//set up transmission sockets and information

	if (flags[use_tx_proxy])
	{
		auto localProxy = zmqProxyHub::getProxy(proxyName);
		if (!localProxy->isRunning())
		{
			localProxy->startProxy();
		}
		txDescriptor.addOperation(socket_ops::connect, localProxy->getIncomingConnection());
	}

	if (flags[use_rx_proxy])
	{
		auto localProxy = zmqProxyHub::getProxy(proxyName);
		if (!localProxy->isRunning())
		{
			localProxy->startProxy();
		}
		rxDescriptor.addOperation(socket_ops::connect, localProxy->getIncomingConnection());
	}
	
	txDescriptor.addOperation(socket_ops::subscribe, getName());
	rxDescriptor.addOperation(socket_ops::subscribe, getName());

	auto id = getID();
	txDescriptor.addOperation(socket_ops::subscribe, std::string(reinterpret_cast<char *>(&id), sizeof(id)));  //I know this is ugly
	rxDescriptor.addOperation(socket_ops::subscribe, std::string(reinterpret_cast<char *>(&id),sizeof(id)));  //I know this is ugly
	decltype(id) broadcastId = 0;
	txDescriptor.addOperation(socket_ops::subscribe, std::string(reinterpret_cast<char *>(&broadcastId), sizeof(broadcastId)));  //I know this is ugly
	rxDescriptor.addOperation(socket_ops::subscribe, std::string(reinterpret_cast<char *>(&broadcastId), sizeof(broadcastId)));  //I know this is ugly
	
	rxDescriptor.callback = [this](const multipart_t &msg) {messageHandler(msg); };
//set up the rx socket reactor
	if (!flags[transmit_only])
	{
		zmqReactor::getReactorInstance("", contextName)->addSocket(rxDescriptor);
	}
	
	txSocket = txDescriptor.makeSocketPtr(zmqContextManager::getContext(contextName));

}

void zmqCommunicator::disconnect()
{
	if (!flags[transmit_only])
	{
		zmqReactor::getReactorInstance("")->closeSocket(getName() + "_rx");
	}
	txSocket = nullptr;
}

void zmqCommunicator::set(const std::string &param, const std::string &val)
{
	if (param == "txconnection")
	{
		txDescriptor.addOperation(socket_ops::connect,val);
	}
	else if (param == "rxconnection")
	{
		rxDescriptor.addOperation(socket_ops::connect, val);
	}
	else if (param == "rxsubscription")
	{
		rxDescriptor.addOperation(socket_ops::subscribe, val);
	}
	else if (param == "txsubscription")
	{
		txDescriptor.addOperation(socket_ops::subscribe, val);
	}
	else if ((param == "proxy") || (param == "proxyname"))
	{
		proxyName = val;
		setFlag("useproxy", true);
	}
	else if ((param == "txtype") || (param == "sockettype"))
	{
		txDescriptor.type = socketTypeFromString(val);
	}
	else if (param == "rxtype")
	{
		rxDescriptor.type = socketTypeFromString(val);
	}
	else
	{
		gridCommunicator::set(param, val);
	}
}

void zmqCommunicator::set(const std::string &param, double val)
{
	gridCommunicator::set(param, val);
}

void zmqCommunicator::setFlag(const std::string &flag, bool val)
{
	if ((flag == "txonly") || (flag == "transmitonly")||(flag=="transmit_only"))
	{
		flags.set(transmit_only, val);
	}
	else if (flag == "transmitsource")
	{
		flags.set(no_transmit_source, !val);
	}
	else if (flag == "notransmitsource")
	{
		flags.set(no_transmit_source, val);
	}
	else if (flag == "transmitdest")
	{
		flags.set(no_transmit_dest, !val);
	}
	else if (flag == "notransmitdest")
	{
		flags.set(no_transmit_dest, val);
	}
	else if (flag == "useproxy")
	{
		flags.set(use_rx_proxy,val);
		flags.set(use_tx_proxy,val);
	}
	else if (flag == "usetxproxy")
	{
		flags.set(use_tx_proxy, val);
	}
	else if (flag == "userxproxy")
	{
		flags.set(use_rx_proxy, val);
	}
	else
	{
		gridCommunicator::setFlag(flag, val);
	}

}

void zmqCommunicator::messageHandler(const multipart_t &msg)
{
	auto sz = msg.size();
	//size should be either 2 or 3
	auto msgBody = (sz == 2) ? msg.peek(1) : msg.peek(2);

	std::string msgString((const char *)(msgBody->data()), msgBody->size());
	auto type=commMessage::extractMessageType(msgString);
	auto gdMsg = coreMessageFactory::instance()->createMessage(type);
	gdMsg->loadString(msgString);

	//call the lower level receive function
	receive(0, getName(), gdMsg);
}