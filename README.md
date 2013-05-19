# CLI-DevTool
## Author
    Yohei.Sekiguchi <ysekiguchi.zzz@gmail.com> 

## Getting started.
 * English page.
 * [Japanese page.](http://yohe.github.io/CLI-DevTool/)

# Description of Software
This software is development tool for creating console. The created console has the following features.
 
* Command-Name completion by tab key
* Parameter completion by tab key
    - This is programable at each command.
* Command History
* Some BuiltInCommands
    1. help command : Display command help.
    2. history command : Display of a history, and execution.
    3. script command : Logging operation. (not cool)
* Customizable key bindings.

----

* Support OS
    * Mac OS X
    * Linux

----
## Required
* Packages
    * cmake 2.6 or later
    * g++ or clang++

* Use libraries
    * ncurses-dev

## Compile and Running.
* For sample
    - Please edit the Makefile if you want to change the compiler.

```bash
$ cmake .
$ make
$ ./console
```

## License
    The MIT License
    Copyright (c) 2011 Yohei.Sekiguchi

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

## Issue
 
 1. key storke can not use following pattern.
  * Example <BR>
    This pattern issue is KeyCode(91).<BR>
    91 has a next key code, and 91 is a last key code. Both of these keystrokes can not be registed.

     | KeyStroke | KeyCode  |
     |-----------|----------|
     | Up        | 27-91-65 |
     | ALt-[     | 27-91    |

   this issues is small. You will not register the complex keystrokes, will you?

