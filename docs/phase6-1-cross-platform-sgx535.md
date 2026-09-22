# Cross-platform SGX535 semantics pass

The pass deliberately separated common naming from a transferable access contract.

| source family | finding | quality | permitted conclusion | prohibited conclusion |
|---|---|---|---|---|
| checked-in SGX535 headers | `EUR_CR_CORE_ID=0x0010`, `EUR_CR_CORE_REVISION=0x0014`, with field masks | PRIMARY-INDIRECT | matches the existing SGX535/Poulsbo software identity evidence | RO, no-side-effect, power, clock, reset, or failure semantics |
| Android/OMAP Series5 headers | nearby Series5 cores expose similarly named ID/revision fields but offsets vary across core definitions | PRIMARY-INDIRECT | names and field layouts are not unique to Poulsbo | equal names/fields imply equal hardware contract |
| 2023 Series5 DT discussion | maintainers discuss the difficulty of determining revision before initialization and whether a read after firmware could reflect firmware information | PRIMARY-INDIRECT; discussion, not a specification | contemporary maintainers did not cite a universal pre-init contract | any requirement becomes PASS |
| Imagination forum debug output | logs print a `EUR_CR_CORE_ID` value after an existing stack runs | PRIMARY-INDIRECT, non-Poulsbo | historical stacks read IDs in their own context | reads are safe in every state |

The varied Series5 offsets are active counterevidence against extrapolating SGX520/530/540/544/545 definitions to SGX535. Cross-platform material adds no explicit SGX535 read-only attribute and no positive proof of absent read side effects. It leaves all Poulsbo integration requirements unchanged.
