--TEST--
Test that the Hprose\Tags class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
echo HproseTags::TagInteger;
echo HproseTags::TagEnd;
?>
--EXPECT--
iz
