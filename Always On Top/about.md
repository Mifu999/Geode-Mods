# Always On Top

Adds a toggle button on the main menu that keeps the GD window on top of every other window on your desktop.

Useful when you don't want popups, alerts, Discord notifications or anything else covering the game during a run.

## How to use

A round button with a pin icon shows up in the bottom menu. First press opens a short info popup, after that it just toggles on/off. A small notification appears in the top-right each toggle (can be turned off in the settings).

## Platform

Windows only — uses Win32 `SetWindowPos` to mark the window as topmost.