--TEST--
Test that the Hprose\Formatter class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class Test {
    private $Id = 1;
    protected $foo = "foo";
    public $bar = array('b', 'a', 'r');
}
class MyData implements IteratorAggregate {
    public $property1 = "Public property one";
    public $property2 = "Public property two";
    public $property3 = "Public property three";

    public function __construct() {
        $this->property4 = "last property";
    }

    public function getIterator() {
        return new ArrayIterator($this);
    }
}
HproseClassManager::register("Test", "t");
$test = new Test();
$data = hprose_serialize($test);
echo $data . "\r\n";
$test2 = HproseFormatter::unserialize($data);
print_r($test2);

$myData = new myData();
$data = hprose_serialize($myData);
echo $data . "\r\n";
$myData2 = HproseFormatter::unserialize($data);
print_r($myData2);

?>
--EXPECT--
c1"t"3{s2"id"s3"foo"s3"bar"}o0{1r1;a3{ubuaur}}
Test Object
(
    [Id:Test:private] => 1
    [foo:protected] => foo
    [bar] => Array
        (
            [0] => b
            [1] => a
            [2] => r
        )

)
a4{s19"Public property one"s19"Public property two"s21"Public property three"s13"last property"}
Array
(
    [0] => Public property one
    [1] => Public property two
    [2] => Public property three
    [3] => last property
)