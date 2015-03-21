--TEST--
Test that the Hprose\Reader class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class User {
    private $id = 1;
    public $name = "默认用户";
    public $age = 18;
}
date_default_timezone_set('Asia/Shanghai');
$bytes = new HproseBytesIO();
$writer = new HproseWriter($bytes, true);
$rawreader = new HproseReader($bytes);
$writer->serialize(0);
$writer->serialize(1);
$writer->serialize(2);
$writer->serialize(3);
$writer->serialize(4);
$writer->serialize(5);
$writer->serialize(6);
$writer->serialize(7);
$writer->serialize(8);
$writer->serialize(9);
$writer->serialize(-2147483648);
$writer->serialize(2147483647);
$writer->serialize(2147483648);
$writer->serialize(3.1415926535898);
$writer->serialize(log(-1));
$writer->serialize(log(0));
$writer->serialize(-log(0));
$writer->serialize(true);
$writer->serialize(false);
$writer->serialize("");
$writer->serialize(null);
$writer->serialize("我");
$writer->serialize("🆚");

var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());

?>
--EXPECT--
int(0)
int(1)
int(2)
int(3)
int(4)
int(5)
int(6)
int(7)
int(8)
int(9)
int(-2147483648)
int(2147483647)
string(10) "2147483648"
float(3.1415926535898)
float(NAN)
float(-INF)
float(INF)