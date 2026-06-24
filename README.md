# Main Levels Config

![—](https://api.geode-sdk.org/v1/mods/lil2kki.main-levels-config/status_badge?stat=downloads) ![—](https://api.geode-sdk.org/v1/mods/lil2kki.main-levels-config/status_badge?stat=version) ![—](https://api.geode-sdk.org/v1/mods/lil2kki.main-levels-config/status_badge?stat=geode_version) ![—](https://api.geode-sdk.org/v1/mods/lil2kki.main-levels-config/status_badge?stat=gd_version) 

Edit main levels at runtime based on config.

So.. watch [config dir](https://docs.geode-sdk.org/functions/geode/dirs/getModConfigDir) yea. Or basic [tutorial video](https://youtu.be/VTVOser006E).

## Tools i think you would needy for:

Useful tools by this mod and reused **Level Edit** hack from <cb>Eclipse</c> allows you to edit main levels, from me goes full config saving support:

<mod:omgrod.rods-fangame-tools>

## Custom audio assets

The mod allows you to override/add game's audio assets by placing your own files inside the mod's **config directory**.  
Supported paths (relative to config dir):

- **Audio**: `audio/<id>.mp3` or `audio.<id>.mp3`
- **SFX**: `sfx/<id>.ogg` or `sfx.<id>.ogg`
- **Songs**: `songs/<id>.<ext>` or `song.<id>.<ext>`

The extension (`.mp3` or `.ogg`) must match the original file's extension used by the game.  
The mod checks these custom locations **before** falling back to default game paths.

--- 

#### <cr>The mod uses Cocos2d's file system. Config dir is NOT the only place - any path added to search priorities works. 
`Resource packs, geode resources, mod resources root`</c>

---

some more ai generated info that i was too lazy to write myself:

## Level configuration

Each main level can have its own config file: `levels/<id>.object.ini`

**Supported fields** (section `[GJGameLevel]`):
- `m_levelName` - level display name
- `m_difficulty` - 0=Auto,1=Easy,2=Normal,3=Hard,4=Harder,5=Insane,6=Demon
- `m_stars` - star count
- `m_audioTrack` / `m_songID` - audio track ID
- `m_songIDs`, `m_sfxIDs` - lists of custom song/sfx IDs
- `m_twoPlayerMode` - true/false
- `m_capacityString` - capacity string

Level string is stored in `levels/<id>.string.txt` (auto-saved when editing in editor).

## Level listing

`levels/_list.txt` controls LevelSelectLayer pages format: `1,2,3` or ranges `1:5,7,9:12`

Edit it via **Geode settings button** (gear icon in mod popup) - opens real-time list editor.

## Editor integration

When editing a main level:
- **Gear button** opens config popup (name, difficulty, stars, track)
- **Coin button** converts all User Coins -> Secret Coins with auto-numbering
- **Trash button** resets level to default or removes from listing

## Custom difficulty icons

Place `diffIcon_<ID>_btn_001.png` anywhere in search paths to override difficulty sprite.

## Misc

- Level position is remembered when entering Secret Door / Tower
- `verifyLevelIntegrity` is bypassed for main levels
