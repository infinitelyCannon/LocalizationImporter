# Localization Importer
This is a tool I made while working on [Path of Kami](https://store.steampowered.com/app/1558840/Path_of_Kami_Journey_Begins/) with the studio Captilight. It simplifies the process of pulling localizations into the engine with a front-end menu that takes an excel spreadsheet and compiles translations into the project.

![Configuration window for updating translations](/Docs/Import_Settings.png)

## How it Works
Once all the settings are configured, the tool will run through each command for updating text with your current setting for the "Game" target.
1. First "Gather Text" is called
2. Then all the text is exported to the local .po files
3. A python script is called to read from the excel sheet and update the .po files
4. All the .po files are imported back in and compiled.

## Setup
This tool depends on “Python Scripting Plugin” and “Editor Scripting Utilities” to be enabled. It also assumes that the cultures you want to update were already added as targets, as it won’t add new cultures that were found in the spreadsheet.

The tool assumes the spreadsheet is formatted a certain way. Where the first column holds the keys for the native culture, the second column holds the values for the native culture, and each column after holds the translated phrase.

| Keys        | English  | (Language) |
| ----------- | -------- | ---------- |
| Text/Id/One | Phrase 1 | Phrase 1   |
| Text/Id/Two | Phrase 2 | Phrase 2   |
| ...         | ...      | ...        |

## Open Source Libraries Used
* [OpenPyXl](https://openpyxl.readthedocs.io/en/stable/index.html) - For reading excel files
* [Material Design Icons](https://materialdesignicons.com/) - To help make the plugin icon