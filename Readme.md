# VenV - Virtual Environment Engine

A lightweight virtual environment engine that emulates complete computer environments with a custom CPU architecture.

## Overview

VenV (Virtual Environment) is a **virtual hardware emulator**, not an OS-level container like Docker. Each environment contains:

- A custom virtual CPU (VenV ISA - not based on x86, ARM, RISC-V, or MIPS)
- Virtual RAM with sparse allocation
- Virtual block device (disk)
- Virtual console
- Virtual timer
- Virtual network interface

The engine provides **virtual hardware**. The guest operating system (eventually Linux) provides all OS functionality.

## Architecture

```
HOST
│
└── VenV Engine
     │
     ├── Virtual CPU (custom ISA)
     ├── Virtual RAM
     ├── Virtual Disk → disk.img
     ├── Virtual Console → host stdout/stdin
     ├── Virtual Timer → interrupts
     └── Virtual NIC → host network
             │
             ▼
          Guest OS (Linux)
               │
               ├── Processes
               ├── Filesystem (ext4, etc.)
               ├── TCP/IP stack
               └── Applications
```

## Project Structure

```
/workspace/
├── Include/
│   ├── Core/           # Core runtime (from existing project)
│   ├── Std/            # Standard library (from existing project)
│   └── VenV/           # VenV engine headers
│       ├── VenV.h      # Main header
│       ├── ISA.h       # CPU instruction set architecture
│       ├── CPU.h       # CPU state and operations
│       ├── Memory.h    # Virtual memory subsystem
│       ├── Device.h    # Virtual device interface
│       ├── Timer.h     # Virtual timer device
│       ├── Console.h   # Virtual console device
│       ├── Block.h     # Virtual block device
│       ├── Net.h       # Virtual network device
│       ├── VM.h        # Virtual machine abstraction
│       └── Assembler.h # Assembler for VenV ISA
├── Src/
│   ├── CPU.c           # CPU implementation
│   ├── Memory.c        # Memory implementation
│   └── Assembler.c     # Assembler implementation
├── Test/
│   └── test_venv.c     # Test program
├── Environments/       # VM instances (created at runtime)
└── Readme.md
```

## Custom CPU ISA

The VenV ISA is a completely new 64-bit architecture designed for efficient emulation:

### Registers
- 16 general-purpose 64-bit registers (x0-x15)
- x0 is hardwired to zero
- x14 is stack pointer (sp)
- x15 is return address (ra)
- Program counter (pc)
- Status register (interrupts, privilege level)

### Instruction Format
- Fixed 32-bit instructions
- Little-endian encoding
- Simple opcode/register/immediate fields

### Instructions
- **Load/Store**: lb, lh, lw, ld, sb, sh, sw, sd
- **Arithmetic**: add, sub, mul, div, rem
- **Logical**: and, or, xor
- **Shift**: sll, srl, sra
- **Compare**: slt, sltu
- **Branch**: beq, bne, blt, bge, bltu, bgeu
- **Jump**: jal, jalr
- **Immediate**: addi, andi, ori, xori, slli, srli, srai, slti, sltiu
- **System**: ecall, ebreak, mret, fence, wfi

### Privilege Levels
- User mode (least privileged)
- Supervisor mode
- Machine mode (most privileged)

### Exceptions & Interrupts
- Instruction faults
- Load/store faults
- System calls (ecall)
- Breakpoints (ebreak)
- Timer interrupts
- External interrupts

## Development Phases

### Phase 1: Core CPU (Current)
- [x] ISA definition
- [x] CPU interpreter
- [x] Virtual memory
- [x] Basic assembler
- [ ] Test programs

### Phase 2: Devices
- [ ] Timer device
- [ ] Console device
- [ ] Block device
- [ ] MMIO framework

### Phase 3: End-to-End
- [ ] Run simple programs
- [ ] Console I/O
- [ ] Disk I/O

### Phase 4: Networking
- [ ] Virtual NIC
- [ ] Host network bridge
- [ ] Port forwarding

### Phase 5: OS Support
- [ ] Full interrupt controller
- [ ] Memory management unit
- [ ] Boot process

### Phase 6: Linux Port
- [ ] Linux kernel port
- [ ] Device drivers
- [ ] Minimal root filesystem

## Building

Requires GCC or Clang with C99 support.

```bash
# Compile test program
gcc -I./Include -o test_venv Test/test_venv.c Src/CPU.c Src/Memory.c Src/Assembler.c

# Run tests
./test_venv
```

## Usage (Future CLI)

```bash
# Create a new environment
venv create myenv --image minimal-linux

# Start environment
venv start myenv

# Execute command in environment
venv exec myenv /bin/sh

# Port forwarding
venv start web --port 8080:80

# Stop environment
venv stop myenv

# Destroy environment
venv destroy myenv
```

## Design Principles

1. **Emulate hardware, not OS** - The engine provides virtual devices; Linux provides filesystems, networking, processes.

2. **Simple and efficient** - The ISA is designed for fast interpretation, with potential for future JIT compilation.

3. **Minimal footprint** - Sparse memory allocation, lazy device initialization.

4. **Clear boundaries** - Well-defined interfaces between CPU, memory, and devices.

5. **Extensible** - Designed to eventually support full Linux virtualization.

## License

See LICENSE file.
