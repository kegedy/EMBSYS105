### Programming with Embedded & Real-Time Operating Systems

#### Week 1

- Embedded Operating System Concepts Part 1 (Labrosse Chap 2: Real-Time Systems Concepts)
  - Foreground/Background Systems
  - Critical sections
  - Shared resources
  - Multitasking
  - Tasks
  - Context switching
- ARM Cortex Review (Yiu chapters 4, 8)
  - States and Modes
  - Registers
  - Assembly language instructions
  - Interrupt semantics
- Assignment 1: uDebugger tool (due week 2)

#### Week 2

Labrosse Chap 2: Real-Time Systems Concepts; Yiu Chapter 10

- Embedded Operating System Concepts Part 2
  - Multitasking OS concepts
    - The kernel, Preemptive and non-preemptive kernels, The scheduler, Scheduling algorithms, Task states, Parameters used in scheduling – quantum, priority, Priority inversion
  - Real time OS concepts
    - Definition, Hard/soft real time, Jitter, OS timer tick, Interrupt latency
  - Context switching on ARM Cortex
  - Assignment 2: Context Switch (due week 3)

#### Week 3

- MicroC/OS-II (uCOS) Introduction
  - Task creation, Task Delay, Sample task code, uCOS startup steps
- uCOS Services (Labrosse Chap 16: μC/OS-II Reference Manual)
  - Task Management
  - Time Management
  - Interrupt Management
  - Semaphores
  - Mutexes
  - Message Mailboxes
  - Message Queues
  - Event Flags
  - Memory Management
  - User-Defined Functions
  - Miscellaneous Services
- uCOS Configuration (Labrosse Chap 17: μC/OS-II Configuration Manual)
- Porting uCOS to our board (Labrosse Chap 13: Porting μC/OS-II)
  - Key data definitions, enabling and disabling interrupts, critical section implementation, initializing the stack, context switching
- Assignment 3 – uCOS Port

#### Week 4

- uCOS Internals
       Labrosse Chap 3: Kernel Structure; Chap 6: Event Control Blocks; Chap 9: Event Flag Management)

  - Task States
  - Task Control Blocks (TCBs)
  - Ready List
  - Task Scheduling
  - Event Control Blocks
    - Placing a task in the ECB Wait List
    - Removing a Task from the ECB Wait List
    - Find the Highest Priority Task Waiting on the ECB
    - List of Free ECBs
    - Initializing an ECB
    - Making a Task Ready
    - Making a Task Wait for an Event
    - Making a task Ready Because of a Timeout
  - Event Flags
- Task Synchronization Techniques (Tanenbaum, Wikipedia, etc.)
- Sharing data between ISRs and tasks
- Producer-Consumer Problem
- Readers and Writers Problem
- Deadlock
- Dining Philosophers Problem
- Event Driven Systems (Wikipedia)
- Characteristics of Event Driven Systems
- Data Flow Diagrams
- Assignment 4 - Task synchronization
- Course Project – MP3 Player

#### Week 5

- Review ARM procedure call standard
- OS_TASK_SW() exploration
- More on event driven systems
- Music Player Project
    - Assignment spec
    - Demo skeleton code
    - Skeleton code walk through
- More design flow diagrams
- Assignment - event driven systems

#### Week 6

- Event driven MP3 player ideas
- Device Drivers
  - What is a Device Driver?
  - What is a Class Driver?
  - What is a Device Driver Model?
  - Char Device Driver Model
- PJDF – PJ Driver Framework
  - Internals of PJDF simplified
  - MP3Player drivers
  - Adding a new driver to PJDF
  - Walk through pjdf.h developer TODO LIST
- C++ Introduction
- Task priorities - Rate monotonic scheduling

#### Week 7

- Adding a device to the MP3 project
- SPI - Serial Peripheral Interface
- I2C - Inter-Integrated Circuit
- VS1053 MP3 decoder

#### Week 8

- SPI
- I2C
- Logic Analyzers
- SD/MMC

#### Week 9

- FAT File System
- Sample code to read directory with Arduino SD library
- Explore SD code
- Changing system clock rate
- Findings of an embedded survey
- Demo homegrown kernel
- Lab: Add a command shell to your multithreading app

#### Week 10

- Student project presentations
