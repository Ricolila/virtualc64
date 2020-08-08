// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "C64.h"

CPU::CPU(CPUModel model, C64& ref) : C64Component(ref)
{
    this->model = model;
	
    setDescription(model == MOS_6502 ? "CPU(6502)" : "CPU");
    
    subComponents = vector<HardwareComponent *> {
        
        &pport,
        &debugger
    };
    
    // Chip model
    model = MOS_6510;

	// Establish callback for each instruction
	registerInstructions();
		    
    // Register snapshot items
    SnapshotItem items[] = {
        
        // Lifetime items
        { &model,              sizeof(model),        KEEP_ON_RESET },

         // Internal state
        { &flags,              sizeof(flags),        CLEAR_ON_RESET },
        { &cycle,              sizeof(cycle),        CLEAR_ON_RESET },
        { &halted,             sizeof(halted),       CLEAR_ON_RESET },
        { &next,               sizeof(next),         CLEAR_ON_RESET },

        { &reg.a,              sizeof(reg.a),        CLEAR_ON_RESET },
        { &reg.x,              sizeof(reg.x),        CLEAR_ON_RESET },
        { &reg.y,              sizeof(reg.y),        CLEAR_ON_RESET },
        { &reg.pc,             sizeof(reg.pc),       CLEAR_ON_RESET },
        { &reg.sp,             sizeof(reg.sp),       CLEAR_ON_RESET },
        { &reg.p,              sizeof(reg.p),        CLEAR_ON_RESET },
        { &reg.adl,            sizeof(reg.adl),      CLEAR_ON_RESET },
        { &reg.adh,            sizeof(reg.adh),      CLEAR_ON_RESET },
        { &reg.idl,            sizeof(reg.idl),      CLEAR_ON_RESET },
        { &reg.d,              sizeof(reg.d),        CLEAR_ON_RESET },
        { &reg.ovl,            sizeof(reg.ovl),      CLEAR_ON_RESET },
        { &pc,                 sizeof(pc),           CLEAR_ON_RESET },
        { &rdyLine,            sizeof(rdyLine),      CLEAR_ON_RESET },
        { &rdyLineUp,          sizeof(rdyLineUp),    CLEAR_ON_RESET },
        { &rdyLineDown,        sizeof(rdyLineDown),  CLEAR_ON_RESET },
        { &nmiLine,            sizeof(nmiLine),      CLEAR_ON_RESET },
        { &irqLine,            sizeof(irqLine),      CLEAR_ON_RESET },
        { &doNmi,              sizeof(doNmi),        CLEAR_ON_RESET },
        { &doIrq,              sizeof(doIrq),        CLEAR_ON_RESET },
        { NULL,                0,                    0 }};
    
    registerSnapshotItems(items, sizeof(items));
}

DisassembledInstruction
CPU::getInstrInfo(long nr, u16 start)
{
    // Update the cache if necessary
    if (info.start != start) _inspect(start);

    return getInstrInfo(nr);
}

DisassembledInstruction
CPU::getInstrInfo(long nr)
{
    assert(nr < CPUINFO_INSTR_COUNT);
    
    DisassembledInstruction result;
    synchronized { result = info.instr[nr]; }
    return result;
}

DisassembledInstruction
CPU::getLoggedInstrInfo(long nr)
{
    assert(nr < CPUINFO_INSTR_COUNT);
    
    DisassembledInstruction result;
    synchronized { result = info.loggedInstr[nr]; }
    return result;
}

void
CPU::_reset()
{
    // Clear snapshot items marked with 'CLEAR_ON_RESET'
     if (snapshotItems != NULL)
         for (unsigned i = 0; snapshotItems[i].data != NULL; i++)
             if (snapshotItems[i].flags & CLEAR_ON_RESET)
                 memset(snapshotItems[i].data, 0, snapshotItems[i].size);
    
    setB(1);
	rdyLine = true;
	next = fetch;
    levelDetector.clear();
    edgeDetector.clear();
}

void
CPU::_inspect()
{
    debug("CPU::_inspect ()\n");
    
    _inspect(getPC());
}

void
CPU::_inspect(u32 dasmStart)
{
    debug("CPU::_inspect (dasmStart = %x)\n", dasmStart);
    
    synchronized {
        
        info.cycle = cycle;

        info.pc = pc;
        info.sp = reg.sp;
        info.a = reg.a;
        info.x = reg.x;
        info.y = reg.y;
        
        info.nFlag = getN();
        info.vFlag = getV();
        info.bFlag = getB();
        info.dFlag = getD();
        info.iFlag = getI();
        info.zFlag = getZ();
        info.cFlag = getC();
        
        info.irq = irqLine;
        info.nmi = nmiLine;
        info.rdy = rdyLine;
        info.halted = isHalted();
        
        info.processorPort = pport.read();
        info.processorPortDir = pport.readDirection();
        
        // Disassemble the program starting at 'dasmStart'
        info.start = dasmStart;
        for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
            info.instr[i] = debugger.disassemble(dasmStart);
            dasmStart += info.instr[i].size;
        }
        
        // Disassemble the most recent entries in the trace buffer
        long count = debugger.loggedInstructions();
        for (int i = 0; i < count; i++) {
            RecordedInstruction rec = debugger.logEntryAbs(i);
            info.loggedInstr[i] = debugger.disassemble(rec);
        }
    }
}

void
CPU::_setDebug(bool enable)
{
    if (enable && isC64CPU()) {
        flags |= CPU::CPU_LOG_INSTRUCTION;
    } else {
        flags |= ~CPU::CPU_LOG_INSTRUCTION;
    }
}

void 
CPU::_dump()
{
    DisassembledInstruction instr = debugger.disassemble();
    
	msg("CPU:\n");
	msg("----\n\n");
    msg("%s: %s %s %s   %s %s %s %s %s %s\n",
        instr.pc,
        instr.byte1, instr.byte2, instr.byte3,
        instr.a, instr.x, instr.y, instr.sp,
        instr.flags,
        instr.command);
	msg("      Rdy line : %s\n", rdyLine ? "high" : "low");
    msg("      Nmi line : %02X\n", nmiLine);
    msg(" Edge detector : %02X\n", edgeDetector.current());
    msg("         doNmi : %s\n", doNmi ? "yes" : "no");
    msg("      Irq line : %02X\n", irqLine);
    msg("Level detector : %02X\n", levelDetector.current());
    msg("         doIrq : %s\n", doIrq ? "yes" : "no");
    msg("   IRQ routine : %02X%02X\n", spypeek(0xFFFF), spypeek(0xFFFE));
    msg("   NMI routine : %02X%02X\n", spypeek(0xFFFB), spypeek(0xFFFA));
	msg("\n");
    
    pport.dump();
}

size_t
CPU::stateSize()
{
    return HardwareComponent::stateSize()
    + levelDetector.stateSize()
    + edgeDetector.stateSize();
}

void
CPU::didLoadFromBuffer(u8 **buffer)
{
    levelDetector.loadFromBuffer(buffer);
    edgeDetector.loadFromBuffer(buffer);
}

void
CPU::didSaveToBuffer(u8 **buffer)
{
    levelDetector.saveToBuffer(buffer);
    edgeDetector.saveToBuffer(buffer);
}

void
CPU::pullDownNmiLine(IntSource bit)
{
    assert(bit != 0);
    
    // Check for falling edge on physical line
    if (!nmiLine) {
        edgeDetector.write(1);
    }
    
    nmiLine |= bit;
}

void
CPU::releaseNmiLine(IntSource source)
{
    nmiLine &= ~source;
}

void
CPU::pullDownIrqLine(IntSource source)
{
	assert(source != 0);
    
	irqLine |= source;
    levelDetector.write(irqLine);
}

void
CPU::releaseIrqLine(IntSource source)
{
    irqLine &= ~source;
    levelDetector.write(irqLine);
}

void
CPU::setRDY(bool value)
{
    if (rdyLine)
    {
        rdyLine = value;
        if (!rdyLine) rdyLineDown = cycle;
    }
    else
    {
        rdyLine = value;
        if (rdyLine) rdyLineUp = cycle;
    }
}

void
CPU::processFlags()
{
    // Record the instruction if requested
    if (flags & CPU_LOG_INSTRUCTION) {
        debugger.logInstruction();
    }
    
    // Check if a breakpoint has been reached
    if (flags & CPU_LOG_INSTRUCTION && debugger.breakpointMatches(reg.pc)) {
        c64.signalBreakpoint();
    }
}
