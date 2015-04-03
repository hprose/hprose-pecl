--TEST--
Test that the Hprose\Client class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
include("../hprose-php/Hprose.php");

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
$arr = array(
            'a' => 'xx',
            'b' => 'yy',
            'c' => 'zz'
        );

    echo '<pre>';
    echo "发送数据：<br>";
    print_r($arr);
    echo "<br>";
    echo "<br>";
    echo "不做处理直接返回数据：<br>";
    $test = new HproseHttpClient("http://192.168.1.2/server2.php");
//    $test->addFilter(new Base64Filter());
//    $test->addFilter(new JsonFilter());

    $args = array($arr);
    var_dump($test->invoke("x", $args, 0, HproseResultMode::Serialized, 0));
    var_dump($test->invoke("x", $args, 0, HproseResultMode::Raw, 0));
    var_dump($test->invoke("x", $args, 0, HproseResultMode::RawWithEndTag, 0));
    var_dump($test->x($arr));

    $args = array($arr);
    var_dump($test->invoke("x", $args, 0, HproseResultMode::Serialized, 0));
    var_dump($test->invoke("x", $args, 0, HproseResultMode::Raw, 0));
    var_dump($test->invoke("x", $args, 0, HproseResultMode::RawWithEndTag, 0));
    var_dump($test->x("Hello 0"));
    var_dump($test->x("Hello 1"));
    var_dump($test->x("Hello 2"));
    var_dump($test->x("Hello 3"));
    var_dump($test->x("Hello 4"));
    var_dump($test->x("Hello 5"));
    var_dump($test->x("Hello 6"));
    var_dump($test->x("Hello 7"));
    var_dump($test->x("Hello 8"));
    var_dump($test->x("Hello 9"));

    try {
        $test->e();
    }
    catch (Exception $e) {
        var_dump($e->getMessage());
    }

    $test->x("async hello1", function($result, $args, $error) {
        echo "result: " . $result . "\r\n";
        echo "error: " . $error . "\r\n";
    });
    $test->x("async hello2", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->x("async hello3", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->x("async hello4", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->x("async hello5", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->x("async hello6", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->x("async hello7", function($result) {
        echo "result: " . $result . "\r\n";
    });
    $test->loop();

?>
--EXPECT--
Array
(
    [a] => xx
    [b] => yy
    [c] => zz
)
