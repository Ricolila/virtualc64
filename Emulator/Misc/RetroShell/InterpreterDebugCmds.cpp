// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// This FILE is dual-licensed. You are free to choose between:
//
//     - The GNU General Public License v3 (or any later version)
//     - The Mozilla Public License v2
//
// SPDX-License-Identifier: GPL-3.0-or-later OR MPL-2.0
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Emulator.h"
#include "IOUtils.h"
#include "Parser.h"

#include <fstream>
#include <sstream>

namespace vc64 {

void
Interpreter::initDebugShell(Command &root)
{
    initCommons(root);

    //
    // Debug variables
    //

    root.add({"debug"}, "Debug variables");

    root.add({"debug", ""}, {},
             "Display all debug variables",
             [this](Arguments& argv, long value) {

        retroShell.dump(emulator, Category::Debug);
    });

    if (debugBuild) {

        for (isize i = DebugFlagEnum::minVal; i < DebugFlagEnum::maxVal; i++) {

            root.add({"debug", DebugFlagEnum::key(i)}, { Arg::boolean },
                     DebugFlagEnum::help(i),
                     [this](Arguments& argv, long value) {

                c64.setDebugVariable(value, util::parseBool(argv[0]));

            }, i);
        }
    }

    
    //
    // Program execution
    //

    root.pushGroup("Program execution");

    root.add({"goto"}, { }, { Arg::value },
             std::pair <string, string>("g[oto]", "Goto address"),
             [this](Arguments& argv, long value) {

        argv.empty() ? emulator.run() : cpu.jump(parseAddr(argv[0]));
    });
    root.clone("g", {"goto"});

    root.add({"step"}, { }, { },
             std::pair <string, string>("s[tep]", "Step into the next instruction"),
             [this](Arguments& argv, long value) {

        emulator.stepInto();
    });
    root.clone("s", {"step"});

    root.add({"next"}, { }, { },
             std::pair <string, string>("n[next]", "Step over the next instruction"),
             [this](Arguments& argv, long value) {

        emulator.stepOver();
    });
    root.clone("n", {"next"});

    root.add({"break"},     "Manage CPU breakpoints");
    root.pushGroup("");

    root.add({"break", ""},
             "List all breakpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Breakpoints);
    });

    root.add({"break", "at"}, { Arg::address }, { Arg::ignores },
             "Set a breakpoint",
             [this](Arguments& argv, long value) {

        cpu.setBreakpoint(parseAddr(argv[0]), parseNum(argv, 1, 0));
    });

    root.add({"break", "delete"}, { Arg::nr },
             "Delete breakpoints",
             [this](Arguments& argv, long value) {

        cpu.deleteBreakpoint(parseNum(argv[0]));
    });

    root.add({"break", "toggle"}, { Arg::nr },
             "Enable or disable breakpoints",
             [this](Arguments& argv, long value) {

        cpu.toggleBreakpoint(parseNum(argv[0]));
    });

    root.popGroup();

    root.add({"watch"},     "Manage CPU watchpoints");
    
    root.pushGroup("");

    root.add({"watch", ""},
             "List all watchpoints",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, Category::Watchpoints);
    });

    root.add({"watch", "at"}, { Arg::address }, { Arg::ignores },
             "Set a watchpoint",
             [this](Arguments& argv, long value) {

        cpu.setWatchpoint(parseAddr(argv[0]), parseNum(argv, 1, 0));
    });

    root.add({"watch", "delete"}, { Arg::nr },
             "Delete watchpoints",
             [this](Arguments& argv, long value) {

        cpu.deleteWatchpoint(parseNum(argv[0]));
    });

    root.add({"watch", "toggle"}, { Arg::nr },
             "Enable or disable watchpoints",
             [this](Arguments& argv, long value) {

        cpu.toggleWatchpoint(parseNum(argv[0]));
    });

    root.popGroup();

    
    //
    // Monitoring
    //

    root.pushGroup("Monitoring");

    root.add({"d"}, { }, { Arg::address },
             "Disassemble instructions",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        cpu.disassembler.disassembleRange(ss, parseAddr(argv, 0, cpu.getPC0()), 16);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"a"}, { }, { Arg::address },
             "Dump memory in ASCII",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        debugger.ascDump(ss, parseAddr(argv, 0, debugger.current), 16);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"m"}, { }, { Arg::address },
             std::pair<string, string>("m[.b|.w|.l]", "Dump memory"),
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        debugger.memDump(ss, parseAddr(argv, 0, debugger.current), 16);
        retroShell << '\n' << ss << '\n';
    });

    root.add({"i"},
             "Inspect a component");

    root.pushGroup("Components");

    root.add({"i", "emulator"},      "Emulator");
    root.add({"i", "c64"},           "C64");

    root.popGroup();

    root.pushGroup("Peripherals");

    root.add({"i", "keyboard"},      "Keyboard");
    root.add({"i", "mouse"},         "Mouse");
    root.add({"i", "joystick"},      "Joystick");

    root.popGroup();

    root.pushGroup("Miscellaneous");

    root.add({"i", "host"},          "Host machine");
    root.add({"i", "server"},        "Remote server");

    root.popGroup();

    root.add({"r"},
             "Show registers");

    root.popGroup();
/*

    root.add({"disassemble"}, { }, { Arg::address },
             "Runs disassembler",
             [this](Arguments& argv, long value) {

        std::stringstream ss;

        auto addr = argv.empty() ? cpu.getPC0() : u16(parseNum(argv[0]));
        cpu.disassembler.disassembleRange(ss, addr, 16);

        retroShell << '\n' << ss << '\n';
    });
    */


    root.pushGroup("Debugging components");

    root.add({"thread"},        "The emulator thread");
    root.add({"c64"},           "The virtual Commodore 64");
    root.add({"cpu"},           "MOS 6810 CPU");
    root.add({"memory"},        "Ram and Rom");
    root.add({"cia1"},          "Complex Interface Adapter 1");
    root.add({"cia2"},          "Complex Interface Adapter 2");
    root.add({"vicii"},         "Video Interface Controller");
    root.add({"sid"},           "Sound Interface Device");

    root.pushGroup("Debugging ports");

    root.add({"controlport1"},  "Control port 1");
    root.add({"controlport2"},  "Control port 2");
    root.add({"expansion"},     "Expansion port");

    root.pushGroup("Debugging peripherals");

    root.add({"keyboard"},      "Keyboard");
    root.add({"mouse"},         "mouse");
    root.add({"joystick"},      "Joystick");
    root.add({"datasette"},     "Commodore tape drive");
    root.add({"drive8"},        "Floppy drive 8");
    root.add({"drive9"},        "Floppy drive 9");
    root.add({"parcable"},      "Parallel drive cable");


    //
    // Thread
    //

    root.add({"thread", ""},
             "Displays the thread state",
             [this](Arguments& argv, long value) {

        retroShell.dump(emulator, Category::State);
    });

    root.add({"thread", "runahead"},
             "Inspects the run-ahead instance",
             [this](Arguments& argv, long value) {

        retroShell.dump(emulator, Category::RunAhead);
    });


    //
    // C64
    //

    root.pushGroup("");

    root.add({"c64"},
             "Displays the component state");

    root.add({"c64", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(c64, { Category::Config, Category::State });
    });

    root.add({"c64", "host"},
             "Displays information about the host machine",
             [this](Arguments& argv, long value) {

        retroShell.dump(host, Category::State);
    });

    root.add({"c64", "checksums"},
             "Displays checksum of various components",
             [this](Arguments& argv, long value) {

        retroShell.dump(c64, Category::Checksums);
    });

    root.add({"c64", "sizeof"},
             "Displays static memory footprints of various components",
             [this](Arguments& argv, long value) {

        retroShell.dump(c64, Category::Sizeof);
    });


    //
    // Memory
    //

    root.add({"memory", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(mem, { Category::Config, Category::State });
    });

    root.add({"memory", "dump"}, { Arg::address },
             "Generates a memory hexdump",
             [this](Arguments& argv, long value) {

        std::stringstream ss;
        mem.memDump(ss, u16(parseNum(argv[0])));
        retroShell << '\n' << ss << '\n';
    });

    root.add({"memory", "read"}, { Arg::address }, { MemoryTypeEnum::argList() },
             "Reads a byte from memory",
             [this](Arguments& argv, long value) {

        auto addr = u16(parseNum(argv[0]));
        MemoryType type = argv.size() == 1 ? mem.peekSrc[addr >> 12] : parseEnum <MemoryTypeEnum> (argv[0], 1);
        auto byte = mem.peek(addr, type);

        std::stringstream ss;
        ss << util::hex(addr) << ": " << util::hex(byte);
        ss << " (" << MemoryTypeEnum::key(type) << ")\n";
        retroShell << ss;
    });

    root.add({"memory", "write"}, { Arg::address, Arg::value }, { MemoryTypeEnum::argList() },
             "Writes a byte into memory",
             [this](Arguments& argv, long value) {

        auto addr = u16(parseNum(argv[0]));
        auto byte = u8(parseNum(argv[1]));
        MemoryType type = argv.size() == 2 ? mem.pokeTarget[addr >> 12] : parseEnum <MemoryTypeEnum> (argv[2]);

        mem.poke(addr, byte, type);
    });


    //
    // Drive
    //

    for (isize i = 0; i < 2; i++) {

        string drive = (i == 0) ? "drive8" : "drive9";

        root.add({drive, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            auto &drive = value ? drive9 : drive8;
            retroShell.dump(drive, { Category::Config, Category::State });
        });

        root.add({drive, "bankmap"},
                 "Displays the memory layout",
                 [this](Arguments& argv, long value) {

            auto &drive = value ? drive9 : drive8;
            retroShell.dump(drive, Category::BankMap);
        });

        root.add({drive, "disk"},
                 "Inspects the current disk",
                 [this](Arguments& argv, long value) {

            auto &drive = value ? drive9 : drive8;
            retroShell.dump(drive, Category::Disk);
        });

        root.add({drive, "layout"},
                 "Displays the disk layout",
                 [this](Arguments& argv, long value) {

            auto &drive = value ? drive9 : drive8;
            retroShell.dump(drive, Category::Layout);
        });
    }


    //
    // Datasette
    //


    root.add({"datasette", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(datasette, { Category::Config, Category::State });
    });


    //
    // CPU
    //

    root.add({"cpu", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(cpu, { Category::Config, Category::State });
    });


    //
    // CIA
    //

    for (isize i = 0; i < 2; i++) {

        string cia = (i == 0) ? "cia1" : "cia2";

        root.add({cia, ""},
                 "Displays the component state",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(cia1, { Category::Config, Category::State });
            } else {
                retroShell.dump(cia2, { Category::Config, Category::State });
            }

        }, i);

        root.add({cia, "registers"},
                 "Displays the current register values",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(cia1, Category::Registers);
            } else {
                retroShell.dump(cia2, Category::Registers);
            }

        }, i);

        root.add({cia, "tod"},
                 "Displays the state of the TOD clock",
                 [this](Arguments& argv, long value) {

            if (value == 0) {
                retroShell.dump(cia1.tod, Category::State);
            } else {
                retroShell.dump(cia2.tod, Category::State);
            }

        }, i);
    }


    //
    // VICII
    //

    root.add({"vicii", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(vic, { Category::Config, Category::State });
    });

    root.add({"vicii", "registers"},
             "Dumps all VICII registers",
             [this](Arguments& argv, long value) {

        retroShell.dump(vic, Category::Registers);
    });


    //
    // SID
    //

    root.add({"sid", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(muxer, { Category::Config, Category::State });
    });


    //
    // Control port
    //

    for (isize i = 0; i < 2; i++) {

        string port = (i == 0) ? "controlport1" : "controlport2";

        root.add({port, ""},
                 "Inspects the internal state",
                 [this](Arguments& argv, long value) {

            retroShell.dump(value == 0 ? port1 : port2, { Category::Config, Category::State });
        });
    }


    //
    // Expansion port
    //

    root.add({"expansion", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(expansionport, { Category::Config, Category::State });
    });

    
    //
    // Keyboard
    //

    root.add({"keyboard", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(keyboard, { Category::Config, Category::State });
    });


    //
    // Joystick
    //

    root.add({"joystick", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(port1.joystick, { Category::Config, Category::State });
        retroShell << '\n';
        retroShell.dump(port2.joystick, { Category::Config, Category::State });
    });


    //
    // Mouse
    //

    root.add({"mouse", ""},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(port1.mouse, { Category::Config, Category::State });
        retroShell << '\n';
        retroShell.dump(port2.mouse, { Category::Config, Category::State });
    });


    //
    // Parallel cable
    //

    root.add({"parcable"},
             "Inspects the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dump(parCable, { Category::Config, Category::State });
    });
}


}
