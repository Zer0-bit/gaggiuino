# Gaggiuino custom translations
Copy the complete `languages/` folder, including `fonts/`, choose a language under **Settings →
Display → Language** and tap **Save language**. The web interface follows it.

| Language | Selector name | Pack |
| --- | --- | --- |
| English | English | [en.json](en.json) |
| Simplified Chinese | 简体中文 | [zh-CN.json](zh-CN.json) |
| German | Deutsch | [de.json](de.json) |
| French | Français | [fr.json](fr.json) |
| Portuguese (Portugal) | Português | [pt-PT.json](pt-PT.json) |
| Romanian | Română | [ro.json](ro.json) |
| Spanish | Español | [es.json](es.json) |
| Swedish | Svenska | [sv.json](sv.json) |
| Greek | Ελληνικά | [el.json](el.json) |
| Thai | ไทย | [th.json](th.json) |
| Vietnamese | Tiếng Việt | [vi.json](vi.json) |
| Japanese | 日本語 | [ja.json](ja.json) |
| Korean | 한국어 | [ko.json](ko.json) |

## Quick start

To improve a bundled language, edit its existing pack. To keep a separate
custom variant, create a new pack as follows:

1. Copy [en.json](en.json) to a new file, such as `de-AT.json`.
2. Change `code` to `de-AT` and `name` to `Deutsch (Österreich)`.
3. Translate the **values** in `translations` and `web`. Keep the English keys
   and all formatting placeholders unchanged.
4. Copy `de-AT.json` to `/languages/de-AT.json` on the SD card.
5. Restart the machine so it discovers the new file.
6. Open **Settings → Display → Language**, select **Deutsch (Österreich)**, and tap
   **Save language**.

The choice is saved in `/languages/selected.txt` and loaded at the next
startup.

- A new JSON pack does not require a firmware or web build when the installed
firmware already supports the language manager and the touchscreen fonts cover
its characters.
- New fonts, keyboard layouts, or translation lookups in UI code
require separate development work.

## Files and installation

Use the SD card’s root `languages` directory:

```text
SDCARD/
└── languages/
    ├── en.json
    ├── zh-CN.json
    ├── de.json             ← German translation
    ├── de-AT.json          ← your custom variant
    ├── fonts/              ← LVGL font files
    ├── keyboard/
    │   └── pinyin.tsv      ← Chinese(Mandarin) keyboard dictionary
    └── selected.txt        ← Default language
```

Keep the existing font and keyboard resources when adding a pack. Keep
`selected.txt` when updating the folder to retain the saved choice. You do not
need to create or edit this preference file for normal touchscreen use.

Firmware updates preserve existing SD dictionaries. To update a translation,
replace its JSON file on the card and restart the machine. Uploading firmware
alone will not replace an older dictionary. Language files and the saved
selection are separate from configuration backups; back them up with the card.

## Pack format

This partial example is a working `de-AT.json`. Messages omitted from it use the
English fallback. For a complete translation, start with the full
[English reference catalogue](en.json).

```json
{
 "version": 1,
 "code": "de-AT",
 "name": "Deutsch (Österreich)",
 "translations": {
  "Home": "Start",
  "Language": "Sprache",
  "Save language": "Sprache speichern",
  "Pressure": "Druck",
  "Width -": "Breite -",
  "%u phases": "%u Phasen",
  "Pinned: %s": "Angeheftet: %s"
 },
 "web": {
  "Brew": "Bezug",
  "Open shot {value0}": "Bezug {value0} anzeigen"
 }
}
```

| Field | Required | Meaning |
| --- | --- | --- |
| `version` | Yes | The integer `1`. |
| `code` | Yes | Language identifier matching the filename, including case. |
| `name` | Yes | Language name shown in the touchscreen selector. |
| `translations` | Yes | English source keys mapped to translated text; may be an empty object for a partial pack. |
| `web` | No | Browser-specific messages and browser overrides of shared entries. |

Use a new language code rather than replacing `en`. English mode uses the
source text directly. Recommended code styles include `de`, `fr`, and `pt-BR`;
underscores, spaces, dots, and path separators are not accepted.

### Translation rules

- **Translate values only.** Keys identify existing interface messages. Changing
  a key creates a different lookup and will not translate the original caption.
- Preserve spaces, punctuation, icon characters, and `\n` escapes in keys.
  ASCII letter case is ignored during lookup, but otherwise keys must match.
- Keep each key unique. Avoid both `Home` and `HOME` in the same dictionary.
- Use nonempty strings as values. To leave a message untranslated, keep its
  English value or omit the entry.
- Use valid JSON: no comments or trailing commas.

## Share a translation

For a GitHub contribution, add `languages/<code>.json` and open a pull request.
Include the language code/name, hardware and firmware tested, known untranslated
areas or font requirements, and screenshots when possible.

Use the latest `en.json` as the reference when updating a pack. 


| Source | Version | Licence |
| --- | --- | --- |
| [Noto Sans Regular](https://github.com/notofonts/noto-fonts/blob/main/hinted/ttf/NotoSans/NotoSans-Regular.ttf) | 2.008 | [SIL OFL 1.1](OFL-NotoSans.txt) |
| [Noto Sans Thai Regular](https://github.com/notofonts/noto-fonts/blob/main/hinted/ttf/NotoSansThai/NotoSansThai-Regular.ttf) | 2.000 | [SIL OFL 1.1](OFL-NotoSans.txt) |
| [Noto Sans CJK SC Regular](https://github.com/notofonts/noto-cjk/blob/main/Sans/OTF/SimplifiedChinese/NotoSansCJKsc-Regular.otf) | 2.004 | [SIL OFL 1.1](OFL.txt) |

Matching Latin subsets use these sources (Selawik takes priority over Noto
Sans Light in the light subsets):

| Source | Version | Licence |
| --- | --- | --- |
| [Montserrat Medium](https://github.com/lvgl/lvgl/blob/v9.6.0/scripts/generators/built_in_font/Montserrat-Medium.ttf) | 7.200 | [SIL OFL 1.1](OFL-Montserrat.txt) |
| [Selawik Light](https://github.com/microsoft/Selawik/releases/tag/1.01) | 1.01 | [SIL OFL 1.1](OFL-Selawik.txt) |
| [Noto Sans Light](https://github.com/notofonts/noto-fonts/blob/main/hinted/ttf/NotoSans/NotoSans-Light.ttf) | 2.008 | [SIL OFL 1.1](OFL-NotoSans.txt) |