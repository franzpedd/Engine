## Cosmos Engine and Editor

 The goal with this engine is to be able to deploy a game in Desktop(Windows and Linux) and then mobile(Android) using the Vulkan as the Renderer API. Other operating systems may be considered after the mentioned ones. At the moment it's not near to it's goal but as the Editor is developed alongside the Engine it'll be done.

## Building
You'll need to have:
 * CMake; // this is used by the Assimp dependency
 * Python 3.14 with TQDM and Requests modules installed; // used to download other dependencies

Steps:
1) Run ```py Setup.py``` on your terminal. It'll download any dependency the project have.  If you don't have the Vulkan SDK in your machine it'll be prompt to be installed, after it's installation close your terminal and re-run ```py Setup.py``` in order to refresh your envieronment variables.
2) On Linux it'll be generated makefiles, XCode solution on MacOS andVisual Studio 2022 Solution on Windows.
3) Build the generated solution.

Info:
* This is an in-development project and therefore some mechanisms may not be fully implemented/tested on all platforms.

## Editor screenshot
![Screenshot](documentation/editor-viewport.jpg)