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
