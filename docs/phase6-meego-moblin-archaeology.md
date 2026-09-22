# MeeGo and Moblin archaeology

## Scope

I searched public MeeGo/Moblin package and source traces for Poulsbo, PSB, GMA500, SGX, SGX535, PowerVR, `8108`, clocks, clock gating, power, reset, suspend, and resume. This was a search for historically relevant platform code, not evidence that such code must exist in MeeGo or Moblin.

## Findings

Public contemporaneous reporting and maintainer/package traces establish that Poulsbo support involved a historical PSB/PowerVR software stack and, at different times, binary components. They are useful locators for package names and source lineage. They do not expose a Poulsbo SGX register-access, power, or reset contract. The searchable public material also includes later Cedar Trail and Moorestown packaging and patches; those target different platforms and are not SGX535/Poulsbo evidence.

The recovered local historical PSB sources remain the useful directly inspectable code path. They show driver operations and software abstractions such as `OSPM_GRAPHICS_ISLAND`, but not the physical definition of that island or a precondition for an SGX identification read. The 2011 gma500 power-management cleanup shows that these abstractions changed over time; it cannot establish the physical state of the Dell target.

## Result

**CONFIRMED:** MeeGo/Moblin are historically relevant search domains for Poulsbo deployment and driver packaging.

**UNKNOWN:** a public MeeGo or Moblin artifact defining the SGX535 clock source, enable order, power island, reset state, or read-failure behavior. No such contract was recovered. No MeeGo/Moblin clue changes a Phase 5 gate field.

## Provenance limits

Public community posts and contemporary reporting are secondary evidence. They were used only to locate historical package/source trails and to delimit platform scope. They are not promoted to hardware documentation.
