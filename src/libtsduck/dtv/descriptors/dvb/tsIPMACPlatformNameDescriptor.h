//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2024, Thierry Lelegard
// BSD-2-Clause license, see LICENSE.txt file or https://tsduck.io/license
//
//----------------------------------------------------------------------------
//!
//!  @file
//!  Representation of an IP/MAC_platform_name_descriptor (INT specific).
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsAbstractDescriptor.h"

namespace ts {
    //!
    //! Representation of an IP/MAC_platform_name_descriptor (INT specific).
    //!
    //! This descriptor cannot be present in other tables than an INT
    //! because its tag reuses an MPEG-defined one.
    //!
    //! @see ETSI EN 301 192, 8.4.5.2.
    //! @ingroup descriptor
    //!
    class TSDUCKDLL IPMACPlatformNameDescriptor : public AbstractDescriptor
    {
    public:
        // IPMACPlatformNameDescriptor public members:
        UString language_code {};  //!< ISO-639 language code, 3 chars.
        UString text {};           //!< Platform name.

        //!
        //! Default constructor.
        //! @param [in] lang ISO-639 language code, 3 chars.
        //! @param [in] name Platform name.
        //!
        IPMACPlatformNameDescriptor(const UString& lang = UString(), const UString& name = UString());

        //!
        //! Constructor from a binary descriptor.
        //! @param [in,out] duck TSDuck execution context.
        //! @param [in] bin A binary descriptor to deserialize.
        //!
        IPMACPlatformNameDescriptor(DuckContext& duck, const Descriptor& bin);

        // Inherited methods
        DeclareDisplayDescriptor();

    protected:
        // Inherited methods
        virtual void clearContent() override;
        virtual void serializePayload(PSIBuffer&) const override;
        virtual void deserializePayload(PSIBuffer&) override;
        virtual void buildXML(DuckContext&, xml::Element*) const override;
        virtual bool analyzeXML(DuckContext&, const xml::Element*) override;
    };
}