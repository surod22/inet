//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <platdep/sockets.h>

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "SocketsRTScheduler.h"
#include "TCPSocket.h"

class TCPExtActiveThread : public TCPSocket::CallbackInterface
{
  protected:
    int extSocketId;
    cModule *appModule;
    SocketsRTScheduler *rtScheduler;
    TCPSocket inetSocket;
  public:
    TCPExtActiveThread();
    virtual ~TCPExtActiveThread();
    virtual void handleMessage(cMessage *msg);
    virtual void openActiveInetSocket(const char *, int port);      //TODO
    virtual void acceptExtConnection(cModule *module, int extConnSocketID);
    virtual void acceptInetConnection(cModule *module, cMessage *msg);

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) { delete status; }
};

class TCPExtListenerThread
{
  protected:
    int extListenerSocketId;
    cModule *appModule;
    SocketsRTScheduler *rtScheduler;
  public:
    TCPExtListenerThread();
    virtual ~TCPExtListenerThread();
    virtual void init(cModule *appModule); // called in INITSTAGE_APPLICATION_LAYER
    virtual void handleMessage(cMessage *msg);
    virtual void acceptExtConnection(cMessage *msg);
};

TCPExtListenerThread::TCPExtListenerThread()
{
    extListenerSocketId = INVALID_SOCKET;
    appModule = NULL;
    rtScheduler = NULL;
}

TCPExtListenerThread::~TCPExtListenerThread()
{
}

void TCPExtListenerThread::init(cModule *appModulePar) // called in INITSTAGE_APPLICATION_LAYER
{
    appModule = appModulePar;
    rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());

    extListenerSocketId = socket(AF_INET, SOCK_STREAM, 0);
    if (extListenerSocketId == INVALID_SOCKET)
        throw cRuntimeError("cannot create socket");

    sockaddr_in sinInterface;
    sinInterface.sin_family = AF_INET;
    sinInterface.sin_addr.s_addr = INADDR_ANY;
    sinInterface.sin_port = htons(uint16_t(appModule->par("tunnelPort")));
    if (bind(extListenerSocketId, (sockaddr*) &sinInterface, sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw cRuntimeError("socket bind() failed");

    listen(extListenerSocketId, SOMAXCONN);

    rtScheduler->addSocket(appModule, this, extListenerSocketId, true);
}

void TCPExtListenerThread::handleMessage(cMessage *msg)
{
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
            throw cRuntimeError("DATA not accepted");
            delete msg;
            break;
        case SocketsRTScheduler::CLOSED:
            if (extListenerSocketId != msg->par("fd").longValue())
                throw cRuntimeError("unknown socket id");
            close(extListenerSocketId);
            extListenerSocketId = INVALID_SOCKET;
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            acceptExtConnection(msg);
            delete msg;
            break;
    }
}

void TCPExtListenerThread::acceptExtConnection(cMessage *msg)
{
    TCPExtActiveThread *thread = new TCPExtActiveThread();
    int extConnSocket = msg->par("fd").longValue();
    thread->acceptExtConnection(appModule, extConnSocket);
}

void TCPExtActiveThread::acceptExtConnection(cModule *module, int extConnSocketID)
{
    appModule = module;
    extSocketId = extConnSocketID;
    openActiveInetSocket(appModule->par("connectAddress").stringValue(), appModule->par("connectPort").longValue());
    rtScheduler->addSocket(appModule, this, extSocketId, false);
}

void TCPExtActiveThread::acceptInetConnection(cModule *module, cMessage *msg)
{
    appModule = module;
}

void TCPExtActiveThread::handleMessage(cMessage *msg)
{
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
        {
            if (extSocketId != msg->par("fd").longValue())
                throw cRuntimeError("socket not opened");
            ByteArrayMessage *pk = check_and_cast<ByteArrayMessage *>(msg);
            inetSocket.send(pk);
            break;
        }
        case SocketsRTScheduler::CLOSED:
            if (extSocketId != msg->par("fd").longValue())
                throw cRuntimeError("unknown socket id");
            closesocket(extSocketId);
            extSocketId = INVALID_SOCKET;
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            throw cRuntimeError("socket already opened");
            delete msg;
            break;
    }
}
