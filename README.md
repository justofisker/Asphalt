# Asphalt
A game I'm working on to learn OpenGL, C, and game engines in general. It is heavily inspired by Minecraft, one of the first games I ever played and the game I've played the most of.

![Image of a landscape](https://github.com/justofisker/Asphalt/blob/master/res/screenshots/screenshot_1.png)

![Image of chunks loading](https://github.com/justofisker/Asphalt/blob/master/res/screenshots/screenshot_2.png)

![Image of underwater](https://github.com/justofisker/Asphalt/blob/master/res/screenshots/screenshot_3.png)

## Cloning
`git clone --recurse-submodules https://github.com/justofisker/Asphalt`

## Building
### Windows
After cloning Asphalt run these commands.
```
cd Asphalt
md build
cd build
cmake ..
```
There should now be a `Asphalt.sln` solution file in the `/build/` directory you can open with Visual Studio and run.
### Linux
After cloning Asphalt run these commands.
```
cd Asphalt
mkdir build
cd build
cmake ..
make
```
There should now be an Asphalt executable you can run through the terminal, but you must run it in the same directory that `/res/` is in.
```
cd ..
./build/Asphalt
```