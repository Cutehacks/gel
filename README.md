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

### dynamicRoles : property bool: false

Specifies whether the added JSON objects added might have a varying set of
properties. If not set only the properties of the first JSON object in a
list will get extracted as roles. Enabling this might have a performance
penalty when dealing with large collections of items.

**NOTE:** QML's ListView seems to have a bug as of present (Qt 5.5.1) where it
doesn't detect roles being added to a model, even if modelReset is fired.
Here's a workaround that can be used to get around that:

```qml
Connections {
    target: model
    onModelReset: {
        // workaround for ListView not handling new roles being added
        var temp = view.model;
        view.model = null;
        view.model = temp;
    }
}
```

### attachedProperties : property jsobject

Specifies additional properties (roles) that should be attached to every object
in the model. These properties will then be exposed to the view that uses the model 
so they are available to the delegate for property binding, but also available for 
sorting (via `comparator`) and ListView's `section` property.

The properties are evaluated at runtime when the view needs them, they are not 
pre-calculated on insertion into the model. Therefore if the delegate does not use 
them, they should have no additional cost.

The types of properties supported are primitive types such as Number, String and Date, 
but perhaps most powerful is the ability to attach functions. For example:

```qml
JsonListModel {
	// ...

	attachedProperties: ({
		lowercase: function(item, index) {
			return item.title.toLowerCase();
		}
	})
}

```

The above example is somewhat contrived since you could just as easily do the same
thing in the delegate, but by using an attached property, you could also sort or
section (group by) based on this property.

**NOTE:** The declaration of the object must be surrounded by round brackets `(` and `)` 

### add(jsobject | jsarray | string | number | date) : function

Add a new JSON object or an array of objects to the model

### remove(jsobject | string | number | date) : function

Remove the specified object from the model.

### clear() : function

Remove all items from the model.

### at(index : number) : function

Return the jsobject at the index specified by the number.

### get(id : jsvalue) : function

Return the jsobject that has the id specified by 'id'.

### asArray([deepCopy : bool]) : array

Creates a new array containing the items in the model. If objects are
stored in the model, then the default behavior is that the array contains
references to the objects in the model such that modifications to the array
will affect the model (though no signals are emitted). If the value of 
`deepCopy` is set to `true` then the objects are cloned before being
added to the array.

## Collection

An item for sorting and filtering a JsonListModel. The Collection itself does not
store any data, but rather proxies the data stored inside the source model.

### model : JsonListModel

The model used to store the JSON objects for this collection.

### comparator : string | function

Specifies the comparator to use when sorting the collection.

* If a string is passed to this property, it refers to the property name that
 should be used for sorting.
* Nested properties are supported by using the dot notation (eg: `"owner.firstname"`)
* If a function is passed to this property, it should have the following signature:

```js
function lessThan(a, b) {
	return a.value < b.value;
}
```

The function will receive two arguments that are JSON objects. The implementation of 
this function is responsible for comparing the two items and returning true if `a` is
less than `b`; otherwise false.

**NOTE** When comparing string properties in a comparator function, you might want to
consider using the `localeCompare` function for proper locale specific sorting. For example:

```js
function lessThan(a, b) {
	return a.value.localeCompare(b.value) < 0;
}
```

### filter : function

A function indicating which items should be included in the resulting collection. The
function should return true for items that should be included; otherwise false.

The signature of the filter function is:

```
    function(item, index) {
        return true; // if item should be included
    }
```


### at(index: number) : function

Return the jsobject at the index specified by the number. If Collection is sorted or filtered, then
the index here refers to the index in the Collection *not* the JsonListModel.

### reSort() : function

Trigger sorting the collection again.

**NOTE** Calling this function is *not* required if data within the model itself has changed
(re-sorting will be conducted automatically in that case). You want to call this function if
external data has been updated, e.g. you sort a model by distance and the current position
of the user has changed.

### reFilter() : function

Trigger refiltering the collection.

**NOTE** Same as reSort(), this function is not required if the data within the model itself
has changed.

### descendingSort : property bool: false

Indicates if the role based sorting is sorted in ascending or descending order.

**NOTE** If the `comparator` is a function, this property has no effect.

### caseSensitiveSort : property bool: true

Indicates if the role based sorting is case sensitive or not.

**NOTE** If the `comparator` is a function, this property has no effect.

### localeAwareSort : property bool: false

Indicates if the role based sorting is locale aware or not.

**NOTE** If the `comparator` is a function, this property has no effect.

## Example

The following is an example of using the items:

```qml
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
