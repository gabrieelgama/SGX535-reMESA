# Poulsbo: interruptions and faults

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-030]** — DDK Poulsbo defines IER/IIR/IMR/ISR Intel in 0x20a0/20a4/20a8/20ac and THALIA/SGX bit18; translates the indication to DEVICE_SGX_INTERRUPT and clears IIR. Linux dispatches this bit to SGX status, separate from display events. Sources: [PSYS:77-94](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:1196-1282](poulsbo-data/../archaeology-data/PSYSC.txt); [LDRVH:91-111](poulsbo-data/../archaeology-data/LDRVH.txt); [LIRQ:198-230](poulsbo-data/../archaeology-data/LIRQ.txt).

**CONFIRMED [P3-031]** — Linux enables BIF_REQUESTER_FAULT and TWOD_COMPLETE. The handler reads INT_STAT/FAULT, logs type, requester, and address, then writes HOST_CLEAR/2; installs shared IRQ. Sources: [LIRQ:151-196](poulsbo-data/../archaeology-data/LIRQ.txt); [LIRQ:278-325](poulsbo-data/../archaeology-data/LIRQ.txt).

**CONFIRMED [P3-032]** — In DDK 1.14, SGX_ISRHandler recognizes SW_EVENT; the MISR can call recovery. The DRM_EXT integration exports SYSPVRServiceSGXInterrupt, calls the device ISR, and schedules MISR when handled. Sources: [INIT:1945-2008](poulsbo-data/INIT.txt); [INIT:2097-2125](poulsbo-data/INIT.txt); [PSYSC:1982-2005](poulsbo-data/../archaeology-data/PSYSC.txt).

## Responsibility chain

**INFERRED, from P3-030–032:** requestor SGX → SGX event state → THALIA Intel aggregation → PCI IRQ → ISR/MISR. The vblank interrupt belongs to the display chain; it does not prove CCB progress or USE execution.

**UNKNOWN:** required order of ack/mask under persistent fault in the real review; if a read has side effects on all power states; guarantee that HOST_CLEAR removes the cause of the BIF fault; safe recovery sequence with a job in flight. The current handler prints and acknowledges the event; it does not by itself provide isolation and recovery of 3D jobs.

Do not enable SW_EVENT or induce a page fault just to test an interrupt. For a future phase, start with logs already produced by the device's proprietary driver. Any active IRQ test requires exclusive ownership and a restoration plan for the mask shared with the display.
