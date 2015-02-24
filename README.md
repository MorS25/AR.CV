# AR.CV
Leverages OpenCV through Wi-Fi to accomplish AR.Drone 2.0 swarming behavior.

## System Requirements
The application is currently built with C++ using OpenCV 2.4.10 through any OS or IDE that successfully compiles it without interfering with Git.

## Current Usage
With successful compilation of the application, one can add an "object.jpg" file and a "scene.jpg" so that the object can be picked out from the scene through homography. Note that the files must be placed in the same directory as the executable for the time being.

The eventuality is that we can nest the existing code into an infinite loop, reading frame by frame from the open video port on the AR.Drone, mapping features over time. Every so often, we can send flight commands to the follower drone based on the recognized features from the leader drone by jugding the distance from the absolute center of the following drone's vision. Basically, the follower drone should always attempt to center its vision on the leader drone.

This doesn't account for depth, but this can be adjusted for by calculating the size of combined recognized features, always attempting to scale to the known features appropriately.

## Credits
AR.CV adapts [this tutorial](http://docs.opencv.org/doc/tutorials/features2d/feature_homography/feature_homography.html) from the official OpenCV documentation.

## Installing OpenCV
### OS X
Here's a [tutorial](https://sites.google.com/site/yonasstephenfyp2013/updates/tutorialinstallingopencvonmacosxmountainlion) for building OpenCV on OS X. The guide works with Yosemite. Note that CMake can be installed via Homebrew.

This guide is also confirmed to work with XCode 6.1.1.

### Windows
Here's a [tutorial](http://www.anlak.com/2012/10/using-opencv-2-4-x-with-visual-studio-2010-tutorial.html) on installing OpenCV for Windows by use of an official binary file.

The guide works with Windows 8.1.

This guide is also confirmed to work with Visual Studio 2013. Just use the "VS12" linkage built into OpenCV -- it works with Visual Studio 2013. Be sure to use the OpenCV 2.4.10 library files as per the system requirements.
