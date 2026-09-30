# whotool — Who is logged on?

`whotool` is a Windows console tool that reports who is logged on to a machine,
either locally or through a network resource share, and can search the network
neighbourhood for the computer a specific user is currently logged on to.

Under the hood it combines three Windows mechanisms:

- **Local logons** are discovered by enumerating the per-user SID subkeys under
  `HKEY_USERS`. Each SID is resolved to a friendly `domain\user` name with
  `LookupAccountSidW`, and the logon time is read from the last-write time of the
  key's `Volatile Environment` subkey.
- **Resource-share logons** are enumerated with `NetSessionEnum`, which lists the
  sessions established by users connecting to the machine's shares.
- **Network search** is done with `NetServerEnum` to walk the list of NT servers
  and check each one for the requested user.

`whotool` provides 4 modes:

- **1 — All logons** — list local logons *and* resource-share sessions on this computer.
- **2 — Local logons only** — list only locally logged-on users on this computer.
- **3 — Specific computer** — list the local logons of a named remote computer.
- **4 — Find a user** — search the network for the computer a given user is logged on to.

All interaction is done through `std::cin` / `std::cout`. There are no command
line arguments, no flags, and no configuration files.

---

## Features

- Lists users logged on locally (from the `HKEY_USERS` registry hive).
- Lists users logged on via resource shares (`NetSessionEnum`).
- Searches the network for the computer a given user is logged on to (`NetServerEnum`).
- Shows the logon date and time, formatted in the current user's locale.
- Interactive, numbered menu with UTF-16 console output (Vietnamese / Unicode safe).

## Requirements

- Windows (developed and tested on Windows 10 / 11).
- A C++17 toolchain:
  - **MSVC** (Visual Studio 2019/2022), or
  - **MinGW-w64** (g++) with Ninja or GNU Make.
- CMake ≥ 3.15 (for Options A and B).
- Windows SDK libraries: `advapi32`, `netapi32`, `shell32` (and the `<LM.h>` NetAPI headers).
- To query a *remote* computer or search the network for a user, the **Remote
  Registry** service must be running on the remote machine, and the account must
  have the required permissions there.

## Build instructions

### Option A — CMake (MSVC)

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executable is produced at `build\Release\whotool.exe`.

### Option B — CMake (MinGW-w64 + Ninja)

```powershell
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=g++
cmake --build build
```

### Option C — MinGW-w64 (g++) directly

```powershell
g++ -std=c++17 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -O2 -o whotool.exe `
  main.cpp who.cpp utils.cpp -ladvapi32 -lnetapi32 -lshell32
```

## Usage

Run `whotool.exe`. A menu is shown:

```
==============================================
        whotool - Who is logged on?
==============================================
Select mode:
  1) All logons (local + network sessions) on this computer
  2) Local logons only on this computer
  3) Local logons on a specific computer
  4) Find which computer a user is logged on to
  0) exit
>
```

### Mode 1 — All logons

Lists local logons, then users logged on via resource shares, on this computer.

```
> 1
[*] Users logged on locally:

    MACHINE\alice   12/31/2026   08:14:03

[*] Users logged on via resource shares:

    MACHINE\bob     12/31/2026   09:00:10
```

### Mode 2 — Local logons only

Same as Mode 1 but without the resource-share section.

**Field notes**: local logon detection relies on the `Volatile Environment`
subkey under each user's SID key. Accounts whose profile has no such subkey are
not reported.

### Mode 3 — Specific computer

Prompts for a computer name and lists its local logons.

```
> 3
Computer name (e.g. \\SERVER): \\SERVER
Connecting to Registry of \\SERVER...
[*] Users logged on locally:
    ...
```

**Field notes**: enter the name with the leading `\\` (for example `\\SERVER`).
The Remote Registry service must be running on the target computer.

### Mode 4 — Find a user

Prompts for a username and searches the network for the computer that user is
logged on to.

```
> 4
Username: alice
Checking  SERVER1...
Checking  SERVER2...
    DOMAIN\alice   12/31/2026   08:14:03
```

**Field notes**: this enumerates the network neighbourhood and can be slow on
large networks. It requires network-browsing privileges; on a standalone
workstation it may report that the user is not logged on to any computer.

---

## Permissions and notes

- Reading `HKEY_USERS` requires at least standard user privileges for the local
  machine; querying another computer requires administrative access there.
- Network browsing (`NetServerEnum` / `NetSessionEnum`) needs the NetAPI service
  and appropriate network permissions.
- On a remote target, the **Remote Registry** service must be enabled.
- The tool uses wide (UTF-16) console I/O, so Unicode account names and
  Vietnamese text are rendered correctly in the console.
- Responsible use only. Enumerate logon sessions only on systems you own or are
  authorised to inspect.

## Project layout

- `main.cpp` — interactive menu, wide console setup, and dispatch to each mode.
- `who.cpp` — the logon enumeration logic (registry, `NetSessionEnum`, `NetServerEnum`).
- `who.h` — declarations for the logon enumeration functions.
- `utils.cpp` / `utils.h` — console output helpers (`print_line`, `checkError`).
- `CMakeLists.txt` — CMake build definition (C++17, links `advapi32 netapi32 shell32`).
- `README.md` — this file.
