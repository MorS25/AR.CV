# AR.CV
Leverages OpenCV through Wi-Fi to accomplish AR.Drone 2.0 swarming behavior.

## System Requirements
The application is currently built with C++ using OpenCV 2.4.10 through any OS or IDE that successfully compiles it without interfering with Git.

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
