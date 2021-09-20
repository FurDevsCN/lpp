# This is the introduction of L++'s programming traps.

L++ programming language was under the MIT license.  
Copyright(c) 2021 nu11ptr team.

## Is it an object or a statement block?

Here is the example:

```
var a={};
typeof a;#???
```

The answer is,it is an **object**.  
\(But you can use it in if,switch,while,for or try...I optimize it.\)  
You can use **{;}** to get a **void statement**.

## ++'s "bug"

```
var a=1;
a++;# 1
++a;# ExpressionError!?
```

It is a feature\(my fault\) in L++.**++a** is not exist.

## Expression joke

```
var a=function(){return "hello";}
var b;
b=a [] # b == "hello"?
```

Actually,**b = function(){return "hello";}**.You **must** use **b=(a [])** to get a function's return value.
