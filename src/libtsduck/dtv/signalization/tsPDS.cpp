//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2024, Thierry Lelegard
// BSD-2-Clause license, see LICENSE.txt file or https://tsduck.io/license
//
//----------------------------------------------------------------------------

#include "tsPDS.h"


TS_DEFINE_GLOBAL(const, ts::Enumeration, ts::PrivateDataSpecifierEnum, ({
    {u"BskyB",     ts::PDS_BSKYB},
    {u"Nagra",     ts::PDS_NAGRA},
    {u"TPS",       ts::PDS_TPS},
    {u"EACEM",     ts::PDS_EACEM},
    {u"EICTA",     ts::PDS_EICTA},  // same value as EACEM
    {u"NorDig",    ts::PDS_NORDIG},
    {u"Logiways",  ts::PDS_LOGIWAYS},
    {u"CanalPlus", ts::PDS_CANALPLUS},
    {u"Eutelsat",  ts::PDS_EUTELSAT},
    {u"OFCOM",     ts::PDS_OFCOM},
    {u"Australia", ts::PDS_AUSTRALIA},
    {u"AVSV",      ts::PDS_AVSVideo},
    {u"AVSA",      ts::PDS_AVSAudio},
    {u"AOM",       ts::PDS_AOM},
    {u"cuvv",      ts::PDS_CUVV},
}));
