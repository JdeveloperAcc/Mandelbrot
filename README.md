My test exam description to enroll into a company:

Fractals Drawing 

Prepare two separate Qt applications for:
1. visualisation of Mandelbrot set
2. fractal computation

Where the first application will send fractal parameters and the second one will calculate and send results back to the first one. 
Data inter-change between applications has to be implemented via a TCP/IP Ethernet connection.

https://en.wikipedia.org/wiki/Mandelbrot_set#/media/File:Mandel_zoom_00_mandelbrot_set.jpg

Let a user set following:
. Resolution of a fractal (a width and a height in pixels)
. Zoom in/out or a position of a complex plane
. Change a palette for fractals' coloring 
. Save the generated picture in JPEG or PNG formats

Some advice: "Try to separate a graphical part of a Qt project from actual computation with using a new thread(s). Test routines are welcome.


--------------------------------------------------------------------------------------------------------------------------------------------------------------

The Referenced Resources:
1.
Qt for Beginners
https://wiki.qt.io/Qt_for_Beginners

2.
QML Tutorial
This tutorial gives an introduction to QML, the language for Qt Quick UIs. It doesn't cover everything; the emphasis is on teaching the key principles, and features are introduced as needed.
https://doc.qt.io/qt-6/qml-tutorial.html

3.
QML Tutorial 2 - QML Components
This chapter adds a color picker to change the color of the text.
https://doc.qt.io/qt-6/qml-tutorial2.html

4.
Mandelbrot Set
https://en.wikipedia.org/wiki/Mandelbrot_set

5.
Mandelbrot
The Mandelbrot example demonstrates multi-thread programming using Qt. It shows how to use a worker thread to perform heavy computations without blocking the main thread's event loop.
https://doc.qt.io/qt-6/qtcore-threads-mandelbrot-example.html

6.
Network Programming with Qt
https://doc.qt.io/qt-6/qtnetwork-programming.html

7.
QTcpServer Class
The QTcpServer class provides a TCP-based server.
https://doc.qt.io/qt-6/qtcpserver.html

8.
QNetworkAccessManager Class
The QNetworkAccessManager class allows the application to send network requests and receive replies
https://doc.qt.io/qt-6/qnetworkaccessmanager.html

9. 
HTTP (Hypertext Transfer Protocol)
https://en.wikipedia.org/wiki/HTTP

Let's make Qt Socket's implementation, QNetworkAccessManager connects to listening QTcpServer, an used protocol is, 'take it simple as much as posible,' so HTTP/1.1.

My designed URL: http://127.0.0.1:8055/?centerX=-0.568348&centerY=-0.0395159&scaleFactor=0.004038&resultWidth=1024&resultHeight=768&pixelRatio=1.5&color=4278190080

10.
QFileDialog Class
The QFileDialog class provides a dialog that allows users to select files or directories.
https://doc.qt.io/qt-6/qfiledialog.html

11.
Chapter 1: Writing a Unit Test
This first chapter demonstrates how to write a simple unit test and how to run the test case as a stand-alone executable.
https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html

12.
Qt Test Best Practices
We recommend that you add Qt tests for bug fixes and new features.
https://doc.qt.io/qt-6/qttest-best-practices-qdoc.html
