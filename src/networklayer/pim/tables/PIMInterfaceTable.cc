//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include "InterfaceMatcher.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "PIMInterfaceTable.h"

using namespace std;

Define_Module(PIMInterfaceTable);

// for WATCH_VECTOR
std::ostream& operator<<(std::ostream& os, const PIMInterface* e)
{
    os << "name = " << e->getInterfacePtr()->getName() << "; mode = ";
    if (e->getMode() == PIMInterface::DenseMode)
        os << "Dense";
    else if (e->getMode() == PIMInterface::SparseMode)
        os << "Sparse";
    return os;
};

std::string PIMInterface::info() const
{
    std::stringstream out;
    out << this;
    return out.str();
}

PIMInterfaceTable::~PIMInterfaceTable()
{
    for (std::vector<PIMInterface*>::iterator it = pimInterfaces.begin(); it != pimInterfaces.end(); ++it)
        delete *it;
}

void PIMInterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PIMInterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
		WATCH_VECTOR(pimInterfaces);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2)
    {
        configureInterfaces(par("pimConfig").xmlValue());

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMInterfaceTable: containing node not found.");
        host->subscribe(NF_INTERFACE_CREATED, this);
        host->subscribe(NF_INTERFACE_DELETED, this);
    }
}

void PIMInterfaceTable::configureInterfaces(cXMLElement *config)
{
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);
    IInterfaceTable *ift = InterfaceTableAccess().get(this);

    for (int k = 0; k < ift->getNumInterfaces(); ++k)
    {
        InterfaceEntry *ie = ift->getInterface(k);
        if (ie->isMulticast() && !ie->isLoopback())
        {
            int i = matcher.findMatchingSelector(ie);
            if (i >= 0)
            {
                PIMInterface *pimInterface = createInterface(ie, interfaceElements[i]);
                if (pimInterface)
                    pimInterfaces.push_back(pimInterface);
            }
        }
    }
}

PIMInterface *PIMInterfaceTable::createInterface(InterfaceEntry *ie, cXMLElement *config)
{
    const char *modeAttr = config->getAttribute("mode");
    if (!modeAttr)
        return NULL;

    PIMInterface::PIMMode mode;
    if (strcmp(modeAttr, "dense") == 0)
        mode = PIMInterface::DenseMode;
    else if (strcmp(modeAttr, "sparse") == 0)
        mode = PIMInterface::SparseMode;
    else
        throw cRuntimeError("PIMInterfaceTable: invalid 'mode' attribute value in the configuration of interface '%s'", ie->getName());

    const char *stateRefreshAttr = config->getAttribute("state-refresh");
    bool stateRefreshFlag = stateRefreshAttr && strcmp(stateRefreshAttr, "true");

    return new PIMInterface(ie, mode, stateRefreshFlag);
}

PIMInterface *PIMInterfaceTable::getInterfaceById(int interfaceId)
{
	for(int i = 0; i < getNumInterfaces(); i++)
		if(interfaceId == getInterface(i)->getInterfaceId())
			return getInterface(i);
	return NULL;
}

void PIMInterfaceTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, details);

    if (signalID == NF_INTERFACE_CREATED)
    {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(details);
        if (ie->isMulticast() && !ie->isLoopback())
            addInterface(ie);
    }
    else if (signalID == NF_INTERFACE_DELETED)
    {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(details);
        if (ie->isMulticast() && !ie->isLoopback())
            removeInterface(ie);
    }
}

PIMInterfaceTable::PIMInterfaceVector::iterator PIMInterfaceTable::findInterface(InterfaceEntry *ie)
{
    for (PIMInterfaceVector::iterator it = pimInterfaces.begin(); it != pimInterfaces.end(); ++it)
        if ((*it)->getInterfacePtr() == ie)
            return it;
    return pimInterfaces.end();
}

void PIMInterfaceTable::addInterface(InterfaceEntry *ie)
{
    ASSERT(findInterface(ie) == pimInterfaces.end());

    cXMLElement * config = par("pimConfig").xmlValue();
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);

    int i = matcher.findMatchingSelector(ie);
    if (i >= 0)
    {
        PIMInterface *pimInterface = createInterface(ie, interfaceElements[i]);
        if (pimInterface)
            pimInterfaces.push_back(pimInterface);
    }
}

void PIMInterfaceTable::removeInterface(InterfaceEntry *ie)
{
    PIMInterfaceVector::iterator it = findInterface(ie);
    if (it != pimInterfaces.end())
        pimInterfaces.erase(it);
}