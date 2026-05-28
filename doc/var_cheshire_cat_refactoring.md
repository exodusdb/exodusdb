# The Cheshire Cat Refactoring of `var`

**Date:** Ongoing (major work in 2026)  
**Author:** Steve Bush + Grok collaboration  
**Goal:** Remove strict aliasing / type punning UB from the `var` class hierarchy while preserving both OO and classic (free-function) APIs, and keeping the ability to split implementation across multiple files.

---

## The Original Problem

The `var` type had a deep inheritance chain:

```cpp
class var      : public var_db
class var_db   : public var_os
class var_os   : public var_stg
class var_stg  : public var_base
```

To allow methods implemented on the lower layers to return a "full-featured" `var` (one that could do string, OS, and DB operations), the design relied on four dangerous implicit conversion operators:

```cpp
// The "grin" — the last thing to disappear
operator       var()  const &  { return *reinterpret_cast<const var*>(this); }
operator const var&() const &  { return *reinterpret_cast<const var*>(this); }
operator       var&()       &  { return *reinterpret_cast<var*>(this); }
operator       var()        && { return *reinterpret_cast<const var*>(this); }
```

These were classic strict aliasing violations (base → derived via `reinterpret_cast`). They worked in practice for years (including at `-O2` in production), but were formally Undefined Behavior.

---

## The Refactoring Strategy

### Core Idea
Keep the **logical** separation of concerns (string handling, OS functions, DB functions, core numeric/string primitives) but eliminate the **type** layering that required the UB conversions.

### Technique Used
- Single `class var` that directly inherits only from `var_base`.
- Class-less versions of the old layer headers (`vars.h`, `varo.h`, `vard.h`) are `#include`d directly inside the body of `class var`.
- This injects all the member declarations into one class while still allowing the declarations to live in separate files for organization and compilation speed.

Current state (as of latest update):
```cpp
class var : public var_base {
    // ... common stuff ...

    #include "vard.h"   // DB members (class-less)
    #include "varo.h"   // OS members (class-less)
    #include "vars.h"   // String members (class-less)
};
```

---

## The Chicken-and-Egg Question (and the Answer)

A recurring point of confusion was:

> "How can `var_base` methods return `var` objects when `var` inherits from `var_base`? Doesn't the derived type need to be complete before the base can mention it?"

**Answer:** C++ allows a base class to *declare* member functions that return or take a derived type by value while the derived type is still incomplete. Completeness is only required at the point of *definition* (when the function body is compiled).

This is achieved through careful include ordering:

1. `var.h` includes `varb.h` early → `var_base` declarations are visible.
2. Later in `var.h`, `class var : public var_base { ... }` is defined (with the class-less layer includes).
3. `varb.cpp` includes `var.h` first → by the time the bodies of `var_base` methods are compiled, `var` is fully defined.

This is standard C++ and does **not** require templates, pointers, or references in the signatures. It is the same rule that lets a base class have a member function returning a derived type by value.

---

## The "Grin of the Cheshire Cat"

The dangerous conversions were poetically referred to as "the grin" — the last visible remnant of the old layered design after the three middle classes (`var_db`, `var_os`, `var_stg`) had been removed.

As of the latest work, those four conversion operators have been disabled/removed. The only remaining questionable casts are two manual `static_cast<var*>(static_cast<void*>(this))` in the prefix `operator++()` and `operator--()` implementations on the base (clearly marked as temporary).

---

## Current State (Summary)

- **UB removed?** Largely yes for the original problematic conversions.
- **API preserved?** Yes — both OO style and classic free-function style continue to work unchanged.
- **File separation?** Yes — string/OS/DB functionality declarations still live in separate headers that are included into `class var`.
- **Remaining debt:**
  - Data members still live in `var_base`.
  - Two manual casts remain in prefix increment/decrement.
  - Some methods are still implemented on `var_base` and rely on explicit `var(...)` construction when they need to return a rich object.
  - Performance on hot paths (especially math) should be re-measured.

---

## Open Questions (as of last discussion)

1. Should the remaining two manual casts in `++` / `--` be eliminated by moving those operators to be defined directly on `var`?
2. Should the data members eventually move out of `var_base` entirely so `var_base` can become pure implementation helpers?
3. Has performance on arithmetic hot paths been affected by the move from zero-cost reinterpret casts to explicit construction in some places?
4. Is the current include-inside-class technique the long-term shape, or is there a cleaner way to achieve the same file separation?

---

## References & Metaphors Used

- "The grin of the Cheshire cat was last to disappear" — referring to the final removal of the `reinterpret_cast` conversions.
- The chicken-and-egg problem of base/derived completeness and how C++ resolves it for member function declarations.

---

## Design Reality Discovered (Late Stage)

After attempting to make `var_base` fully independent (by changing prefix `++`/`--` on the base to return `var_base&` and exploring removing the `RETVAR`/`RETVARREF` macros), it became clear that:

> **The fundamental design contract of `var_base` is that it exists to provide dynamic typing and overloaded operators to a richer parent class (`var`). It is not intended to stand alone.**

Key observations:

- Virtually all public methods on `var_base` are declared to return `RETVAR` / `RETVARREF` (i.e. the rich `var` type).
- When the `RETVAR` / `RETVARREF` macros were temporarily changed to resolve to `var_base` types, the build broke in many places.
- The old upcasting mechanism (`operator var()` etc.) was the hidden way this "promotion" happened without the caller noticing.
- Even after removing the UB upcasts, the *design intent* remained: `var_base` operations should usually hand back a full `var`.

This was documented in `varb.h` approximately as:

```cpp
// The design philosophy of var_base is that it overloads all the basic operators
// (^ = concat) to provide a dynamic type to a parent class var.
// var_base is not designed to work by itself and all its public functions return a var.
```

This is an important constraint on future evolution:
- If a truly standalone `var_base` is ever desired, it will require a deliberate parallel API (separate macros, different return types, etc.) and will be a significant breaking change for anyone using the base layer directly.
- For the time being, `var_base` should be treated as an implementation substrate whose public surface is intentionally tied to the richer `var` type.

---

## Final Architectural Conclusion

After removing all three intermediate layers (`var_db`, `var_os`, `var_stg`), the following reality became clear:

**The lack of safe upcasting of temporaries with identical storage prevents traditional inheritance-based layering of disparate classes.**

Once the dangerous `reinterpret_cast` upcasts (the "grin") were removed, it was no longer practical to have `var` inherit from a chain of progressively richer base classes while still being able to return full-featured `var` objects from methods implemented on those bases.

The chosen long-term approach is:

- A single `class var` that directly inherits only from `var_base`.
- All additional functionality (string handling, OS operations, database operations) is provided by `#include`ing class-less header fragments directly inside the body of `class var`.
- This technique allows the code to remain physically split across multiple `.h` files for organization and compilation speed, while avoiding any need for unsafe base → derived upcasting.

This "classless include" composition model replaces the old inheritance hierarchy for everything above `var_base`.

`var_base` itself is retained as a foundational dynamic type (with its own design contract documented in `varb.h`), but it is understood that it is not intended to be used completely standalone for most operations.

---

*This document was created to preserve the key technical insights from a long, deep refactoring discussion.*