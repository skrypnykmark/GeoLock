# GeoLock

A small [Geode](https://geode-sdk.org/) mod for Geometry Dash.

Open any online or created level, press the lock button, and confirm. Once locked, it is the only level you can play until you beat it.

## How it works

1. Open any online or created level's page. A lock button appears in the
   bottom-left, next to the existing side buttons.
2. Press it and confirm the single popup: **Are you sure you want to lock in?**
   (`Yes` / `No`).
3. Pressing `Yes` stores that level (id + name) as the locked level. The lock
   survives restarts because it is stored in the mod's saved values.
4. While a lock is active, the locked level keeps its normal play button. Every
   other online or created level shows the lock instead of play, and any attempt
   to start it is blocked with a single message:

   > Go back to "level_name" (ID: 123456).

   This covers online levels, saved/downloaded levels, creator/editor levels,
   official RobTop levels and every other normal way of entering gameplay.
5. The locked level itself plays normally. As soon as it is completed (outside
   practice mode), the lock is cleared automatically, silently restoring the
   play button everywhere.

There is no manual unlock button. The only ways out of a lock are to beat the
locked level, disable the mod, or uninstall it.

## Dependencies

- [`geode.node-ids`](https://github.com/geode-sdk/NodeIDs)
- [`cvolton.level-id-api`](https://github.com/Cvolton/level-id-api-geode)

## Building

Requires the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and the
Geode CLI, with the `GEODE_SDK` environment variable pointing at your SDK
install.

```sh
geode build
```

The packaged mod is written to `build/skrypnykmark.geolock.geode`.

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
