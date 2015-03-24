--TEST--
Test that the Hprose\ResultMode class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
echo HproseResultMode::Normal;
echo HproseResultMode::Serialized;
echo HproseResultMode::Raw;
echo HproseResultMode::RawWithEndTag;
?>
--EXPECT--
0123
