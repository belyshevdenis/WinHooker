# WinHooker

The simple package for hooking processes on Windows. Solution compiles with Visual Studio 2019. It uses external components:
1. mhook: https://github.com/martona/mhook
2. plog: https://github.com/SergiusTheBest/plog

## Usage

1. Add desired hooks to the `hook` project. See `hook/hook.cpp` with example.
2. Build `hook.dll` from `hook` project.
3. Build `injector.exe` from `injector` project.
4. Run `injector.exe` with parameters:
```
injector <dll> <target>
```
Where `target` is the name or PID of the target process and `dll` is the full path to dll with hooks.
_Note_: if the `target` is specified as process name, the tool finds __all__ processes with this name and injects `dll` to __all__ processes found.
5. Have fun :)

## Logs

You can enable logging in hooks by setting the environment variable `LOG_DIR`. if this variable is set, the `hook.dll` will write logs to the `%LOG_DIR%\<target>.log` file. Refer to `hook/hook.cpp` to see an axample.
