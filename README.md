# minwm
An ncurses based app switcher / launcher / window manager developed with the PinePhone in mind.

The name inspiration comes from the fact that the switching mechanism will revolve around the desktop analogy that when an app is selected, the minwm will be “minimized” and the app will display on the same “desktop”. Switching back to minwm from the app requires the current app to be “minimized” as well. Also, I’m following the inspiration behind SXMO and the “suckless” programs that tend to be minimalistic and do one job and do it well, so “min” is a play on that.

## All Team Member names 
Trent Wilson

## Project Overview: what will your project do? What function will it serve?
minwm will enhance the functionality of the SXMO UI. minwm will be an ncurses based app that lists the currently open gui apps. Selecting one will “switch to the app”. At first my idea was to interface with a window manager in order to implement the “switching” but after a lot of research and many different ideas I think it is feasible to manage child processes myself, and code directly with the xorg api to map and unmap programs as needed.

## Project Technologies: what do you expect to use to build it? Languages, libraries, etc 
Language: C
Compiler: Need to compile for aarch64
Emulator: QEMU with postmarketOS for testing
Libraries: Potentially Xlib or XCB as well as SXMO packages
Target device: PinePhone
I’ll use SXMO UI as a starting point. minwm will eventually replace its window manager dwm and also dmenu for navigation and new process launching. Most development wont actually depend on SXMO but to be able to use minwm on the PinePhone, other components will need to be present on the phone e.g. SXMO’s lisgd library converts swipe gestures to any commands (like accessible shortcuts basically). These will be useful once minwm is implemented to switch back to minwm from an open app. Also, the onscreen keyboard svkbd will be important for text input once on the phone.

## Related Work: what is already out there that is comparable to what you are proposing? (open source and commercial) 
Currently, SXMO uses dwm as a tiling window manager with multiple desktops. The user swipes left and right to go from desktop to desktop which can have different apps on them. This is a direct application of a normal desktop computer’s tiling window manager with touch added on top. The downside of this is that if the user is on desktop 1 and there are 10 active desktops, he has to swipe through 9 apps to get where he wants to. And there is no view to see a list of everything that’s open, so it’ll be easy to forget which desktop has what he’s looking for anyway. The idea of SXMO is brilliant. However, even a desktop that is considered very minimalistic on a traditional desktop is relatively complex when used on a device like a phone where the interaction with the device needs to be simpler. So minwm will reduce the features of the wm by 95% but will be more usable. (no need for tiling, split windows, virtual desktops, etc)

Other opensource phone Uis are Phosh which is GNOME’s attempt and Plasma which is KDE’s offering. Then of course there are Apple and Google’s official UI’s that we have been forced to use for the past decade.

## Three-week plan to get to a runnable pre-alpha version AKA Roadmap
1. DONE Get a handle on the ncurses API, and allow for user to input text and display the user's text on screen
2. build a scrollable list in the UI and populate it with open child processes and programs launchable from $PATH. 
3. Add a launch button to launch the selected program/process from the list. 
4. Add an advanced view button to also see background processes and add the ability to kill any selected process as well as refocus/launch

I think that's all doable by the three week mark. Then...

5. There will need to be some basic tiling component required. At a minimum I need to create a way for svkbd to tell minwm it has launched and minwm needs to give the bottom portion of the screen for the keyboard. And adjust back when the keyboard exits. And also decide how to deal with dialogue boxes.
6. Start interfacing with x11 and figure out how to map and unmap programs from the screen.
7. Once the fundamental app switching works, install lisgd library and test so we have a method of returning to minwm from a focused app
8. Start filling in the expected "Extended Window Manager Hints" requirements e.g. set "window" of unfocused or "minimized" app propert _NET_WM_STATE_HIDDEN set to true

ncurses resource:
http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/printw.html

Good explanation and view on how window managers work and what I want to do.
Virtual desktops section, implementation note talks about a compelling strategy.
https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm45805412385120

c library api for X11 that may be useful to get the map/unmap or max/min or whatever.
https://xcb.freedesktop.org/

## Issues and concerns 
How will I map and unmap incoming and outgoing active processes with x11?
Need to learn the available APIs more
