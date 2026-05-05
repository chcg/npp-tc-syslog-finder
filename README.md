# TC Syslog Finder
![Release](https://img.shields.io/github/v/release/bsagarzazu/npp-tc-syslog-finder)
![Downloads](https://img.shields.io/github/downloads/bsagarzazu/npp-tc-syslog-finder/total)
![License](https://img.shields.io/github/license/bsagarzazu/npp-tc-syslog-finder)

A Notepad++ plugin to open the latest Teamcenter syslog instantly. 

### Why?
Every time you start a new Teamcenter session —which happens a lot during debugging— you need to find and open a different syslog file. Whether you check it in Teamcenter's *Help > About > Log File* or keep the syslog folder open, it feels like a tedious waste of time. I wanted to make my daily work more efficient.

This plugin replaces that whole process with a single shortcut.

### Shortcuts
* `Alt + Shift + L`: Open the latest syslog at the end of the file, with monitoring mode activated.
* `Alt + Shift + F`: Open the logs folder in Windows Explorer.
* `Alt + Shift + C`: Delete syslogs older than 7 days.

### Installation

#### Plugins Admin (Recommended)
TC Syslog Finder is an official Notepad++ plugin since `v8.9.5`. You can install it directly from the editor:
1. Open Notepad++.
2. Go to **Plugins > Plugins Admin...**
3. Search for **TC Syslog Finder**
4. Check it and click **Install**.

#### Manual Installation
1. Download `TCSyslogFinder.dll` from [Releases](https://github.com/bsagarzazu/npp-tc-syslog-finder/releases/).
2. Create the folder: `C:\Program Files\Notepad++\plugins\TCSyslogFinder\`
3. Drop the DLL inside and restart Notepad++.

### Configuration
Go to **Plugins > TC Syslog Finder > Set TC Syslog Path** and select your logs directory once. Use **Edit Settings** to change the search pattern if needed (default is `tcserver*.syslog`).

---
License: GPL-2.0
