# GeoLock

A small [Geode](https://geode-sdk.org/) mod for Geometry Dash.

Open an online or created level, press the lock button, and confirm. Play the
locked level and an optional practice copy until you beat the original.

## How it works

1. Open an online or created level's page. Look for the lock button near the
   gear.
2. Press the lock button and choose **Yes** in the confirmation popup.
3. Pressing `Yes` stores that level as the active lock. The lock survives
   restarts and carries over from earlier versions of GeoLock. A brief lock-in
   animation plays when you create a new lock.
4. While a lock is active, the target and its attached practice copy keep their
   normal play buttons. Other online and created levels show the lock instead
   of play. Blocked launches show the target's name and ID. Press **OK** to
   open the Lock Room.

   The launch checks cover online and saved levels, created levels and editor
   playtests, official levels, Tower floors, and secret levels.
5. A normal-mode clear on the original target releases the lock and restores
   the play buttons.

There is no manual unlock button. The only ways out of a lock are to beat the
locked level, disable the mod, or uninstall it.

## Practice copies

Press the lock icon on the target or practice copy to open the Lock Room, then
select **Choose Practice**. Pick an existing level from **My Levels**, such as
a StartPos copy. You can play or editor-playtest that copy during the lock.

The attached level is highlighted in green. Select another row to replace it or
select the green row again to detach it. The attachment survives renames and
restarts, and the picker scrolls to it when you reopen it. If you delete the
attached level, the picker shows it as missing and lets you choose another.

## Lock Room

Press the lock icon in the top-right of the main menu to see your active target,
when you locked in, and your progress.

- **Starting Best** is your Normal Mode record when you locked the target.
- **Best Since Lock** is your best recorded Normal Mode run on the target during
  this lock.
- **Locked Attempts** counts gameplay attempts on the target and its attached
  practice copy in Normal and Practice Mode.

**Open Target** and **Open Practice** take you to those level pages. If you
open the Lock Room from one of those pages, it offers the other destination
when available. When an online target needs a download, you can retry from the
Lock Room.

Progress tracking starts with this version. Older locks keep their target and
practice copy and show **N/A** for Starting Best. Platformer locks track attempts.

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
│   ├── lock-in.jpeg
│   └── lock.png
├── src/
│   ├── hooks/      # Geometry Dash entry points, gameplay, and downloads
│   ├── lock/       # Level identity, saved state, and lock rules
│   └── ui/         # Lock button, play-button artwork, and popups
├── README.md
├── LICENSE
└── .gitignore
```

## License

[MIT](LICENSE)
