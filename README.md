# wxmeasure - take measurements from photographs

wxmeasure allows one to take measurements from a photograph where an object known to be 
an imperial foot is present.  The program is calibrated by measuring the object in calibration
mode, then measurements can be taken in the photograph with measurement mode. 

This code is offered for use under the terms and conditions of the MIT License.  See MIT.txt
for specifics.

## Building

wxmeasure uses the wxWidgets GUI library.  The library can be package-installed by your 
operating system's package manager, in which case bulding wxmeasure is as simple as:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

There's no install command right now.

If you're building this on Windows with the MSYS2 environment, the cmake command should look like
this:

```
$ cmake -G "MSYS Makefiles" ..
```

If you don't want to package-install wxWidgets, you can build it yourself in another directory.
If you do that, you can use in in wxmeasure by adding "-DWXDIR=..." to the cmake command, like 
this:

```
$ cmake -DWXDIR=path/to/another/wxwidgets/build_directory
```

and cmake will run wx-config to configure wxWidgets for wxmeasure's build.

## Usage

If a file called wxmeasure.conf is in the same directory as the wxmeasure executable, it is read on startup and the properties contained therein
are used by the program.  You can change the properties by using the menu, Edit->Properties..., and a dialog is displayed where you can add, 
edit, and delete properties.  Here are the properties wxmeasure recognizes (so far):

- **backgroundcolor**:  - sets background color.  Default: BLACK
- **filepath**: Sets the default path when the program is started.  Default: current working directory

## ToDo

- Release tag.
- Metric system measurements.
- Calibration to other lengths than one foot.
- Windows installer for the release.
