# MultiDexed

![MultiDexed](https://user-images.githubusercontent.com/2480569/222845457-eff2f74f-9699-4c49-bbec-8e7f58b7d14b.jpg)

MultiDexed ist a standalone application and a VST plugin for Windows, macOS, and Linux that runs multiple instances of [Dexed](https://github.com/asb2m10/dexed) to create the unison effect demonstrated [here](https://www.youtube.com/watch?v=Hzwvd8aZUUU).

MultiDexed is especially useful in DAWs with a limited number of tracks, such as Ableton Live Lite.

__Currently it is not functional yet. Any help appreciated.__

- [x] Make it build on GitHub Actions for Windows, macOS, and Linux
- [ ] Make it build on CirrusCI for FreeBSD
- [ ] Load Dexed VST and create multiple instances of it
- [ ] Each instance can be slightly detuned
- [ ] Each instance can be stereo shifted (panned)
- [ ] Add GUI for selecting number of instances, amount of detune, and amount of panning
- [ ] Save and restore plugin state
- [ ] Stretch goal: Make it read, write, and use [MiniDexed](https://github.com/probonopd/MultiDexed) performance files
