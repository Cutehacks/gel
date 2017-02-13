// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

import QtQuick 2.3
import QtTest 1.0

import com.cutehacks.gel 1.0

TestCase {
    id: test2
    name: "JsonListModel"

    JsonListModel {
        id: jsonModel
    }

    SignalSpy {
        id: countSpy
        target: jsonModel
        signalName: "countChanged"
    }

    function arrayData(len) {
        var a = [];
        for (var i = 0; i < len; i++) {
            a.push({id: i, value: "foo" + i});
        }
        return a;
    }

    function init() {
        jsonModel.clear();
        countSpy.clear();
    }

    function test_add_array() {
        jsonModel.add(arrayData(10));

        verify(countSpy.count > 0);
        compare(jsonModel.count, 10);
    }

    function test_add_single_object() {
        jsonModel.add({id: 1, value: "a"});
        verify(countSpy.count >= 1);
        compare(jsonModel.count, 1);
    }

    function test_add_multiple_object() {
        jsonModel.add({id: 1});
        jsonModel.add({id: 2});
        jsonModel.add({id: 3});

        verify(countSpy.count >= 3);
        compare(jsonModel.count, 3);
    }

    function test_update() {
        jsonModel.add({id: 1});
        jsonModel.add({id: 2});
        jsonModel.add({id: 3, value: "a"});
        jsonModel.add({id: 3, value: "b"});
        jsonModel.add({id: 3, value: "c"});

        verify(countSpy.count >= 3);
        compare(jsonModel.count, 3);
        compare(jsonModel.get(3).value, "c");
    }

    function test_remove_id() {
        jsonModel.add(arrayData(10));
        jsonModel.remove(9);

        verify(countSpy.count >= 2);
        compare(jsonModel.count, 9);

        var nine = jsonModel.get(9);
        verify(nine === undefined);
    }

    function test_remove_object() {
        jsonModel.add(arrayData(10));
        jsonModel.remove({id: 9});

        verify(countSpy.count >= 2);
        compare(jsonModel.count, 9);

        var nine = jsonModel.get(9);
        verify(nine === undefined);
    }

    function test_as_array() {
        var a = arrayData(10);
        jsonModel.add(a);
        var b = jsonModel.asArray(true);

        for (var i = 0; i > a.length; i++) {
            compare(a[i] === b[i]);
        }
    }

    function test_at() {
        var a = arrayData(10);
        jsonModel.add(a);

        var nine = jsonModel.at(9);
        compare(nine.id, 9);
    }

    function test_clear() {
        var a = arrayData(10);
        jsonModel.add(a);
        compare(jsonModel.count, 10);
        jsonModel.clear();
        compare(jsonModel.count, 0);
    }
}
