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
$date = new DateTime('2015-02-19T14:34:48.123456');
$writer->serialize($date);
$writer->serialize(new DateTime('2015-02-19T14:34:48.123456',
                                 new DateTimeZone('UTC')));
$bytes->write("T123456.123456Z");
//$writer->serialize("我");
//$writer->serialize("🆚");

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
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
var_dump($rawreader->unserialize());
$d = $rawreader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
$d = $rawreader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
$d = $rawreader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());

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
bool(true)
bool(false)
string(0) ""
NULL
string(26) "2015-02-19 14:34:48.123456"
string(13) "Asia/Shanghai"
string(26) "2015-02-19 14:34:48.123456"
string(3) "UTC"
string(26) "1970-01-01 12:34:56.123456"
string(3) "UTC"