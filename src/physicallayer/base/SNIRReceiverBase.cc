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

#include "SNIRReceiverBase.h"
#include "SynchronizationDecision.h"
#include "ReceptionDecision.h"

namespace inet {

namespace physicallayer {

void SNIRReceiverBase::initialize(int stage)
{
    ReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        snirThreshold = FWMath::dB2fraction(par("snirThreshold"));
}

bool SNIRReceiverBase::areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const
{
    return carrierFrequency1 + bandwidth1 / 2 >= carrierFrequency2 - bandwidth2 / 2 &&
           carrierFrequency1 - bandwidth1 / 2 <= carrierFrequency2 + bandwidth2 / 2;
}

const SynchronizationIndication *SNIRReceiverBase::computeSynchronizationIndication(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    // TODO: compute minSNIR for synchronization duration
    const INoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    double minSNIR = computeMinSNIR(reception, noise);
    delete noise;
    SynchronizationIndication *indication = new SynchronizationIndication();
    indication->setMinSNIR(minSNIR);
    return indication;
}

bool SNIRReceiverBase::computeIsSynchronizationSuccessful(const IListening *listening, const IReception *reception, const SynchronizationIndication *indication) const
{
    return indication->getMinSNIR() > snirThreshold;
}

const ISynchronizationDecision *SNIRReceiverBase::computeSynchronizationDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    bool isSynchronizationPossible = computeIsSynchronizationPossible(listening, reception);
    bool isSynchronizationAttempted = isSynchronizationPossible && computeIsSynchronizationAttempted(listening, reception, interferingReceptions);
    const SynchronizationIndication *indication = isSynchronizationAttempted ? computeSynchronizationIndication(listening, reception, interferingReceptions, backgroundNoise) : NULL;
    bool isSynchronizationSuccessful = isSynchronizationAttempted && computeIsSynchronizationSuccessful(listening, reception, indication);
    return new SynchronizationDecision(reception, indication, isSynchronizationPossible, isSynchronizationAttempted, isSynchronizationSuccessful);
}

const ReceptionIndication *SNIRReceiverBase::computeReceptionIndication(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    const INoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
    double minSNIR = computeMinSNIR(reception, noise);
    delete noise;
    ReceptionIndication *indication = new ReceptionIndication();
    indication->setMinSNIR(minSNIR);
    return indication;
}

bool SNIRReceiverBase::computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, const ReceptionIndication *indication) const
{
    return indication->getMinSNIR() > snirThreshold;
}

const IReceptionDecision *SNIRReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception);
    bool isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, interferingReceptions);
    const ReceptionIndication *indication = isReceptionAttempted ? computeReceptionIndication(listening, reception, interferingReceptions, backgroundNoise) : NULL;
    bool isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(listening, reception, indication);
    return new ReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

} // namespace physicallayer

} // namespace inet

