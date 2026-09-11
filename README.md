# GeoLock

A small [Geode](https://geode-sdk.org/) mod for Geometry Dash.

Open any online level, press the lock button, and confirm. Once locked, it is the only level you can play until you beat it.

## How it works

1. Open any online level's page. A lock button is added in the bottom-left
   area, next to the existing side buttons.
2. Press it and confirm the single popup: **Are you sure you want to lock in?**
   (`Yes` / `No`).
3. Pressing `Yes` stores that level (id + name) as the locked level. The lock
   survives restarts because it is stored in the mod's saved values.
4. While a lock is active, every attempt to start a different level is blocked
   with a single message:

   > Go back to "level_name" (ID: 123456).

   This covers online levels, saved/downloaded levels, creator/editor levels,
   official RobTop levels and every other normal way of entering gameplay.
5. The locked level itself plays normally. As soon as it is completed (outside
   practice mode), the lock is cleared automatically and silently.

There is no manual unlock button. The only ways out of a lock are to beat the
locked level, disable the mod, or uninstall it.

## Building

Requires the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and the
Geode CLI, with the `GEODE_SDK` environment variable pointing at your SDK
install.

```sh
geode build
```

The packaged mod is written to `build/skrypnykmark.geolock.geode`. That file can
be attached directly to a GitHub Release.

## Project layout

```
GeoLock/
├── CMakeLists.txt
├── mod.json
├── about.md
├── logo.png
├── resources/
│   └── lock.png
├── src/
│   └── main.cpp
├── README.md
├── LICENSE
└── .gitignore
```

## License

[MIT](LICENSE)
