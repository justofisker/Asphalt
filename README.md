# Asphalt
A game I'm working on to learn OpenGL, C, and game engines in general. It is heavily inspired by Minecraft, one of the first games I ever played and the game I've played the most of.

| Screenshots | |
| ----- | ----- |
| ![Image of a landscape](res/screenshots/screenshot_1.png) | ![Image of chunks loading](res/screenshots/screenshot_2.png)
| ![Image of underwater](res/screenshots/screenshot_3.png)

## Building and running

To build Asphalt you must first clone it with this command.

```
git clone --recurse-submodules https://github.com/justofisker/Asphalt.git
```


### Windows
For windows you can do either of the following

**With Visual Studio Build Tools** 

Open the Developer Command Prompt for VS. then navigate to Asphalt's directory and run the following commands.
```
SetupBuild.bat
Build.bat
build/Release/Asphalt.exe
```

**With Visual Studio IDE**

Run the following command

```
SetupBuild.bat
```

There should now be a `Asphalt.sln` solution file in the `build/` directory you can open with Visual Studio and run.

**Note:** MinGW might also work but I don't officially support it.

### Linux
For compiling and running on Linux just run these commands.
```
cd Asphalt
./SetupBuild.sh
./Build.sh
./build/Asphalt
```
