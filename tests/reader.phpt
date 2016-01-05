--TEST--
Test that the Hprose\Reader class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class User {
    private $id;
    public $Name;
    public $age;
    public function __construct($name = "默认用户", $age = 12) {
        $this->id = 1;
        $this->Name = $name;
        $this->age = $age;
    }
}
class NoCtor {
    public $name = "NoCtor";
}
date_default_timezone_set('Asia/Shanghai');
$bytes = new HproseBytesIO();
$writer = new HproseWriter($bytes);
$reader = new HproseReader($bytes);
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
$writer->serialize($date);
$writer->serialize("我");
$writer->serialize("🆚");
$writer->serialize("Hello World! 你好，中国！");
$writer->serialize("Hello World! 你好，中国！");
$bytes->write("T123456.123456Z");
$bytes->write('g"550e8400-e29b-41d4-a716-446655440000"');

var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
$d = $reader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
$d = $reader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
$d = $reader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
$d = $reader->unserialize();
var_dump($d->format("Y-m-d H:i:s.u"));
var_dump($d->getTimezone()->getName());
var_dump($reader->unserialize());

$bytes->reset();
$writer->reset();
$reader->reset();

$writer->serialize(array(1,2,3,4,5));
$writer->serialize(array("Apple", "Banana", "Cherry"));
$writer->serialize(array("Apple"=>"苹果", "Banana"=>"香蕉", "Cherry"=>"樱桃"));
$stdobj = new stdClass();
$stdobj->name = "张三";
$stdobj->age = 18;
$writer->serialize($stdobj);
$user = new User();
$writer->serialize($user);
$writer->serialize($user);
$writer->serialize(new User("张三", 18));
$noctor = new NoCtor();
$writer->serialize($noctor);
$writer->serialize($noctor);

var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
var_dump($reader->unserialize());
print_r($reader->unserialize());
print_r($reader->unserialize());
print_r($reader->unserialize());
print_r($reader->unserialize());
print_r($reader->unserialize());

$writer->serialize("测试继承的readRaw。");
echo $reader->readRaw();

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
string(26) "2015-02-19 14:34:48.123456"
string(13) "Asia/Shanghai"
string(3) "我"
string(4) "🆚"
string(31) "Hello World! 你好，中国！"
string(31) "Hello World! 你好，中国！"
string(26) "1970-01-01 12:34:56.123456"
string(3) "UTC"
string(36) "550e8400-e29b-41d4-a716-446655440000"
array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
  [4]=>
  int(5)
}
array(3) {
  [0]=>
  string(5) "Apple"
  [1]=>
  string(6) "Banana"
  [2]=>
  string(6) "Cherry"
}
array(3) {
  ["Apple"]=>
  string(6) "苹果"
  ["Banana"]=>
  string(6) "香蕉"
  ["Cherry"]=>
  string(6) "樱桃"
}
array(2) {
  ["name"]=>
  string(6) "张三"
  ["age"]=>
  int(18)
}
User Object
(
    [id:User:private] => 1
    [Name] => 默认用户
    [age] => 12
)
User Object
(
    [id:User:private] => 1
    [Name] => 默认用户
    [age] => 12
)
User Object
(
    [id:User:private] => 1
    [Name] => 张三
    [age] => 18
)
NoCtor Object
(
    [name] => NoCtor
)
NoCtor Object
(
    [name] => NoCtor
)
s13"测试继承的readRaw。"