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
//!
//!  @file
//!  Representation of a DTG preferred_name_list_descriptor.
//!  This is a private descriptor, must be preceded by the DTG/OFCOM PDS.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractPreferredNameListDescriptor.h"

namespace ts {
    //!
    //! Representation of a DTG preferred_name_list_descriptor.
    //!
    //! This is a private descriptor, must be preceded by the DTG/OFCOM PDS.
    //! @see The D-Book 7 Part A (DTG), section 8.5.3.7
    //! @ingroup descriptor
    //!
    class TSDUCKDLL DTGPreferredNameListDescriptor : public AbstractPreferredNameListDescriptor
    {
    public:
        //!
        //! Default constructor.
        //!
        DTGPreferredNameListDescriptor();

        //!
        //! Constructor from a binary descriptor
        //! @param [in,out] duck TSDuck execution context.
        //! @param [in] bin A binary descriptor to deserialize.
        //!
        DTGPreferredNameListDescriptor(DuckContext& duck, const Descriptor& bin);

        //!
        //! Destructor.
        //!
        virtual ~DTGPreferredNameListDescriptor() override;
    };
}
