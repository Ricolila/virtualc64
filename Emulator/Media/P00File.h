// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyCollection.h"

class P00File : public AnyCollection {

public:

    static bool isCompatible(const string &name);
    static bool isCompatible(std::istream &stream);
    
    
    //
    // Initializing
    //
    
    P00File() : AnyCollection() { }
    P00File(isize capacity) : AnyCollection(capacity) { }
    P00File(const string &path) throws { init(path); }
    P00File(const u8 *buf, isize len) throws { init(buf, len); }
    P00File(class FileSystem &fs) throws { init(fs); }
    
private:
    
    using AnyFile::init;
    void init(FileSystem &fs) throws;
    
    
    //
    // Methods from C64Object
    //

    const char *getDescription() const override { return "P00File"; }


    //
    // Methods from AnyFile
    //
    
    bool isCompatiblePath(const string &path) override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) override { return isCompatible(stream); }
    FileType type() const override { return FILETYPE_P00; }
    PETName<16> getName() const override;


    //
    // Methods from AnyCollection
    //

    PETName<16> collectionName() override;
    isize collectionCount() const override;
    PETName<16> itemName(isize nr) const override;
    u64 itemSize(isize nr) const override;
    u8 readByte(isize nr, u64 pos) const override;
};
