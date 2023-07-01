# Contributions

Thanks for considering contribution to Mono's music manager.


# Style

Momuma uses a style similar to the
[Linux kernel](https://www.kernel.org/doc/Documentation/process/coding-style.rst)
(8-wide tabs) with a few changes:

- Never omit braces for single statement `if`, `else`, `for`, `while`, `switch`, etc...
- The column limit is 100, with 80 being a recommended limit.
- Alignment is to be done using spaces.  
- Use `G_LIKELY` and `G_UNLIKELY` with `if` statements to better document the
code, not to improve performance.

In general, try to follow the style of existing/surrounding code.

Personal recommendation: make whitespace visible in your editor of choice.

## Variable, Function and Class/Struct names
**Functions** (and methods) will be named using `snake_case`, **classes**/**structs** will be named using `PascalCase`,  
and **variables** will be named using `camelCase`. Class member variables will contain the following prefixes:

- `_<public member>`
- `m_<private member>`
- `d_<dependency-injected member>`


# Internals

## Logging

All logging and printing is done using the `spdlog` library. Do *not* use `cout`, `cerr`, `printf` or `fprintf`.

Do *NOT* use `#include <spdlog/spldog.h>`, instead use `#include "momuma/spdlog.h"`.
Do *NOT* use `#include <sigc++/signal.h>`, instead use `#include "momuma/sigc.h"`.

## Namespaces

Other than a few existing exceptions, do not use `namespace thing = Namespace::thing;`
in global scopes or in header files.

The only exceptions are `std::chrono` and `std::filesystem`, which were renamed to
`chrono` and `fs` respectively inside of `momuma/spdlog.h` (more such namespaces are unlikely to be added).

## Strings

Avoid usage of `std::string`, and stick to `Glib::ustring`.
`fs::path` (`std::filesystem::path`) may only be used when working with file paths.

- To instansiate a `fs::path` from a `Glib::ustring`, use the `Glib::ustring::raw()` instance method.
- To instansiate a `Glib::ustring` from a `fs::path`, use the `fs::path::string()` instance method.
