//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2023, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#include "tsIPPacketPlugin.h"
#include "tsPluginRepository.h"

TS_REGISTER_PROCESSOR_PLUGIN(u"ip", ts::IPPacketPlugin);


//----------------------------------------------------------------------------
// Output constructor
//----------------------------------------------------------------------------

ts::IPPacketPlugin::IPPacketPlugin(TSP* tsp_) :
    ProcessorPlugin(tsp_, u"Send TS packets using UDP/IP, multicast or unicast, and pass them to next plugin", u"[options] address:port"),
    _datagram(TSDatagramOutputOptions::ALLOW_RTP | TSDatagramOutputOptions::ALWAYS_BURST)
{
    _datagram.defineArgs(*this);
}


//----------------------------------------------------------------------------
// Redirect all methods to _datagram.
//----------------------------------------------------------------------------

bool ts::IPPacketPlugin::isRealTime()
{
    return true;
}

bool ts::IPPacketPlugin::getOptions()
{
    return _datagram.loadArgs(duck, *this);
}

bool ts::IPPacketPlugin::start()
{
    return _datagram.open(*tsp);
}

bool ts::IPPacketPlugin::stop()
{
    return _datagram.close(tsp->bitrate(), *tsp);
}

ts::ProcessorPlugin::Status ts::IPPacketPlugin::processPacket(TSPacket& pkt, TSPacketMetadata& pkt_data)
{
    return _datagram.send(&pkt, 1, tsp->bitrate(), *tsp) ? TSP_OK : TSP_END;
}