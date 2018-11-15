DS-JSON
========
DS-PROTOTYP JSON DE-SERIALIZATION LIBRARY
-------------------------------------
<sup>And some other funky conversions like base-64 and Latin-1 to/from UTF8</sup>

**Resource optimized for small 8-bit and 32-bit microcontrollers.**

**Authors:**
* D. Taylor 2018 (gmail: senseitg)

**License:**
* Ask and thou shalt receive

**Features:**
* Parses and validates JSON
* No dynamic memory allocation
* Delivers decoded data via callback
* Handles JSON in RAM as well as streaming JSON
* Fully compliant and well tested

**Options (*.h):**
* 
* Configurable max nesting depth
* Configurable standards-breaking optimizations
* Configurable data sizes for number representation

**Resource requirements:**
* Minimal memory requirements (Not 32 bytes of RAM)
* Minimal stack use (non-recursive, few local variables)
* Minimal program space (compact, lean codebase)
* Minimal heap use (none, actually)

**Notes:**
* Designed for high speed, low foot print - not a rich feature set.

**Caveats:**
* Requires C99 (-std=c99 for GCC)

**Performance:**
* Pretty darn good. You get to do the comparisons!
