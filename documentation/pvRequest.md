# pvRequest Parser Syntax and Output

This is an attempt to create an EBNF definition of the syntax currently accepted
by the epics::pvData::CreateRequest parser and to document the structure that it
creates. I developed this document by a combination of reading through the
source code and experimenting, but since the parser does not use normal
recursive descent techniques and has various short-cuts in it I might have
missed some aspects of what it accepts or rejects.

## EBNF Syntax Used

The EBNF definitions below use the following syntax:

* Spaces and newlines are only significant inside quotes.
* `=` marks the name preceeding it as a token being defined.
* `;` signifies the end of a token definition.
* `(` and `)` mark the start and end of a group of tokens.
* `[` and `]` are for declaring character classes.
* `|` introduces an alternative token/group to those preceeding it.
* `?` means the token/group preceeding it is allowed 0 or 1 times.
* `*` means the token/group preceeding it is allowed 0 or more times.
* `+` means the token/group preceeding it is allowed 1 or more times.

## EBNF

```
    fieldName = [A-Za-z_] [A-Za-z_0-9]+ ;

    option = fieldName '=' optionValue ;
    options = option ( ',' option )* ;
    optionList = '[' options ']' ;

    nameWithOptions = fieldName optionList? ;

    subRequest =
        nameWithOptions (
            ( ',' subRequest? ) |
            ( '.' subRequest ) |
            ( '{' subRequest '}' ( ',' subRequest? )? )
        ) ;

    record   = 'record' optionList ;
    field    = 'field(' subRequest? ')' ;
    getField = 'getField(' subRequest? ')' ;
    putField = 'putField(' subRequest? ')' ;

    createRequest = ( record | field | getField | putField )+ | subRequest? ;
```

## Generated pvStructure

I created a simple test program `pvrDump` that just dumps the structure of its
single pvRequest string argument. The simplest way to show how the structure is
generated from the input is to show a series of examples using this tool.

An empty string becomes an empty structure, as does a string with an empty set
of global `record` options:

```
tux% pvrDump ''
structure 

tux% pvrDump 'record[]'
structure 

```

Including any `key=value` pairs inside the `record` brackets creates a sub-tree
`record` with `_options` inside that holds the set of option value strings
provided:

```
tux% pvrDump 'record[process=true]'
structure 
    structure record
        structure _options
            string process true

```

A `field()` section starts another sub-tree in the output called `field`, even
if the parentheses are empty. A `fieldName` or `fieldName.fieldName...` inside
the parentheses generates structure nodes for each of those names inside the
`field` sub-tree:

```
tux% pvrDump 'field()'
structure 
    structure field

tux% pvrDump 'field(a)'
structure 
    structure field
        structure a

tux% pvrDump 'field(a.b)'
structure 
    structure field
        structure a
            structure b

```

Additional fields can be named and get attached at the top level:

```
tux% pvrDump 'field(a,b,c,d)'
structure 
    structure field
        structure a
        structure b
        structure c
        structure d

tux% pvrDump 'field(a.b,c.d)'
structure 
    structure field
        structure a
            structure b
        structure c
            structure d

```

When multiple fields share the same parent path, it is necessary to use braces
to specify them:

```
tux% pvrDump 'field(a{b,c.d})'
structure 
    structure field
        structure a
            structure b
            structure c
                structure d

tux% pvrDump 'field(a{b,c},d)'
structure 
    structure field
        structure a
            structure b
            structure c
        structure d

```

Options given after any `fieldName` part become a sub-tree under that name
called `_options`:

```
tux% pvrDump 'field(a[x=1])'
structure 
    structure field
        structure a
            structure _options
                string x 1

tux% pvrDump 'field(a.b[y=2])'
structure 
    structure field
        structure a
            structure b
                structure _options
                    string y 2

tux% pvrDump 'field(a[x=1].b[y=2])'
structure 
    structure field
        structure a
            structure _options
                string x 1
            structure b
                structure _options
                    string y 2

```

Any `getField()` or `putField()` sections are handled just like the `field()`
section described above.

```
tux% pvrDump 'record[]field()getField()putField()'
structure 
    structure field
    structure getField
    structure putField

tux% pvrDump 'record[x=1]field(a) putField(a),getField(a)'
structure 
    structure record
        structure _options
            string x 1
    structure field
        structure a
    structure getField
        structure a
    structure putField
        structure a

```

## Parsing Issues

The set of characters that are legal in an `optionValue` token is not fixed, and
is slightly different for the `optionList` inside a `record` than for one inside
a `field`, `getField` and/or `putField` token.

Every open brace, parenthesis or bracket character in the input string must
always be balanced with an equivalent closing character. However these
characters don't always have to be valid structurally as long as they appear in
`optionValue` tokens. Some cases are properly rejected, but these examples are
currently accepted:

```
tux% pvrDump 'record[x=a[),y=b(]]'
structure 
    structure record
        structure _options
            string x a[)
            string y b(

tux% pvrDump 'record[x=a)]field(a[y=b(])'
structure 
    structure record
        structure _options
            string x a)
    structure field
        structure a
            structure _options
                string y b(

```

If a `fieldName` contains spaces they are not rejected, but do get removed when
creating the sub-tree for that name:

```
tux% pvrDump 'field(a,b    c,d)'
structure 
    structure field
        structure a
        structure bc
        structure d

```

Errors in the syntax of the input string aren't always reported as such:

```
tux% pvrDump 'field(a{b,c}.d)'
while creating Structure exception null field name 

tux% pvrDump 'field(a{b,c}d)'
Invalid charactor '}' (125) in field name "c}" must be A-Z, a-z, 0-9, or '_'
```

As long as a pvRequest string contains at least one `recordOptions`, `field`,
`getField` and/or `putField` token and all such tokens are syntactically
correct, there can be additional material in the string (outside of those
tokens) which the parser completely ignores. For example this is accepted:

```
tux% pvrDump '][this-can=be*garbagerecord[]I-am(also)ignored+putField()etc.'
structure 
    structure putField

```

## Suggestions

It is trivial to augment the parser so that it also accepts a JSON-formatted
object, while still supporting the existing syntax for backwards compatibility.
A request string can never begin with an open-brace character `{` since the
first component of the string must either be a `fieldName` or one of the
keywords `record`, `field`, `getField` or `putField`. Thus I have a modified
version of `createRequest()` that detects strings that start with `{` and end
with `}` and passes them to an alternative parser.

EPICS Base has included a JSON parser since the first 3.15-series release, and
Michael has already added both `printJSON()` and `parseJSON()` functions to the
pvDataCPP module, so my modified version just passes JSON strings to that. I
also modified my `pvrDump` test program to be able to output the pvRequest
structure from the parser using `printJSON()` with the following results:

```
tux% pvrDump -j 'record[x=1]field(a) putField(a),getField(a)'
{
  "record": {
    "_options": {
      "x": "1"
    }
  },
  "field": {
    "a": {
    }
  },
  "getField": {
    "a": {
    }
  },
  "putField": {
    "a": {
    }
  }
}
```

The JSON version is longer than the original because of JSON's requirement for
quotes around all strings, and because of the explicit `_options` field-name.
Here is the same input displayed on one line, then used as input to the modified
parser to show that it works:

```
tux% pvrDump -j -1 'record[x=1]field(a) putField(a),getField(a)'
{"record": {"_options": {"x": "1"}},"field": {"a": {}},"getField": {"a": {}},"putField": {"a": {}}}

tux% pvrDump '{"record": {"_options": {"x": "1"}},"field": {"a": {}},"getField": {"a": {}},"putField": {"a": {}}}'
structure 
    structure field
        structure a
    structure getField
        structure a
    structure putField
        structure a
    structure record
        structure _options
            string x 1

tux% pvrDump -j -1 value,timeStamp
{"field":{"value":{},"timeStamp":{}}}

tux% pvget -r '{"field":{"value":{},"timeStamp":{}}}' -p ca anj:ai
anj:ai
structure 
    double value 0
    time_t timeStamp 2106-02-07T00:28:16.000 0

```

We could reduce the length of the JSON version by adopting the IOC's "relaxed
JSON" or JSON-5 syntax, which allows quotes to be omitted for strings that only
contain a limited subset of characters. We have already been asked to allow this
to be used elsewhere, but this will require modifying the parser. After doing
that the JSON below will parse into the structure shown above:

```
{record:{_options:{x:1}},field:{a:{}},getField:{a:{}},putField:{a:{}}}
```

The new parser does permit pvRequest structures to be created that were not
possible/legal with the old parser, and this may cause problems with some
servers or providers, for example:

```
tux% pvget -r '{"field":["value","timeStamp"]}' -p ca anj:ai
CA client library tcp receive thread terminating due to C++ exception "anj:ai DbdToPv::activate illegal pvRequest structure 
    string[] field [value,timeStamp]
"
CA.Client.Exception...............................................
    Warning: "Virtual circuit disconnect"
    Context: "tux.aps.anl.gov:5064"
    Source File: ../cac.cpp line 1237
    Current Time: Wed Aug 08 2018 13:09:38.961470693
..................................................................
Timeout
```

Interestingly pvaSrv from EPICS-4.6.0 doesn't crash or show any error messages
when given the above pvRequest, it just returns all known fields.
