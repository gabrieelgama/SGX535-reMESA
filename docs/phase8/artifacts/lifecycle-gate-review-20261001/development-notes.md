# Local verification development notes

Initial expanded test compilation failed strict GCC dangling-pointer diagnostics because the test PCI fixture's local address was retained in a static test boundary. Fixtures were given static lifetime; no production source was altered. Focused and full final builds pass without warning.

Initial provenance helper selected fixture `Makefile` instead of its recorded filename `original-Makefile`, and stopped with FileNotFoundError before patch replay. Corrected helper uses the authoritative fixture name; full strict replay and hash checks pass. Neither local test-development error caused a candidate build, patch change, or target contact.
