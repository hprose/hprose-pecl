--TEST--
Test that the Hprose\BytesIO class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
$bytes = new HproseBytesIO("Hello World! 你好，中国");
echo $bytes->length() . "\r\n";
echo $bytes->getc();
echo $bytes->getc();
echo $bytes->getc();
echo $bytes->getc();
echo $bytes->getc() . "\r\n";
$bytes->mark();
echo $bytes->read(3) . "\r\n";
echo $bytes->readuntil(" ") . "\r\n";
echo $bytes->readString(2) . "\r\n";
echo $bytes->readfull() . "\r\n";
$bytes->reset();
$bytes->skip(8);
echo $bytes->readString(5) . "\r\n";
$bytes->close();
echo $bytes->length() . "\r\n";
echo $bytes->getc() . "\r\n";
$bytes->unmark();
$bytes->write("I'm coming back.");
$bytes->reset();
echo $bytes->length() . "\r\n";
echo $bytes->toString() . "\r\n";
echo $bytes->toString() . "\r\n";
var_dump($bytes->eof());
$bytes->skip(100);
var_dump($bytes->eof());
unset($bytes);
$bytes = new HproseBytesIO();
$bytes->write("123", 2);
echo $bytes->toString() . "\r\n";
$clone = clone $bytes;
echo ((string)$clone == (string)$bytes) . "\r\n";
$clone->write("3");
echo $bytes->toString() . "\r\n";
echo $clone->toString() . "\r\n";
echo ((string)$clone != (string)$bytes) . "\r\n";
?>
--EXPECT--
28
Hello
 Wo
rld!
你好
，中国
你好，中国
0

16
I'm coming back.
I'm coming back.
bool(false)
bool(true)
12
1
12
123
1