# MultiDexed ![](https://github.com/probonopd/MultiDexed/actions/workflows/main.yml/badge.svg)

![MultiDexed](https://user-images.githubusercontent.com/2480569/222845457-eff2f74f-9699-4c49-bbec-8e7f58b7d14b.jpg)

MultiDexed ist a standalone application and a VST plugin for Windows, macOS, and Linux that runs multiple instances of [Dexed](https://github.com/asb2m10/dexed) to create the unison effect explained [here](https://www.youtube.com/watch?v=Hzwvd8aZUUU) and showcased [here](https://www.youtube.com/watch?v=BjxuzeVXCDU&t=1190s) by Anders Enger Jensen.

MultiDexed is especially useful in DAWs with a limited number of tracks, such as Ableton Live Lite.

__Currently it is not fully functional yet.__ In the meantime, with https://gitlab.com/kushview/element/ you can do similar things, albeit a bit more manually.

- [x] Make it build on GitHub Actions for Windows, macOS, and Linux
- [x] Do not crash when loaded into REAPER for Windows (running on FreeBSD with Proton WINE)
- [x] Load Dexed VST and create multiple instances of it
- [x] Make the instances produce sound (thanks [__@getdunne__](https://github.com/getdunne))
- [x] Each instance is slightly detuned
- [x] Each instance is stereo shifted (panned)
- [ ] Add GUI for selecting number of instances, amount of detune, and amount of panning
- [ ] Save and restore plugin state
- [ ] Make it build for Linux on Raspberry Pi (aarch64)
- [ ] Make it build on CirrusCI for FreeBSD
- [ ] Stretch goal: Make it read, write, and use [MiniDexed](https://github.com/probonopd/MultiDexed) performance files
