--TEST--
Test that the Hprose\Tags class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class Test {}
class Test2 {}
HproseClassManager::register("Test", "t");
echo Hprose\ClassManager::getAlias("Test") . "\r\n";
echo Hprose\ClassManager::getAlias("Test2") . "\r\n";
echo Hprose\ClassManager::getClass("t") . "\r\n";
echo Hprose\ClassManager::getClass("tttt") . "\r\n";
?>
--EXPECT--
t
Test2
Test
stdClass