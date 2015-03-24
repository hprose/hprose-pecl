--TEST--
Test that the Hprose\Formatter class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class Test {
    private $id = 1;
    protected $foo = "foo";
    public $bar = array('b', 'a', 'r');
}
HproseClassManager::register("Test", "t");
$test = new Test();
$data = hprose_serialize($test);
echo $data . "\r\n";
$test2 = HproseFormatter::unserialize($data);
print_r($test2);
?>
--EXPECT--
c1"t"3{s2"id"s3"foo"s3"bar"}o0{1r1;a3{ubuaur}}
Test Object
(
    [id:Test:private] => 1
    [foo:protected] => foo
    [bar] => Array
        (
            [0] => b
            [1] => a
            [2] => r
        )

)