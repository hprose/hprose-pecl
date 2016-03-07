# Hprose extension for PHP

[![Join the chat at https://gitter.im/hprose/hprose-pecl](https://img.shields.io/badge/GITTER-join%20chat-green.svg)](https://gitter.im/hprose/hprose-pecl?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/hprose/hprose-pecl.svg)](https://travis-ci.org/hprose/hprose-pecl)
![Supported PHP versions: 5.2 .. 7.0](https://img.shields.io/badge/php-5.2~7.0-blue.svg)
[![GitHub release](https://img.shields.io/github/release/hprose/hprose-pecl.svg)](https://github.com/hprose/hprose-pecl/releases)
[![License](https://img.shields.io/github/license/hprose/hprose-pecl.svg)](http://opensource.org/licenses/MIT)

## Introduction

*Hprose* is a High Performance Remote Object Service Engine.

It is a modern, lightweight, cross-language, cross-platform, object-oriented, high performance, remote dynamic communication middleware. It is not only easy to use, but powerful. You just need a little time to learn, then you can use it to easily construct cross language cross platform distributed application system.

*Hprose* supports many programming languages, for example:

* AAuto Quicker
* ActionScript
* ASP
* C++
* Dart
* Delphi/Free Pascal
* dotNET(C#, Visual Basic...)
* Golang
* Java
* JavaScript
* Node.js
* Objective-C
* Perl
* PHP
* Python
* Ruby
* ...

Through *Hprose*, You can conveniently and efficiently intercommunicate between those programming languages.

This project is the implementation of Hprose extension for PHP.

## Installation

There are many ways to build the package. Below you can find details for most
useful ways of package building:

-----------------------------------------------------------------------------
### Way 1: Building the package with PHP

1.  Create ext/hprose folder in the php-source-folder. Copy all files
    from the package into created folder.

2.  Run

        ./buildconf

    to rebuild PHP's configure script.

3.  Compile php with option:

    `--enable-hprose` to build bundled into PHP module

    `--enable-hprose=shared` to build dinamycally loadable module

-----------------------------------------------------------------------------
### Way 2: Building the package with phpize utility

1.  Unpack contents of the package.

2.  Run

        phpize

    script, which will prepare environment for building hprose package.

3.  Run

        ./configure --enable-hprose=shared

    to generate makefile.

4.  Run

        make

    to build hprose extension library. It will be placed into
    ./modules folder.

5.  Run

        make install

    to install hprose extension library into PHP

-----------------------------------------------------------------------------
### Way 3: Install via pecl

1. Run:

        pecl install hprose

    That's all.

-----------------------------------------------------------------------------

## Usage

This extension is a part of [Hprose for PHP](http://github.com/hprose/hprose-php), It only implements hprose serialize, unserialize and RPC protocol abstraction layer. The real http client and server implementation are written in PHP. If you want to know how to use it, look at here:

http://github.com/hprose/hprose-php
