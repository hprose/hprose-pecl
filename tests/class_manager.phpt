--TEST--
Test that the Hprose\ClassManager class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
class Test {}
class Test2 {}
HproseClassManager::register("Test", "t");
echo HproseClassManager::getAlias("Test") . "\r\n";
echo HproseClassManager::getAlias("Test2") . "\r\n";
echo HproseClassManager::getClass("t") . "\r\n";
echo HproseClassManager::getClass("tttt") . "\r\n";
?>
--EXPECT--
t
Test2
Test
stdClass