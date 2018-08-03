# pvRequest Parser Syntax

This is an attempt to create an EBNF definition of the syntax currently accepted
by the epics::pvData::CreateRequest parser. I developed this by reading through
the code and experimenting, but since the parser does not use normal recursive
descent techniques and has various short-cuts in it I might have missed some
aspects of what it accepts or rejects.

## EBNF Syntax Used

The definitions below use the following syntax:

* Spaces and newlines are only significant inside quotes.
* `=` marks the name preceeding it as a token being defined.
* `;` signifies the end of a token definition.
* `()` are used for grouping tokens.
* `[]` are for declaring character classes.
* `|` separates alternatives.
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
    record[x=a[),y=b(]]
    record[x=a)]field(a[y=b(])
```

As long as a pvRequest string contains at least one `recordOptions`, `field`,
`getField` and/or `putField` token and all such tokens are syntactically
correct, there can be additional material in the string (outside of those
tokens) which the parser completely ignores. For example this is accepted:

```
    ][this-can=be*garbagerecord[]I-am(also)ignored+putField()etc.
```
