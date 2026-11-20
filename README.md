# Debugging
Here is what you need for enabling debugging mode in Zephyr:
1. Installing OpenOCD compatible vendor
2. Path management (there are multiple toolchain, SDK, and OpenOCD paths to wire together)
3. Running scripts (`tasks.json` and `launch.json`)

---

### 1. Installing OpenOCD & USB Permissions
For this example, we will use **espressif-openocd** (`openocd-esp32`), targeting an **ESP32-C3** use case.

1. **Install `openocd-esp32` and store it in a dedicated directory for VS Code:**
   Create a `~/tools` directory, download the latest Linux `x86_64` release archive from Espressif, and extract it:
   ```bash
   mkdir -p ~/tools && cd ~/tools

   curl -s [https://api.github.com/repos/espressif/openocd-esp32/releases/latest](https://api.github.com/repos/espressif/openocd-esp32/releases/latest) \
     | grep "browser_download_url.*linux-amd64.*\.tar\.gz" \
     | cut -d '"' -f 4 \
     | xargs wget -O openocd-esp32.tar.gz

   tar -xzf openocd-esp32.tar.gz && rm openocd-esp32.tar.gz
   ```

2. **Verify installation:**
   Check using this:
   ```bash
   ~/tools/openocd-esp32/bin/openocd --version
   ```

3. **Grant USB-JTAG permissions (Linux `udev`):**
   Allow non-root access to the ESP32-C3 built-in USB-JTAG interface (`VID 0x303a / PID 0x1001`):
   ```bash
   echo 'ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", MODE="0666", GROUP="plugdev"' \
     | sudo tee /etc/udev/rules.d/99-esp32c3-jtag.rules
   sudo udevadm control --reload-rules && sudo udevadm trigger
   ```

---

### 2. Path Management & Firmware Prep
Before writing the VS Code scripts, collect the exact paths on your machine and prepare your Zephyr config:

| Purpose | Example Path / Setting |
| :--- | :--- |
| **OpenOCD Binary** | `/home/izasoerya/tools/openocd-esp32/bin/openocd` |
| **OpenOCD Scripts** | `/home/izasoerya/tools/openocd-esp32/share/openocd/scripts` |
| **Board Config** | `board/esp32c3-builtin.cfg` (uses native USB-JTAG on `GPIO18`/`GPIO19`) |
| **RISC-V GDB** | `/home/izasoerya/zephyr-sdk-1.0.1/gnu/riscv64-zephyr-elf/bin/riscv64-zephyr-elf-gdb` |
| **Compiled ELF** | `${workspaceFolder}/build/zephyr/zephyr.elf` |

* **Update `prj.conf` for debugging:** Disable compiler optimizations (`-O0`) so local variables remain readable in RAM and functions are not inlined:
  ```ini
  # Debugger Config
  CONFIG_DEBUG_THREAD_INFO=y
  CONFIG_NO_OPTIMIZATIONS=y
  ```

---

### 3. Create the Scripts
The automation scripts live inside `.vscode/`, wrapped inside **`tasks.json`** (for auto build & flash before debugging), **`launch.json`** (for calling GDB and configuring the ESP32-C3 hardware breakpoints and memory map), and **`.gdbinit`** (for convert all breakpoint to hw-breakpoint).

`.gdbinit` (Auto Build & Flash)
Create `.vscode/tasks.json` to convert all breakpoint to hw-breakpoint (using normal breakpoint won't work somehow):
``` bash
# Mark ESP32-C3 XIP Flash & DROM as Read-Only so GDB forces hardware breakpoints
mem 0x42000000 0x43000000 ro
mem 0x3C000000 0x3C800000 ro
mem 0x3FC80000 0x3FCE0000 rw
set breakpoint auto-hw on
```

#### `.vscode/tasks.json` (Auto Build & Flash)
Create `.vscode/tasks.json` so pressing `F5` automatically compiles the latest changes and flashes the board before GDB attaches:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "West Build Blinky",
            "type": "shell",
            "command": "/home/izasoerya/zephyrproject/.venv/bin/west build",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

#### `.vscode/launch.json` (GDB & Target Setup)
Create `.vscode/launch.json` using `cppdbg`. Placing target commands in `postRemoteConnectCommands` ensures they run **after** GDB attaches to `localhost:3333`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug ESP32-C3 (cppdbg)",
            "type": "cppdbg",
            "request": "launch",
            "program": "/home/izasoerya/zephyrproject/build/zephyr/zephyr.elf",
            "args": [],
            "stopAtEntry": false,
            "cwd": "/home/izasoerya/zephyrproject",
            "MIMode": "gdb",
            "miDebuggerPath": "/home/izasoerya/zephyr-sdk-1.0.1/gnu/riscv64-zephyr-elf/bin/riscv64-zephyr-elf-gdb",
            "miDebuggerArgs": "-x /home/izasoerya/zephyrproject/projects/blinky/.gdbinit",
            "miDebuggerServerAddress": "localhost:3333",
            "setupCommands": [
                {
                    "description": "Enable pretty printing",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "postRemoteConnectCommands": [
                {
                    "description": "Disable FreeRTOS thread detection for Zephyr",
                    "text": "monitor esp32c3.cpu0 configure -rtos none",
                    "ignoreFailures": true
                },
                {
                    "description": "Reset and halt ESP32-C3",
                    "text": "monitor reset halt",
                    "ignoreFailures": false
                },
                {
                    "description": "Wipe stale hardware triggers from ESP32-C3 registers",
                    "text": "monitor rbp all",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```
*(Note: If you want to attach to already-flashed firmware without rebuilding/flashing on every `F5`, remove the `"preLaunchTask"` line.)*

---

### 4. Running a Debug Session

1. **Start the OpenOCD GDB server** in a terminal:
   ```bash
   pkill -9 openocd && west debugserver \
     --openocd /home/izasoerya/tools/openocd-esp32/bin/openocd \
     --openocd-search /home/izasoerya/tools/openocd-esp32/share/openocd/scripts \
     --config board/esp32c3-builtin.cfg
   ```
2. **Keep active VS Code breakpoints to 1–2 maximum** (and do not place a red dot on the first line of `main()`, since `hbreak main` already halts there). This preserves free hardware triggers so `Step Over` (`F10`) works cleanly.
3. **Press `F5`** in VS Code to build, flash, connect to `localhost:3333`, and halt at `main()`.