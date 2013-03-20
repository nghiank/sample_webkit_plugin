#Boilerplate Webkit Plugin for automation purpose#

## What is it about? ##
This plugin project is modified from a sample webkit plugin of chromium. I modified the project so that it can be built in our development environment. 

## Things need to do to enable automation for your web project ##
1. Add this line `<embed type="application/x-webkit-test-netscape" />` into body of your html which you want to automate on
2. Inside [main.cpp](https://github.com/nghiank/sample_webkit_plugin/blob/master/main.cpp), feel free to modify inside C function NPP_New(this function is triggered when your webkit plugin instance is created)
to excute the script you want.
3. Build the project(see below for details) and place the result dll or link the result dll to %RUNROOT%\Program Files\AutoCAD xxxx - English\AcWebBrowser\plugins 
4. When you open webbrowser in AutoCAD which navigate to your html (embeded with `<embed type="application/x-webkit-test-netscape" />`), NPP_New will be triggered

## How to build it? ##
Please use [amake.bat](https://github.com/nghiank/sample_webkit_plugin/blob/master/amake.bat) in the project to build it only in 32bit environment. If some error due to missing header _wtf/Platform.h_, you may need to copy the missing header files from Chromium project to your environment.  

## Interested in webkit plugin ? ##
http://trac.webkit.org/wiki/QtWebKitPlugins has info you need



