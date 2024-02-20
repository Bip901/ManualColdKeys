# ManualColdKeys

Poor man's AutoHotKey to bypass anti-cheat software.

## Usage

* Edit `Main.cpp` and insert your hotkeys (section marked with "DEFINE HOTKEYS HERE")
* Build and run
* To run it automatically when you log-on, create a symlink to the executable in `C:\Users\your_username\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup`

The configuration process is unfriendly on purpose. If you want good software that actually interprets a config file, use [AutoHotKey](https://github.com/AutoHotkey/AutoHotkey).

## Motivation

I'm a programmer. I like having Ctrl+Alt+T open a terminal.

Unlike Ubuntu, Windows doesn't have a built-in way to bind such hotkeys, so I use AutoHotKey.

Whenever I play video games with an overly-aggressive anti-cheat engine they immediatly flag my innocent usage of AutoHotKey as cheating.
