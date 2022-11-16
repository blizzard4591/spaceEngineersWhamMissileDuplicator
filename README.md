# Space Engineers Missile Blueprint Duplicator
A utility for duplicating missiles made with the [WHAM (Whip's Homing Advanced Missile) script](https://steamcommunity.com/sharedfiles/filedetails/?id=2108743626).
Usually, when you change or update your missile, you will spent a significant amount of time duplicating your blueprint - always enabling the batteries, updating the group name, recompiling the script, changing the grid name, setting the batteries to recharge again...

Tedious? **Yes!** Error prone? **Yes!**

## License
The tool is governed by the GNU GPL v2.0 license. See [`LICENSE`](LICENSE) for more information.

## Requirements
 - [CMake](https://cmake.org/)
 - [Qt 5 or 6](https://www.qt.io/)
 - C++17, meaning a recent MSVC/GCC/Clang that supports at least C++17
 
## How to use
Simply run the tool and follow the on-screen instructions.
Lets say that your missile type is called `Urmel Wasp MK_1` and your blueprint is called `Urmel Wasp MK_1 XYZ`, where `XYZ` is a number like `1`.
The script assumes that:
 - the grid of your blueprint is named `Urmel Wasp MK_1 XYZ`,
 - the name of the blueprint is `Urmel Wasp MK_1 XYZ`,
 - the group is called `SOMETHING XYZ` and
 - all blocks carry the prefix `(SOMETHING XYZ) `.
The `XYZ` number should be the same for all.
If any of these assumptions are found violated, the program will quit.

1. The app will ask you for your blueprint folder, the default is `%APPDATA%/SpaceEngineers/Blueprints/local`.
2. Once this path is deemed valid, you will be presented with a list of your blueprints. Select one by entering its number.
3. You will now be asked for the *initial index* and the *number of copies*.
   Given you have updated `Urmel Wasp MK_1 1` (the first one) and you want a total of eight missile blueprints, you are missing seven copies.
   Therefore, choose `2` as the starting index and `7` as the number of copies.
4. Enjoy!

On Linux or MacOS, if CMake and Qt are readily available:
```
mkdir build
cd build
cmake ..
make -j4
```

On Windows, edit `CMakeLists.txt` such that `PROJECT_CMAKE_SEARCH_PATH` points to your Qt6 installation.
