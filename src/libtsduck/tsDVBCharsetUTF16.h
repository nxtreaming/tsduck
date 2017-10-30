//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2017, Thierry Lelegard
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
//!  Declaration of class DVBCharsetUTF16.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsDVBCharset.h"

namespace ts {
    //!
    //! Definition of the UTF-8 DVB character set.
    //! @see ETSI EN 300 468, Annex A
    //!
    class TSDUCKDLL DVBCharsetUTF16 : public DVBCharset
    {
    public:
        //!
        //! UNICODE character set singleton
        //!
        static const DVBCharsetUTF16 UNICODE;

        // Inherited methods.
        virtual bool decode(UString& str, const uint8_t* dvb, size_t dvbSize) const override;
        virtual bool canEncode(const UString& str, size_t start = 0, size_t count = UString::NPOS) const override;
        virtual size_t encode(uint8_t*& buffer, size_t& size, const UString& str, size_t start = 0, size_t count = UString::NPOS) const override;

    protected:
        //!
        //! Constructor.
        //! There is only one definition for such DVB character sets.
        //!
        DVBCharsetUTF16() : DVBCharset("UNICODE", 0x000011) {}

    private:
        // Unaccessible operations.
        DVBCharsetUTF16(const DVBCharsetUTF16&) = delete;
        DVBCharsetUTF16& operator=(const DVBCharsetUTF16&) = delete;
    };
}
