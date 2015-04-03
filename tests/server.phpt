--TEST--
Test that the Hprose\Service class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
include("../hprose-php/Hprose.php");
function sum($a, $b, $c, $d) {
    return $a + $b + $c + $d;
}
function x(&$s) {
    var_dump($s);
    return $s;
}
function e() {
    throw new Exception("I am Exception");
}
class Base64Filter implements HproseFilter {
    public function inputFilter($data, $context) {
        return base64_decode($data);
    }
    public function outputFilter($data, $context) {
        return base64_encode($data);
    }
}
class JsonFilter implements HproseFilter {
    public function inputFilter($data, $context) {
        return json_decode($data);
    }
    public function outputFilter($data, $context) {
        return json_encode($data);
    }
}
$server = new HproseHttpServer();
$server->setErrorTypes(E_ALL);
$server->setDebugEnabled();
$server->add(array('x','e'));
$server->add(array('x','e'), array('y', 'z'));
$server->add(array("inputFilter", "outputFilter"), "Base64Filter");
$server->add(array("inputFilter", "outputFilter"), "Base64Filter", "base64");
$server->add(array("inputFilter", "outputFilter"), "Base64Filter", array('base64_decode', 'base64_encode'));
$server->add(new Base64Filter(), null, "base64");
$server->add("JsonFilter", null, "json");
$server->add('sum');
$server->addFilter(new HproseJSONRPCServiceFilter());
//$server->addFilter(new JsonFilter());
//$server->addFilter(new Base64Filter());
$server->start();
?>
--EXPECT--
OK