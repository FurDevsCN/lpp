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

This overload will not modify the scope.

```
// Use exp_calc without modifications of scope.(Please use this overload as rarely as possible,it will copy the scope,and it is very slow.)
/*
const Variable::var &exp : The expression that will be calculated.
const Variable::var &scope : The current scope.
const Variable::var &all_scope : The global scope.
const Variable::var &this_scope : This scope of this object.
*/
const Variable::var exp_calc(const Variable::var &exp,
                               const Variable::var &scope,
                               const Variable::var &all_scope,
                               const Variable::var &this_scope,
                               const bool newObjectIsConst = false) const;
```

...And this overload will.

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
Return_Object get_object(const std::wstring &n, Variable::var &scope,
                           Variable::var &all_scope, Variable::var &this_scope,
                           const size_t &count_dont_parse,
                           const bool &nonewobject) const;
```

You can use it to **get** the value by name.Here is the Return_Object's introduction.

```
typedef enum Object_Type {
	is_pointer = 0, // this value can modify
  is_const_value = 1, // this value is a const value.
  is_native_function = 2, // this value is a native function(see #native-function).
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
  bool isConst;

 public:
  bool getIsConst() { return isConst; } // if the value is literal or cannot modify,this will return true.
  Variable::var &getValue() { return *value; } // return got value's reference.
  Variable::var &getParent() { return *parent; } // return got value's parent.
  Variable::var getConstValue() { return const_value; } // return got value's const value(recommend).
  Variable::var getConstParent() { return const_parent; } // return got value's const parent(recommend).
  Variable::var &getThis() { return *this_object; } // return got value's this object.
  Object_Type tp;// The type of return's object.
  Return_Object();
  Return_Object(const Variable::var &x, Variable::var *p, Variable::var *s);
  Return_Object(const Variable::var &x, const Variable::var &p,
                  Variable::var *s);
  Return_Object(Variable::var *x, Variable::var *p, Variable::var *s);
  Return_Object(const std::nullptr_t &x, const Variable::var &w,
                  Variable::var p, Variable::var *s);
  Return_Object(const std::nullptr_t &x, const Variable::var &w,
                  Variable::var *p, Variable::var *s);
} Return_Object;
```

##### Variable::var

Please see **var.h**::Variable::var() for more informations.

##### Variable::parse

```
/*
const std::wstring &x : The string that will be parsed.
const bool& isConst : The parsed value's isConst property.
*/
const var parse(const std::wstring &x, const bool& isConst = false);
```

Parse the string to Variable.  
**Warning** The literal string **{}** is an Object,not a Function.  
**Info** The literal string **1+1** is an Expression.You can use **exp_calc** to calculate it.

##### Variable::var_tp

Please see **var.h**::Variable::var_tp for more informations.

### Variables

L++ have these types:  
format: sample -> public id(private id) : can convert to : ...  
**null** -> null(Null) : can convert to : nothing.  
**1 0xf 1.2** -> int(Int) : can convert to : Boolean.  
**true false** -> boolean(Boolean) : can convert to : Int.  
**"hello world" "\u0032"** -> string(String) : can convert to : Array.  
**\[1,2,3\] \[3,4,5\]** -> array(Array) : can convert to : Object.  
**{"object":1}** -> object(Object) : can convert to : nothing.  
**{return 1;}** -> function(Function) : can convert to : nothing.  
**tips:**You can use **const** command to make sure a literal has const attribute.

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
information\[2\]:If no statment block uses break command or no statement block matches,it will execute statement block \[default\].  
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
  b+=1;
};#it will repeat 4 times.
```

**for \(\[settings\]\),\[fn\]** : Repeats statement block \[fn\].
setting's format:\(start(first time);exp(expression,if it is not equal to true,the loop will not start.If it is not spe);routine(executes after once loop)\)  
sample\[1\]:

```
var a=4,b=0;
for (var i=0;i!=a;i+=1),{
    b+=1;
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

**new \[fn\]\(,arguments=[]\)** : Puts the function in the object and execute it.  
sample\[1\]:

```
const fn={
    this["a"]=arguments[0];
};
var a=(new fn,[1]);
#a = {"a":1}
```

**fn \[arguments\]** : Calls function \[fn\] and set variable 'arguments' to \[arguments\].  
sample\[1\]:

```
var fn={
    if arguments.length!=1,{
        return -1;
    };
    return arguments[0];
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

**try \[func\],catch=\[ExceptionFunc\]** : If \[func\] throws any Exception,\[ExceptionFunc\] will catch it(Exception is stored in variable 'err').  
sample\[1\]:

```
var errors_last;
try {throw "error"},catch={errors_last=err};#errors_last="error"
```

**typeof \[var\]** returns the type of \[var\].  
sample\[1\]:

```
var a=(typeof {});#a="object"
```

**eval \[str\]** Executes \[str\] and returns value.str must be a String.  
sample\[1\]:

```
var a=(eval "1");#a=1
```

### Extend Commands(use ENABLE_EXT to enable)

**load \[path\]** Bind path as a function.You can call function to call it(with arguments).path **must** be String.  
sample\[1\]:

```
var a=(load "/bin/echo");
a ["Hello World!"];# Hello World!
```

**import \[path\]** import path as a L++ script.It will be a object and you can use **member operator** to access it's member.path **must** be String.  
sample\[1\]:

```
var a=(import "test.lpp");
a.test [];#...
```

### Native Members

using **object.\[the name of the member\]** or **object\[\["the name of the member"\]\]** to access the Native Members.  
**keys : Array -> can use on Object** : get the object's keys.  
**length : Int -> can use on Array/String** : get the string/array's length.  
**isConst : Boolean -> can use on Any** : get the variable's operability.  
**convert\(new_type:String\) : Any -> can use on Any** : convert variable to new type.  
**push\(elem:Any\) : Null -> can use on Array** : push a element to the array.  
**pop\(\) : Null -> can use on Array** : pop a element from the array.  
**resize\(new_size:Int\) Null -> can use on Array** : resize the array.  
**insert\(index:Int,elem:Any\) Null -> can use on Array** : insert a element before the index.  
**join\(str:String\) Null -> can use on Array** : put all the elements in the array into a string,split with str.  
**toString\(\) : String -> can use on Any** : get the variable's string.  
**substr\(start,\(end=-1\)\) String -> can use on String** : intercept the string from start,count end.

### Sample

sample\[a+b\]:

```
const fn={
    if arguments.length!=2,{
        throw "failed";
    }
    return arguments[0]+arguments[1];
};
var a=1,b=2;
var final;
final=(fn [a,b]);
return final;#3
```

sample\[Fibonacci sequence \(recursion\)\]:

```
const fib={
    if arguments==[0]||arguments==[1],{
      return arguments;
    }
    var x,y;
    x=(fib [arguments[0]-1]);
    y=(fib [arguments[0]-2]);
    return x+y;
};
var x;
x=(fib [10]);
#x=55
```

sample\[Fibonacci sequence \(not recursion\)\]:

```
const fib={
    var a=[0,1,1],n=arguments[0];
    if n<0,{throw "failed!";}
    if n>=3,{
        for (var i=3;i<=n;i++),{
            a[i]=(a[i-1]+a[i-2]);
        }
    }
    return a[n];
};
var x;
x=(fib [10]);
#x=55
```
