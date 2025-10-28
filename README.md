# Logger 1.0

## Tested Compilers / IDE
- MSVC / Visual Studio 2026

## Properties
- C++23
- Modules-based
- Thread-safe
- Flexible
- Platform: Windows
- Uses `std::format`
- No dependencies

## Queue
- Each `Logger` class instance has its own `Queue` object as a member.
- A queue can be **Bound** or **Unbound**.
- The queue takes a **capacity** argument.

## Outputs
- Currently, three thread-safe output types are implemented 
- (class-based on `std::string` output type):
  - **File**
  - **Console**
  - **Debug** (calls `OutputDebugStringA` from the Windows API)

# Entries
- **Entries** are classes that inherit from the `Entry` base class.  
  These classes are used as Mixins types for the `Logger` class.
- Each `Entry` has a property specifying whether it will be forwarded  
  to the `Logger`'s `Queue` or output directly by the calling thread.
- Currently implemented entry types:
  - **ErrorEntry** class (`error` member function) — cross-platform  
  - **InfoEntry** class (`info` member function) — cross-platform  
  - **LinesEntry** class (`lines` member function) — cross-platform  
  - **SequenceEntry** class (`sequence` member function) — cross-platform  
  - **SuccessEntry** class (`success` member function) — cross-platform  
  - **TraceEntry** class (`trace` member function) — cross-platform  
  - **WarningEntry** class (`warning` member function) — cross-platform  
  - **BytesEntry** class (`bytes` member function) — implemented for Windows only

- You can create new `Entry` classes as needed.

# Fields
- **Fields** are classes used as Mixins types for the `Entry` classes.  
- Currently implemented field types:
  - **LocationField** — cross-platform  
  - **LoggerNameField** — cross-platform  
  - **MessageField** — cross-platform  
  - **ThreadIDField** — cross-platform  
  - **TimePointField** — cross-platform  
  - **TypeField** — cross-platform  
  - **ErrorCodeField** — implemented for Windows only

- You can create new `Field` classes as needed.

# Exception
- The Logger library uses the `Exception` class when throwing exceptions.
- **Cases where `Exception` can be thrown (caused by user code):**

  - Calling an `XEntry` class function that takes an `(auto&& message)` argument  
    with a `nullptr const char*` value.

  - Calling the `BytesEntry` class function that takes a `(const void* address)` argument  
    with a `nullptr` address.

  - An exception will be thrown if you try to create more than one instance of  
    `ConsoleStringOutput`. Only a single instance can exist at a time.  
    You may destroy the current instance and create another later,  
    but only one can be active simultaneously.

  - An exception will be thrown if you try to create more than one instance of  
    `DebugStringOutput`. Only a single instance can exist at a time.  
    You may destroy the current instance and create another later,  
    but only one can be active simultaneously.

  - Multiple instances of `FileStringOutput` can be created freely.
