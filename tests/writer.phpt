--TEST--
Test that the Hprose\Writer class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
date_default_timezone_set('Asia/Shanghai');
$bytes = new HproseBytesIO();
$writer = new HproseWriter($bytes, true);
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
$writer->serialize(3.14159265897932);
$writer->serialize(log(-1));
$writer->serialize(log(0));
$writer->serialize(-log(0));
$writer->serialize(true);
$writer->serialize(false);
$writer->serialize("");
$writer->serialize(null);
$writer->serialize("我");
$writer->serialize("🆚");
echo $bytes . "\r\n";
$bytes->close();
unset($writer);
$writer = new HproseWriter($bytes);
$date = DateTime::createFromFormat('YmdHis.u', '20150219143448.123456');
$writer->serialize($date);
$writer->serialize(DateTime::createFromFormat('YmdHis.u', '20150219143448.123456',
                                                new DateTimeZone('UTC')));
$writer->serialize($date);
$writer->serialize("Hello World! 你好，中国！");
$writer->serialize("Hello World! 你好，中国！");
echo $bytes . "\r\n";
?>
--EXPECT--
0123456789i-2147483648;i2147483647;l2147483648;d3.14159265897932;NI-I+tfenu我s2"🆚"
D20150219T143448.123456;D20150219T143448.123456Zr0;s19"Hello World! 你好，中国！"r2;
