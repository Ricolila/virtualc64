// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ExpansionPort.h"
#include "C64.h"

namespace vc64 {

void
ExpansionPort::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (cartridge) {
        cartridge->reset(hard);
        cartridge->resetCartConfig();
    } else {
        setCartridgeMode(CRTMODE_OFF);
    }
}

isize
ExpansionPort::_size()
{
    util::SerCounter counter;
    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    if (cartridge) counter.count += cartridge->size();
    return counter.count;
}

u64
ExpansionPort::_checksum()
{
    util::SerChecker checker;

    applyToPersistentItems(checker);
    applyToResetItems(checker);

    if (cartridge) {
        checker.hash = util::fnvIt64(checker.hash, cartridge->checksum());
    }

    return checker.hash;
}

isize
ExpansionPort::_load(const u8 *buffer)
{
    util::SerReader reader(buffer);
    applyToPersistentItems(reader);
    applyToResetItems(reader);
    
    // Load cartridge (if any)
    if (crtType != CRT_NONE) {
        cartridge = std::unique_ptr<Cartridge>(Cartridge::makeWithType(c64, crtType));
        reader.ptr += cartridge->load(reader.ptr);
    }
    
    debug(SNP_DEBUG, "Recreated from %ld bytes\n", isize(reader.ptr - buffer));
    return isize(reader.ptr - buffer);
}

isize
ExpansionPort::_save(u8 *buffer)
{
    util::SerWriter writer(buffer);
    applyToPersistentItems(writer);
    applyToResetItems(writer);

    // Save cartridge (if any)
    if (crtType != CRT_NONE) {
        writer.ptr += cartridge->save(writer.ptr);
    }
    
    debug(SNP_DEBUG, "Serialized to %ld bytes\n", isize(writer.ptr - buffer));
    return isize(writer.ptr - buffer);
}

void
ExpansionPort::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Inspection) {
        
        os << tab("Game line");
        os << bol(gameLine) << std::endl;
        os << tab("Exrom line");
        os << bol(exromLine) << std::endl;
        os << tab("Cartridge");
        os << bol(cartridge != nullptr, "attached", "none") << std::endl;

        if (cartridge) {
            os << std::endl;
            cartridge->dump(category, os);
        }
    }

    if (category == Category::Debug) {

        if (cartridge) {
            os << std::endl;
            cartridge->dump(category, os);
        }
    }
}

CartridgeInfo
ExpansionPort::getInfo() const
{
    return cartridge ? cartridge->getInfo() : CartridgeInfo { };
}

CartridgeRomInfo
ExpansionPort::getRomInfo(isize nr) const
{
    return cartridge ? cartridge->getRomInfo(nr) : CartridgeRomInfo { };
}

CartridgeType
ExpansionPort::getCartridgeType() const
{
    return cartridge ? cartridge->getCartridgeType() : CRT_NONE;
}

u8
ExpansionPort::peek(u16 addr)
{
    return cartridge ? cartridge->peek(addr) : 0;
}

u8
ExpansionPort::spypeek(u16 addr) const
{
    return cartridge ? cartridge->spypeek(addr) : 0;
}

u8
ExpansionPort::peekIO1(u16 addr)
{
    /* "Die beiden mit "I/O 1" und "I/O 2" bezeichneten Bereiche
     *  sind für Erweiterungskarten reserviert und normalerweise ebenfalls offen,
     *  ein Lesezugriff liefert auch hier "zufällige" Daten (dass diese Daten gar
     *  nicht so zufällig sind, wird in Kapitel 4 noch ausführlich erklärt. Ein
     *  Lesen von offenen Adressen liefert nämlich auf vielen C64 das zuletzt vom
     *  VIC gelesene Byte zurück!)" [C.B.]
     */
    return cartridge ? cartridge->peekIO1(addr) : vic.getDataBusPhi1();
}

u8
ExpansionPort::spypeekIO1(u16 addr) const
{
    return cartridge ? cartridge->spypeekIO1(addr) : vic.getDataBusPhi1();
}

u8
ExpansionPort::peekIO2(u16 addr)
{
    return cartridge ? cartridge->peekIO2(addr) : vic.getDataBusPhi1();
}

u8
ExpansionPort::spypeekIO2(u16 addr) const
{
    return cartridge ? cartridge->spypeekIO2(addr) : vic.getDataBusPhi1();
}

void
ExpansionPort::poke(u16 addr, u8 value)
{
    if (cartridge) {
        cartridge->poke(addr, value);
    } else if (!c64.getUltimax()) {
        mem.ram[addr] = value;
    }
}

void
ExpansionPort::pokeIO1(u16 addr, u8 value)
{
    assert(addr >= 0xDE00 && addr <= 0xDEFF);
    
    if (cartridge) cartridge->pokeIO1(addr, value);
}

void
ExpansionPort::pokeIO2(u16 addr, u8 value)
{
    assert(addr >= 0xDF00 && addr <= 0xDFFF);
    
    if (cartridge) cartridge->pokeIO2(addr, value);
}

void
ExpansionPort::setGameLine(bool value)
{
    gameLine = value;
    vic.setUltimax(!gameLine && exromLine);
    mem.updatePeekPokeLookupTables();
}

void
ExpansionPort::setExromLine(bool value)
{
    exromLine = value;
    vic.setUltimax(!gameLine && exromLine);
    mem.updatePeekPokeLookupTables();
}

void
ExpansionPort::setGameAndExrom(bool game, bool exrom)
{
    gameLine = game;
    exromLine = exrom;
    vic.setUltimax(!gameLine && exromLine);
    mem.updatePeekPokeLookupTables();
}

CRTMode
ExpansionPort::getCartridgeMode() const
{
    switch ((exromLine ? 0b10 : 0) | (gameLine ? 0b01 : 0)) {
            
        case 0b00: return CRTMODE_16K;
        case 0b01: return CRTMODE_8K;
        case 0b10: return CRTMODE_ULTIMAX;
        default:   return CRTMODE_OFF;
    }
}

void
ExpansionPort::setCartridgeMode(CRTMode mode)
{
    switch (mode) {
            
        case CRTMODE_16K:     setGameAndExrom(0,0); return;
        case CRTMODE_8K:      setGameAndExrom(1,0); return;
        case CRTMODE_ULTIMAX: setGameAndExrom(0,1); return;
        default:              setGameAndExrom(1,1);
    }
}

void
ExpansionPort::attachCartridge(Cartridge *c)
{
    assert(c);
    assert(c->isSupported());
    
    {   SUSPENDED
        
        // Remove old cartridge (if any) and assign new one
        detachCartridge();
        cartridge = std::unique_ptr<Cartridge>(c);
        crtType = c->getCartridgeType();
        
        // Reset cartridge to update exrom and game line on the expansion port
        cartridge->reset(true);
        
        msgQueue.put(MSG_CRT_ATTACHED);
        if (cartridge->hasSwitch()) msgQueue.put(MSG_CART_SWITCH);
        
        debug(EXP_DEBUG, "Cartridge attached to expansion port");
        
    }
}

void
ExpansionPort::attachReuCartridge(isize kb)
{
    debug(EXP_DEBUG, "Attaching REU cartridge (%zu KB)", kb);
    attachCartridge(new Reu(c64, kb));
}

void
ExpansionPort::attachGeoRamCartridge(isize kb)
{
    debug(EXP_DEBUG, "Attaching GeoRAM cartridge (%zu KB)", kb);

    // kb must be a power of two between 64 and 4096
    assert(kb >= 64 && kb <= 4096 && !(kb & (kb - 1)));
    
    attachCartridge(new GeoRAM(c64, kb));
}

void
ExpansionPort::attachCartridge(const string &path, bool reset)
{
    attachCartridge(new CRTFile(path), reset);
}

void
ExpansionPort::attachCartridge(CRTFile *file, bool reset)
{
    assert(file);
    
    // Only proceed if this cartridge is supported
    if (!file->isSupported()) throw VC64Error(ERROR_CRT_UNSUPPORTED);
    
    // Create cartridge from cartridge file
    Cartridge *cartridge = Cartridge::makeWithCRTFile(c64, *file);

    // Attach cartridge to the expansion port
    {   SUSPENDED
        
        attachCartridge(cartridge);
        if (reset) c64.hardReset();
    }
}

void
ExpansionPort::attachIsepicCartridge()
{
    debug(EXP_DEBUG, "Attaching Isepic cartridge\n");
    
    Cartridge *isepic = new Isepic(c64); //  Cartridge::makeWithType(c64, CRT_ISEPIC);
    (void)attachCartridge(isepic);
}

void
ExpansionPort::detachCartridge()
{
    {   SUSPENDED
        
        if (cartridge) {
            
            cartridge = nullptr;
            crtType = CRT_NONE;
            
            setCartridgeMode(CRTMODE_OFF);
            
            debug(EXP_DEBUG, "Cartridge detached from expansion port");
            msgQueue.put(MSG_CRT_DETACHED);
        }
    }
}

void
ExpansionPort::detachCartridgeAndReset()
{
    {   SUSPENDED
        
        detachCartridge();
        c64.hardReset();
    }
}

isize
ExpansionPort::getRamCapacity() const
{
    return cartridge ? cartridge->getRamCapacity() : 0;
}

bool
ExpansionPort::hasBattery() const
{
    return cartridge ? cartridge->getBattery() : false;
}

void
ExpansionPort::setBattery(bool value)
{
    if (cartridge) cartridge->setBattery(value);
}

isize
ExpansionPort::numButtons() const
{
    return cartridge ? cartridge->numButtons() : 0;
}

const string
ExpansionPort::getButtonTitle(isize nr) const
{
    return cartridge ? cartridge->getButtonTitle(nr) : nullptr;
}

void
ExpansionPort::pressButton(isize nr)
{
    if (cartridge) cartridge->pressButton(nr);
}

void
ExpansionPort::releaseButton(isize nr)
{
    if (cartridge) cartridge->releaseButton(nr);
}

bool
ExpansionPort::hasSwitch() const
{
    return cartridge ? cartridge->hasSwitch() : false;
}

isize
ExpansionPort::getSwitch() const
{
    return cartridge ? cartridge->getSwitch() : 0;
}

bool
ExpansionPort::switchIsNeutral() const
{
    return cartridge ? cartridge->switchIsNeutral() : false;
}

bool
ExpansionPort::switchIsLeft() const
{
    return cartridge ? cartridge->switchIsLeft() : false;
}

bool
ExpansionPort::switchIsRight() const
{
    return cartridge ? cartridge->switchIsRight() : false;
}

const string
ExpansionPort::getSwitchDescription(isize pos) const
{
    return cartridge ? cartridge->getSwitchDescription(pos) : "";
}

const string
ExpansionPort::getSwitchDescription() const
{
    return getSwitchDescription(getSwitch());
}

bool
ExpansionPort::validSwitchPosition(isize pos) const
{
    return cartridge ? cartridge->validSwitchPosition(pos) : false;
}

bool
ExpansionPort::hasLED() const
{
    return cartridge ? cartridge->hasLED() : false;
}

bool
ExpansionPort::getLED() const
{
    return cartridge ? cartridge->getLED() : false;
}

void
ExpansionPort::setLED(bool value)
{
    if (cartridge) cartridge->setLED(value);
}

void
ExpansionPort::execute()
{
    if (cartridge) cartridge->execute();
}

void
ExpansionPort::updatePeekPokeLookupTables()
{
    if (cartridge) cartridge->updatePeekPokeLookupTables();
}

void
ExpansionPort::nmiWillTrigger()
{
    if (cartridge) cartridge->nmiWillTrigger();
}

void
ExpansionPort::nmiDidTrigger()
{
    if (cartridge) cartridge->nmiDidTrigger();
}

}
