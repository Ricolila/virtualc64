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

namespace vc64 {

class PRGFile : public AnyCollection {

public:
    
    //
    // Class methods
    //
    
    static bool isCompatible(const string &name);
    static bool isCompatible(std::istream &stream);
    
    
    //
    // Initializing
    //
    
    PRGFile() : AnyCollection() { }
    PRGFile(isize capacity) : AnyCollection(capacity) { }
    PRGFile(const string &path) throws { init(path); }
    PRGFile(const u8 *buf, isize len) throws { init(buf, len); }
    PRGFile(class FileSystem &fs) throws { init(fs); }
    
private:
    
    using AnyFile::init;
    void init(FileSystem &fs) throws;
    
    
    //
    // Methods from CoreObject
    //
    
    const char *getDescription() const override { return "PRGFile"; }
    
    
    //
    // Methods from AnyFile
    //

    bool isCompatiblePath(const string &path) override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) override { return isCompatible(stream); }
    FileType type() const override { return FILETYPE_PRG; }
    
    
    //
    // Methods from AnyCollection
    //

    PETName<16> collectionName() override;
    isize collectionCount() const override;
    PETName<16> itemName(isize nr) const override;
    isize itemSize(isize nr) const override;
    u8 readByte(isize nr, isize pos) const override;
};

}
