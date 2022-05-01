# 这是部分现有语法问题的介绍。

## 前自增问题

```
var a=1;
a++;# 1
++a;# ExpressionError!?
```

L++旧版本的设计机制使得前自增不能被支持。重构后的新版本将考虑支持前自增。

## 调用被吞了！？

```
var a=function(){return "hello";}
var b;
b=a [] # b == "hello"?
```

实际上，**b = function(){return "hello";}**。
您必须使用**b=(a [])**来获得返回值。
此问题将在新版被修复。请期待。
