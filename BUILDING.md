# Building

Builds are produced using on GitHub Actions. This page describes how to set up local development environments.

To see the general build steps and dependencies, have a look at [the GitHub Actions workflows](../../tree/main/.github/workflows).

## Windows

Since I normally don't run Windows, here is a way to do it on FreeBSD using VirtualBox.
Using this approach requires a powerful recent computer with "lots" (16 GB) of RAM and even more fast SSD space.

* Download https://developer.microsoft.com/en-us/windows/downloads/virtual-machines/ (22 GB), it is free, requires no login, comes with Visual Studio Community 2022 which includes a C++ compiler
* Unzip (another 22 GB)
* Import into VirtualBox (another 22 GB)
* Set RAM to 12 GB and increase CPU cores
* First thing before booting, make a snapshot in VirtualBox
* Add an optical drive to the VM so that you can install Guest Extensions
* Boot Windows (first boot takes ~20 minutes)
* Install Guest Extensions (6.1.36 is what comes with the VirtualBox on FreeBSD 13.1-RELEASE with 2023Q1 packages)
* Reboot
* **Why can't I resize the screen?** 
* Shut down
* Make another snapshot
* Download and install JUCE to `C:\JUCE`
* Open Visual Studio and clone this repository
* Open Projucer
* Create a VisualStudio build configuration
* Double click the respective `*.vcxproj` and open with Visual Studio Community 2022
* Press F7 to build
* It compiles straight away without a hitch
* Running the standalone version seems to crash, as a window is never shown
* Running it with the Visual Studio Community 2022 debugger to see in the code crashes occur

## Linux

Section to be written

## macOS

Section to be written
