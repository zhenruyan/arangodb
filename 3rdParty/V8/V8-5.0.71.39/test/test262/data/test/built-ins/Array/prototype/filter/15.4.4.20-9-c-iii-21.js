// Copyright (c) 2012 Ecma International.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
es5id: 15.4.4.20-9-c-iii-21
description: >
    Array.prototype.filter - return value of callbackfn is the Math
    object
---*/

        function callbackfn(val, idx, obj) {
            return Math;
        }

        var newArr = [11].filter(callbackfn);

assert.sameValue(newArr.length, 1, 'newArr.length');
assert.sameValue(newArr[0], 11, 'newArr[0]');