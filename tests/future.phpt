--TEST--
Test that the Hprose\Future class works.
--SKIPIF--
<?php if (!extension_loaded("hprose")) print "skip"; ?>
--FILE--
<?php
function test1() {
    $completer = new HproseCompleter();
    $future = $completer->future();
    $future->then(function($result) {
        return $result + 2;
    })->then(function($result) {
        return $result + 3;
    })->then(function($result) {
        return $result + 4;
    })->then(function($result) {
        echo $result . "\r\n";
    });
    $completer->complete(1);
}
function test2() {
    $completer = new HproseCompleter();
    $future = $completer->future();
    $completer->complete("Hello");
    $future->then(function($result) {
        return $result . " Tom";
    })->then(function($result) {
        return $result . " and Jerry!";
    })->then(function($result) {
        echo $result . "\r\n";
    });
}
function test3() {
    $completer = new HproseCompleter();
    $future = $completer->future();
    $completer->complete(array());
    $future->then(function($result) {
        $result[] = 1;
        return $result;
    })->then(function($result) {
        $result[] = 2;
        return $result;
    })->then(function($result) {
        print_r($result);
    });
}
function test4() {
    $completer = new HproseCompleter();
    $future = $completer->future();
    $completer->complete(new stdClass());
    $future->then(function($result) {
        $result->name = "Tom";
        return $result;
    })->then(function($result) {
        $result->age = 18;
        return $result;
    })->then(function($result) {
        print_r($result);
    });
}
function test5() {
    $completer = new HproseCompleter();
    $future = $completer->future();
    $future->catchError(function($e) {
        echo $e->getMessage();
    });
    $completer->completeError(new Exception("My Exception"));
}
test1();
test2();
test3();
test4();
test5();
?>
--EXPECT--
10
Hello Tom and Jerry!
Array
(
    [0] => 1
    [1] => 2
)
stdClass Object
(
    [name] => Tom
    [age] => 18
)
My Exception