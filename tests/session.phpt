--TEST--
Test that the session works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
ini_set("session.serialize_handler", "hprose");
class User {
    public $name;
    public $age;
}
HproseClassManager::register("User", "User");
date_default_timezone_set('UTC');
session_start();
$_SESSION["Hello"] = "World";
$_SESSION["number"] = 3.14;
$_SESSION["date"] = new DateTime("1980-12-01");
$_SESSION["array"] = array("a" => "A", "b" => "B");
$user = new User();
$user->name = "Tom";
$user->age = 18;
$_SESSION["user"] = $user;
echo session_encode();
?>
--EXPECT--
m5{s5"Hello"s5"World"s6"number"d3.14;s4"date"D19801201T000000.000000Zs5"array"m2{uauAubuB}s4"user"c4"User"2{s4"name"s3"age"}o0{s3"Tom"i18;}}
