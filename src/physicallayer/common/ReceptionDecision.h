//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_RECEPTIONDECISION_H
#define __INET_RECEPTIONDECISION_H

#include "IReceptionDecision.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionDecision : public IReceptionDecision, public cObject
{
  protected:
    const IReception *reception;
    const ReceptionIndication *indication;
    const bool isPossible_;
    const bool isAttempted_;
    const bool isSuccessful_;

  public:
    ReceptionDecision(const IReception *reception, const ReceptionIndication *indication, bool isPossible, bool isAttempted, bool isSuccessful) :
        reception(reception),
        indication(indication),
        isPossible_(isPossible),
        isAttempted_(isAttempted),
        isSuccessful_(isSuccessful)
    {}

    virtual void printToStream(std::ostream& stream) const;

    virtual const IReception *getReception() const { return reception; }

    virtual const ReceptionIndication *getIndication() const { return indication; }

    virtual bool isPossible() const { return isPossible_; }

    virtual bool isAttempted() const { return isAttempted_; }

    virtual bool isSuccessful() const { return isSuccessful_; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONDECISION_H

