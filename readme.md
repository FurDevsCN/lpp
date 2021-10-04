# Welcome to use L++ programming language.

L++ programming language was under the MIT license.  
Copyright(c) 2021 nu11ptr team.  
[How to make & use interpreter](howto_interpreter.md)  
[Programming traps](howto_traps.md)

## Features

### Based on C++ Standard template libraries

You don't need any other library to create a minimal L programming language,such as "iomanip" or "iostream".

### Expansibility

The L++ programming language use lambda function to let you add your own functions.  
Read [How to develop](#how-to-develop) to develop custom commands.

### How to develop

Before adding a custom lambda function,Please read this.

#### The lambda function

A normal command lambda function should be like this:

```
[your_capture](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
						   //something...
					   };
```

**cmd** : this is the user's command.You can use **name**\(std::wstring\) to get the command name,and **args**\(std::vector< std::wstring >\) to get the command arguments.  
**scope** : this is the **current** variable scope.  
**all_scope** : this is the **global** variable scope.  
**this_scope** : this is the variable scope of _this_.

#### Critical functions

##### Lpp::Lpp::exp_calc

```
// Use exp_calc with modifications of scope.
/*
const Variable::var &exp : The expression that will be calculated.
Variable::var &scope : The current scope.
Variable::var &all_scope : The global scope.
Variable::var &this_scope : This scope of this object.
*/
const Variable::var exp_calc(const Variable::var &exp, Variable::var &scope,
                               Variable::var &all_scope,
                               Variable::var &this_scope,
                               const bool newObjectIsConst = false) const;
```

You can use it to **calculate** values.It will return a **const** value.  
Use **cmd.exp_calc** to call it.  
Maybe you want use **parse** function with it.

```
cmd.exp_calc(Variable::parse(something),scope,all_scope,this_scope);
```

##### Lpp::Lpp::get_object

This function may modify the scope.

```
/*
const std::wstring& n : the path(expression) that will be accessed.
Variable::var &scope : the scope.
Variable::var &all_scope : the global scope.
Variable::var &this_scope : the "this" scope.
const bool nonewobject : If nonewobject==true,it will not create new objects(as far as possible).
const bool nonative : If nonative==true,it will not parse native functions,and it will get it's overload's value.
*/
Return_Object get_object(const std::wstring &n, Variable::var &scope,
                           Variable::var &all_scope,
                           Variable::var &this_scope,
                           const bool nonewobject,
                           const bool nonative) const;
```

You can use it to **get** the value by name.Here is the Return_Object's introduction.

```
  typedef enum Object_Type {
    is_pointer = 0,
    is_const_value = 1,
  } Object_Type;
  typedef enum Object_Errors {
    member_cant_visit = 0,
    member_not_exist =
        1,  // only use for null,int,function,expression,array,boolean and
            // native function/members.
  } Object_Errors;
  typedef class Return_Object {
    Variable::var *value;
    Variable::var const_value;
    Variable::var *parent;
    Variable::var const_parent;
    Variable::var *this_object;
    bool lastIsThis;
    bool isNative;

   public:
    const bool lastThis() const { return lastIsThis; }
    const bool native() const { return isNative; }
    Variable::var &getValue() { return *value; }
    Variable::var &getParent() { return *parent; }
    const Variable::var &getConstValue() { return const_value; }
    const Variable::var &getConstParent() { return const_parent; }
    Variable::var &getThis() { return *this_object; }
    Object_Type tp;
  } Return_Object;
```

##### Variable::var

Please see **var.h**::Variable::var() for more informations.

##### Variable::parse

```
/*
const std::wstring &x : The string that will be parsed.
const bool isConst : The parsed value's isConst property.
*/
const var parse(const std::wstring &x, const bool isConst = false);
```

Parse the string to Variable.  
**Warning** The literal string **{}** is an Object,not a Statement block.  
**Info** The literal string **1+1** is an Expression.You can use **exp_calc** to calculate it.

##### Variable::var_tp

Please see **var.h**::Variable::var_tp for more informations.

### Variables

L++ have these types:  
format: sample -> public id(private id) : can convert to : ...  
**null** -> null(Null) : can convert to : nothing.  
**1 0xf 1.2** -> int(Int) : can convert to : Boolean.  
**true false** -> bool(Boolean) : can convert to : Int.  
**"hello world" "\u0032"** -> string(String) : can convert to : Int(parseInt),Array.  
**\[1,2,3\] \[3,4,5\]** -> array(Array) : can convert to : Object.  
**{"object":1}** -> object(Object) : can convert to : Array.  
**function(a,b=1){return "hi";}** -> function(Function) : can convert to : nothing.  
**{return 1;}** -> Unknown(StmtBlock) : can convert to : nothing.  
**1+1** -> Unknown(Expression) : can convert to : nothing.
**tips:**You can use **const** command to make sure a literal has const attribute.

### Type declaration keyword

You can use **\[type\] \[value\]** to convert a value to another type.
sample\[1\]:

```
string "1";#string
string 1;#string
```

### Commands

These commands are the basic L++ interpreter supports.  
tips:\[required\] \(optional=default\)  
**var \[name1\]=\(value1\),\[name2\]=\(value2\),...** : Defines \[name\] \(to value\) \(if \(value\) is not specified,it will default to null\(is not const value\).  
sample\[1\]:

```
var z;#z=null,z.isConst=false
var x=1;#x=1,x.isConst=false
const b=1;#b=1,b.isConst=true
var a=b;#a=1,a.isConst=false
```

**const \[name1\]=\(value1\),\[name2\]=\(value2\),...** : Defines \[name\] \(to value\) \(if \(value\) is not specified,it will default to null\(is const value\).  
sample\[1\]:

```
const z;#z=null,z.isConst=true
const x=1;#x=null,x.isConst=true
var b=1;#b=1,b.isConst=false
const a=b;#a=1,a.isConst=true
```

**delete \[name1\],\[name2\],...** : Deletes \[name\].CANNOT DELETE A CONST VARIABLE!It will return count of successfully deleted.  
sample\[1\]:

```
var a;
delete a;# = 1
a # ExpressionError
const a;
delete a;# = 0
```

**if \[exp\],\[then\]\(,else=\(\)\)** If condition \[exp\] is equal to true,it will execute statement block \[then\].Otherwise,it will execute statement block \(else\) \(if it is specified\).  
sample\[1\]:

```
if 1,{throw 1},{throw 2};#it will throw 1.
if 0,{throw 1},{throw 2};#it will throw 2.
```

**switch \[value\],\[\[comp1={...}\],\[comp2={...}\],...,\(default={...}\)\]** : Switches conditions.  
information\[1\]:If \[value\] is equal to \[comp*n*\],it will execute statement block \[{...}\].  
information\[2\]:If no statement block uses break command or no statement block matches,it will execute statement block \[default\].  
sample\[1\]:

```
var a;
switch 4,[1={
  a=1;
  break;
},2={
  a=2;
  break;
},3={
  a=3;
  break;
},default={
  a="default";
  break;
}];#a="default"
```

**while \[exp\],\[fn\]** : Repeats statement block \[fn\] until the condition \[exp\] is not equal to true.  
sample\[1\]:

```
var a=4,b=0;
while b!=a,{
  b++;
};#it will repeat 4 times.
```

**for \(\[settings\]\),\[fn\]** : Repeats statement block \[fn\].
setting's format:\(start(first time);exp(expression,if it is not equal to true,the loop will not start.If it is not spe);routine(executes after once loop)\)  
sample\[1\]:

```
var a=4,b=0;
for (var i=0;i!=a;i++),{
    b++;
};#after this,variable 'b' will be 4.
```

**continue** : (only can use in **while**,**for**) start next loop.  
**break** : (only can use in **switch**,**while**,**for**) jump out from the loop/case.  
sample\[1\]:

```
while 1,{
    break;#it is not a dead loop.
}
while 1,{
    continue;#it is a dead loop.
}
```

**return \(value=null\)** : Returns \[value\].  
sample\[1\]:

```
return 0;#returns 0.
```

**new \[fn\]\(,arguments=[]\)** : Puts the function in the object and execute it.(_arguments_ must be an **array** and **cannot** be a variable.)  
sample\[1\]:

```
const fn=function(a){
    this["a"]=a;
};
var a=(new fn,[1]);
#a = {"a":1}
```

New feature after **1.4.1-20210921_beta**:If you type a **new**,you can get the new status of this function.  
sample\[2\]:

```
var t=function(){
  this["a"]=(new);
}
var s=(new);#false
var q=(new t);#{"a":true}
```

**\[fn\] \[arguments\]** : Calls function \[fn\] and set variable 'arguments'\(and function argument list\) to \[arguments\].(_arguments_ must be an **array** and **cannot** be a variable.)  
sample\[1\]:

```
var fn=function(a=-1){
    return a;
};
var a,b;
a=(fn []);#a=-1
b=(fn [1]);#b=1
```

**throw \[Exception\]** Throws \[Exception\].  
sample\[1\]:

```
throw 0;#it will throw 0
```

**try \[func\],catch=\[ExceptionFunc\]** : If \[func\] throws any Exception,\[ExceptionFunc\] will catch it(you can use **what** command to get Exception).  
sample\[1\]:

```
var err;
try {throw "error"},catch={err=(what)};#errt="error"
```

**typeof \[var\]** returns the type of \[var\].  
sample\[1\]:

```
var a=(typeof {});#a="object"
```

**eval \[str\]** Executes \[str\] and returns value.  
sample\[1\]:

```
var a=(eval "1");#a=1
var b=(eval {});#b={}
```

**what** Gets the error\(use only in **catch**\).  
sample\[1\]:

```
var s;
try {throw 1;},catch={s=(what)+1;}
# s = 2
```

**char \[ch\]** Allows you convert string<->int.If ch is int,convert it to string;If ch is string,convert it to int.  
sample\[1\]:

```
var s;
s=(char 97);# s = "a"
s=(char 'a');# s = 97
```

**import \[path\]** Imports path as a L++ script.It will be an object and you can use **member operator** to access it's member.path **must** be String.  
sample\[1\]:

```
var a=(import "test.lpp");
a.test [];#...
```

**export \[name\]=\[value\]** Exports value as a name.It will be visibility in the object.  
sample\[1\]:

```
#test.lpp
export a=1;
#main.lpp
var a=(import "test.lpp");
a.a;#1
```

New feature after **1.4.1-20210921_beta**:If you type a **export**,you can get the export status of this script.  
sample\[2\]:

```
#test.lpp
export t=(export);
#main.lpp
var a=(import "test.lpp");
var b=(export);#false
a.t;#true
```

### Extend Commands(use ENABLE_EXT to enable)

**ext \[name\],\[arguments\]** Calls the extend function **name** with **arguments**.name **must** be String.If zero arguments given,then returns true if the extend functions enabled.  
sample\[1\]:

```
# __ext__ functions:
# system [name:string,arguments:array] -> execute a program.
# getline [] -> get a line from stdin.
# print [any,...] -> print a(n) string/object to screen.
ext "print",["Hello World!"];# Hello World!
```

**void \[expr\]** Calculates the **expr** and returns **null**.  
sample\[1\]:
```
1+1;#2
void 1+1;#null
```

### Native Members

using **object.\[the name of the member\]** or **object\[\["the name of the member"\]\]** to access the Native Members.  
**keys : Array -> can use on Object** : get the object's keys.  
**length : Int -> can use on Array/String** : get the string/array's length.  
**isConst : Boolean -> can use on Any** : get the variable's operability.  
**push\(elem:Any\) : Null -> can use on Array** : push a element to the array.  
**pop\(\) : Null -> can use on Array** : pop a element from the array.  
**resize\(new_size:Int\) Null -> can use on Array** : resize the array.  
**insert\(index:Int,elem:Any\) Null -> can use on Array** : insert a element before the index.  
**join\(str:String\) Null -> can use on Array** : put all the elements in the array into a string,split with str.  
**toString\(\) : String -> can use on Any** : get the variable's string.  
**substr\(pos,\(cnt=-1\)\) String -> can use on String** : intercept the string from pos,count cnt.
**trim\(\) String -> can use on String** : remove the blank characters from begin and end.
**split\(str\) Array -> can use on String** : split string by str.
### Overload

You can use this\["..."\] to override a native function.  
You can define operator... to overload a operate\(+,-,\*,etc.\)

### Sample

sample\[a+b\]:

```
const fn=function(a,b){
    return a+b;
};
var a=1,b=2;
return (fn [a,b]);#3
```

sample\[Fibonacci sequence \(recursion\)\]:

```
const fib=function(x){
    if x==0||x==1,{
      return x;
    };
    return (fib [x-1])+(fib [x-2]);
};
var x;
x=(fib [10]);
#x=55
```

sample\[Fibonacci sequence \(not recursion\)\]:

```
const fib=function(n){
    var a=[0,1,1];
    if n<0,{throw "failed!";};
    if n>=3,{
        for (var i=3;i<=n;i++),{
            a[i]=(a[i-1]+a[i-2]);
        }
    };
    return a[n];
};
var x;
x=(fib [10]);
#x=55
```
