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

#ifndef __INET_IEEE80211DIMENSIONALTRANSMITTER_H
#define __INET_IEEE80211DIMENSIONALTRANSMITTER_H

#include "inet/physicallayer/apsk/APSKDimensionalTransmitter.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/Ieee80211PhyMode.h"

namespace inet {

namespace physicallayer {

// TODO: remove APSK dependency, factor out common part
class INET_API Ieee80211DimensionalTransmitter : public APSKDimensionalTransmitter
{
  protected:
    const Ieee80211ModeSet *modeSet;
    const IIeee80211Mode *mode;

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211DimensionalTransmitter();

    virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211DIMENSIONALTRANSMITTER_H

