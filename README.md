# MultiDexed ![](https://github.com/probonopd/MultiDexed/actions/workflows/main.yml/badge.svg)

![MultiDexed](https://user-images.githubusercontent.com/2480569/222845457-eff2f74f-9699-4c49-bbec-8e7f58b7d14b.jpg)

MultiDexed is a standalone application and a VST plugin for Windows, macOS, and Linux that runs multiple instances of [Dexed](https://github.com/asb2m10/dexed) to create the unison effect explained [here](https://www.youtube.com/watch?v=Hzwvd8aZUUU) and showcased [here](https://youtu.be/TutoLkJ_bks?t=718) by Anders Enger Jensen.

The instances can be detuned and stereo panned.

MultiDexed is especially useful in DAWs with a limited number of tracks, such as Ableton Live Lite.

__This is work in progress.__ Any help is greatly appreciated.

- [x] Make it build on GitHub Actions for Windows, macOS, and Linux
- [x] Do not crash when loaded into REAPER for Windows (running on FreeBSD with Proton WINE)
- [x] Load Dexed VST and create multiple instances of it
- [x] Make the instances produce sound (thanks [__@getdunne__](https://github.com/getdunne))
- [x] Each instance is slightly detuned
- [x] Each instance is stereo shifted (panned)
- [x] Add GUI for selecting amount of detune, and amount of stereo panning
- [ ] Add GUI for selecting number of instances
- [ ] Save and restore plugin state
- [ ] Make it build for Linux on Raspberry Pi (aarch64)
- [ ] Make it build on CirrusCI for FreeBSD
- [ ] Stretch goal: Make it read, write, and use [MiniDexed](https://github.com/probonopd/MultiDexed) performance files and/or TX816, TX802 performances

__NOTE:__ A Dexed version newer than 0.9.6 needs to be installed (e.g., the NIGHTLY version from the Dexed GitHub page). Dexed 0.9.6 and earlier are based on JUCE 6 which seemingly leads to crashes when being hosted in the MultiDexed vst3.
