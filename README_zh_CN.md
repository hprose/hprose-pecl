# Hprose extension for PHP

[![Join the chat at https://gitter.im/hprose/hprose-pecl](https://img.shields.io/badge/GITTER-join%20chat-green.svg)](https://gitter.im/hprose/hprose-pecl?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/hprose/hprose-pecl.svg)](https://travis-ci.org/hprose/hprose-pecl)
![Supported PHP versions: 5.2 .. 7.0](https://img.shields.io/badge/php-5.2~7.0-blue.svg)
[![GitHub release](https://img.shields.io/github/release/hprose/hprose-pecl.svg)](https://github.com/hprose/hprose-pecl/releases)
[![License](https://img.shields.io/github/license/hprose/hprose-pecl.svg)](http://opensource.org/licenses/MIT)

## 简介

*Hprose* 是高性能远程对象服务引擎（High Performance Remote Object Service Engine）的缩写。

它是一个先进的轻量级的跨语言跨平台面向对象的高性能远程动态通讯中间件。它不仅简单易用，而且功能强大。你只需要稍许的时间去学习，就能用它轻松构建跨语言跨平台的分布式应用系统了。

*Hprose* 支持众多编程语言，例如：

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

通过 *Hprose*，你就可以在这些语言之间方便高效的实现互通了。

本项目是 Hprose 的 PHP 的 C 扩展实现。

## 安装

有许多方式来构建此包，下面是两种最常用的方式：

-----------------------------------------------------------------------------
### 方式 1：跟 PHP 一起构建包

1.  在 PHP 源码文件夹下创建 ext/hprose 文件夹。将包中的所有文件复制到创建的文件夹下。

2.  运行

        ./buildconf

    来重新构建 PHP 的配置脚本。

3.  使用此选项来编译 PHP：

    `--enable-hprose` 以捆绑方式来构建 PHP 模块。

    `--enable-hprose=shared` 构建动态加载模块。

-----------------------------------------------------------------------------
### 方式 2：使用 phpize 工具来构建包

1.  解压此包。

2.  运行脚本

        phpize

    它将为构建 hprose 扩展准备环境。

3.  运行

        ./configure --enable-hprose=shared

    以生成 makefile

4.  运行

        make

    以构建 hprose 扩展库. 它将被置于 ./modules 目录下。

5.  运行

        make install

    以安装 hprose 扩展库到 PHP 环境下。

-----------------------------------------------------------------------------
### 方式 3：通过 pecl 安装

1. 运行：

        pecl install hprose

    这就好了。

-----------------------------------------------------------------------------

## 使用

本扩展是 [Hprose for PHP](http://github.com/hprose/hprose-php) 的一部分，它仅实现了 hprose 序列化，反序列化以及 RPC 协议抽象层。实际的 Http 的客户端和服务器实现是使用 PHP 编写的。你可以通过以下网址来了解如何使用它：

http://github.com/hprose/hprose-php
