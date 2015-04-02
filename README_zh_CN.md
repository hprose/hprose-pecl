# Hprose extension for PHP

[![Build Status](https://travis-ci.org/hprose/hprose-pecl.svg)](https://travis-ci.org/hprose/hprose-pecl)

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

## 使用

本扩展是 [Hprose for PHP](http://github.com/hprose/hprose-php) 的一部分，它仅实现了 hprose 序列化，反序列化以及 RPC 协议抽象层。实际的 Http 的客户端和服务器实现是使用 PHP 编写的。你可以通过以下网址来了解如何使用它：

http://github.com/hprose/hprose-php
