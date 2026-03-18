# Bare-Metal STM32 Infrared (IR) Controller & Modulator
**Zero-Overhead Hardware Pulse Modulation via DMA and Master-Slave Timers**

This repository contains a purely bare-metal (No-HAL) C implementation of an Infrared transmitter for STM32 microcontrollers. It bypasses high-level abstractions to achieve nanosecond-level precision by directly manipulating CPU registers, advanced timer topologies, and the Direct Memory Access (DMA) controller.

## Architectural Highlights
Instead of relying on blocking CPU delays or basic interrupt toggling, this system leverages a highly complex hardware state machine to generate IR protocols (e.g., NEC, RC5) with **zero CPU overhead** during transmission:

* **Master-Gated-Slave Timer Topology:** * **Slave Timer (`TIM3`):** Configured in PWM mode to continuously generate the precise 38kHz carrier wave.
  * **Master Timer (`TIM2`):** Configured to gate the Slave Timer. Its duty cycle dictates exactly how long the 38kHz burst lasts (logical `1` or `0`).
* **Dual-Stream DMA Modulation (`DMA1`):** * The mathematical pulse sequences are pre-calculated and compacted (`IR_compact_message()`).
  * Two concurrent DMA streams (`DMA1_Stream5`, `DMA1_Stream7`) feed the Auto-Reload (`ARR`) and Capture/Compare (`CCR`) registers of the Master Timer *in flight*. 
  * This dynamically modulates the pulse widths at the exact microsecond the previous pulse finishes, completely independently of the ARM Cortex core.

## Overcoming Silicon Limitations
Developing this bare-metal controller required navigating undocumented hardware behavior and silicon errata:
* **DMA/PWM Halt Desynchronization:** STM32 DMA streams do not natively shut off a PWM timer cleanly at the exact end of a buffer. To prevent signal trailing, a custom 4-stage interrupt state machine (`TRANSFER_END_SEQ_...`) was engineered to load dummy values and gracefully halt the carrier wave on the final Capture/Compare event.
* **Pipeline Deadlocks:** Identified and bypassed instruction pipeline synchronization issues during interrupt masking, requiring precise injection of `__NOP()` instructions before entering Wait-For-Interrupt (`__WFI()`) sleep states to prevent CPU lockups.

## Technical Debt & Known Limitations
This project was built in 2023 with a primary focus on mastering bare-metal hardware mechanics (Timers, DMA, Interrupts) rather than strict C language architecture. As a result, there are architectural flaws that remain as a baseline record of my learning process:

* **C-Language Architectural Flaws (ODR Violations):** This codebase strictly predates my work in Linux Kernel module development. Consequently, to expedite the build process at the time, several function implementations and global constants were placed directly into header files (`.h`), violating the One Definition Rule (ODR). This project stands as a baseline record of my early architectural learning curve before adopting the strict C standards required for kernel-space engineering.
* **Hardware Re-Arming Desynchronization (The "One-Shot" Bug):** The mathematical timing model and the initial DMA transfer logic work flawlessly. However, the software logic responsible for re-arming the DMA streams and clearing timer event flags (`TCIF`, `NDTR` reloads) after the first transmission is flawed. The state machine fails to correctly reset the hardware lifecycle, meaning the MCU successfully transmits only the first message and requires a hard reset for subsequent ones. It was a failure of software state management, not a silicon limitation.
* **Global State Coupling:** Interrupt Service Routines (ISRs) are hardcoupled to a global singleton (`main_controller`). The architecture is not reentrant, preventing the scaling of the system to multiple IR transmitters on different timers without duplicating the interrupt logic.
* **Type Erasure Hazards:** The controller structure uses `void*` arrays for buffer management with manual casting based on flag states, bypassing the compiler's type safety rather than using memory-safe `union` structures. 

## Tech Stack
* **Language:** Pure C99 (Bare-metal)
* **Hardware:** STM32 Cortex-M Series
* **Concepts:** Advanced DMA, Multi-Timer Synchronization, Direct Register Manipulation, Interrupt Priority Masking, Low-Power States, Polymorphic C Structures.

## Build
This project was built for the STM32F4xx architecture. It does not compile natively and provided `Makefile` withing the `IRController/` directory has hardcoded include paths for a cross-compilation toolchain that was available in the lab.
