Gel is a framework for storing JSON objects in a ListView compatible model.
It offers more functionality than the builtin ListModel by offering support
for sorting and filtering.

# Installation

Gel is available on qpm:

`qpm install com.cutehacks.gel`

# Usage

In your main.cpp:

```
#include "com/cutehacks/gel/gel.h"
	
int main(int argc, char *argv[])
{
	...
	
	com::cutehacks::gel::registerEngine(&engine);
	
	...
}
```

The above function will register two QML types with the QML engine which are documented
below.

# API

## JsonListModel

A implementation of a model specifically designed for storing for JSON objects. The item
has the following properties and functions:

### idAttribute : property string: "id"

Specifies the name of the property that should be used to uniquely identify 
this object within the model. Adding an object with the same value for this
property will update the existing object instead of creating a new one.

### add(jsobject | jsarray | string | number | date) : function

Add a new JSON object or an array of objects to the model

### remove(jsobject | string | number | date) : function

Remove the specified object from the model.

### clear() : function

Remove all items from the model.

### at(index : number) : function

Return the jsobject at the index specified by the number.

## Collection

An item for sorting an filtering a JsonListModel. The Collection itself does not
store any data, but rather proxies the data stored inside the source model.

### model : JsonListModel

The model used to store the JSON objects for this collection.

### comparator : string | function

Specifies the comparator to use when sorting the collection.

* If a string is passed to this property, it refers to the property name that
 should be used for sorting.
* Nested properties are supported by using the dot notation (eg: `"owner.firstname"`)
* If a function is passed to this property, it should have the following signature:

```
function lessThan(a, b) {
	return a.value < b.value;
}
```

The function will receive two arguments that are JSON objects. The implementation of 
this function is responsible for comparing the two items and returning true if `a` is
less than `b`; otherwise false.

**NOTE** When comparing string properties in a comparator function, you might want to
consider using the `localeCompare` function for proper locale specific sorting. For example:

```
function lessThan(a, b) {
	return a.value.localeCompare(b.value) < 0;
}
```

### filter : function

A function indicating which items should be included in th resulting collection. The
function should return true for items that should be included; otherwise false.

### at(index: number) : function

Return the jsobject at the index specified by the number. If Collection is sorted or filtered, then
the index here refers to the index in the Collection *not* the JsonListModel.

## Example

The following is an example of using the items:

```
    Collection {
        id: collection
        model: JsonListModel {
            id: jsonModel
        }
        filter: function(item) {
            return item.value.indexOf("foo") >= 0;
        }
        comparator: "value"
    }

```

The above snippet will sort a list of JSON objects based on the "value"
property, but will only show objects whose "value" property contains the
word "foo".
