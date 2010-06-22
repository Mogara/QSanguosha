Open Source Sanguosha
==========

Introduction
----------

Sanguosha is both a popular board game and online game,
this project try to clone the Sanguosha online version.
The whole project is written in C++, 
use Qt's graphics view framework as game engine.
I've tried many open source game engine, 
such as SDL, HGE, Clanlib and others before, 
but many of them lacks some feature. 
Although Qt is an application framework instead of a game engine, 
but its graphics view framework is suitable for game developing.

For game scripting language, I choose QtScript. I tried Lua, 
as it is my favorite scripting language and it is commonly used in games. 
However, QtScript will be very easy to integrate in Qt application, 
and its grammer is just like JavaScript which many programmer familiar with.

Feature
----------

Most of them are in unfinished status.

1. Framework
    * Open source with Qt graphics view framework
    * Easy to add user-defined generals and cards

2. Operation experience
    * Drag card to target to use card
    * Use mouse's right button to drag card to use general's skill
    * Keyboard shortcut
    * Cards sorting (by card type and card suit)
    * Voice control (just experimental, very unstable)

Problems
----------

I found that this application is very unstable, 
the CPU usage will be 40~50% for a short while,
and the application won't response in the meantime. 
I think it is caused by some unstability of Qt's graphics view.


