# Swiss Train Display

A real-time Swiss train departure display with a 3-button interface, inspired by [Real-time BART Display](https://filbot.com/real-time-bart-display/).

## About

This is a **Python prototype** for testing and development. The final version will run on an **ESP32** microcontroller with a physical display and 3 hardware buttons.

## Features

- 🚆 Real-time train departure information using Swiss public transport API
- ⏱️ Multiple display presets (single route, multiple destinations, multiple routes, clock)
- 🔄 Auto-loop through presets
- 🎮 3-button navigation (Up, Down, OK) with long-press shortcuts
- ⌨️ On-screen keyboard for station configuration
- 🔢 Number picker for interval settings
- 💾 Smart caching to prevent API rate limiting
- 🔁 Configurable auto-refresh intervals

## Controls

### Short Press
- **u**: Up
- **d**: Down
- **o**: OK/Select

### Long Press (Auto Mode)
- **U**: Previous preset
- **D**: Next preset
- **O**: Quick config

## Requirements

```
requests
```

## Usage

```bash
python3 swiss_train_simple.py
```

## Configuration

All station names, routes, and destinations can be configured via the on-screen menu using the 3-button interface.

## API

Uses the [transport.opendata.ch](https://transport.opendata.ch/) API for real-time Swiss public transport data.

---

**Status**: Prototype/Testing Phase
**Target Platform**: ESP32 with physical display
