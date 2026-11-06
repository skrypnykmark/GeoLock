# GeoLock

A small [Geode](https://geode-sdk.org/) mod for Geometry Dash.

Open an online or created level, press the lock button, and confirm. Play the
locked level and an optional practice copy until you beat the original.

## How it works

1. Open any online or created level's page. A lock button appears in the
   bottom-left, next to the existing side buttons.
2. Press it and confirm the single popup: **Are you sure you want to lock in?**
   (`Yes` / `No`).
3. Pressing `Yes` stores that level as the active lock. The lock survives
   restarts and carries over from earlier versions of GeoLock.
4. While a lock is active, the target and its attached practice copy keep their
   normal play buttons. Other online and created levels show the lock instead
   of play. Blocked launches show a single message:

   > Go back to "level_name" (ID: 123456).

   This covers online levels, saved/downloaded levels, creator/editor levels,
   official RobTop levels and every other normal way of entering gameplay.
5. A normal-mode clear on the original target releases the lock and restores
   the play buttons.

There is no manual unlock button. The only ways out of a lock are to beat the
locked level, disable the mod, or uninstall it.

## Practice copies

On the locked level's page, press the copy icon where the lock button was.
Choose one existing level from **My Levels**, such as a StartPos copy. You can
play or editor-playtest that copy during the lock. Only the original target can
release the lock.

The attached level is highlighted in green. Select another row to replace it or
select the green row again to detach it. The attachment survives renames and
restarts, and the picker scrolls to it when you reopen it. If you delete the
attached level, the picker shows it as missing and lets you choose another.

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
│   ├── hooks/      # Geometry Dash entry points and completion events
│   ├── lock/       # Level identity, saved state, and lock rules
│   └── ui/         # Lock button, play-button artwork, and popups
├── README.md
├── LICENSE
└── .gitignore
```

## License

[MIT](LICENSE)
