/*
L++ was under the MIT license.
Copyright(c) 2022 FurDevsCN.
*/
#ifndef _VAR_MODULE
#define _VAR_MODULE
#include <cmath>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
/*
Var命名空间存储了各种有关变量的实现。
*/
namespace Var {
/*
enum class Error;
用于存储var.h内所有标准错误。
*/
typedef enum class Error {
  DivError = 0,    // 除以0操作。
  ExprError = 1,   // 表达式语法错误。
  ConvError = 2,   // 变量类型转换失败。
  CalcError = 3,   // 计算操作发生错误。
  TypeError = 4,   // 类型不正确。
  IndexError = 5,  // 下标不正确。
  OpError = 6      // 运算符不存在。
} Error;
/*
bool isCoveredWith(const std::wstring& p, const wchar_t left,
                   const wchar_t right);
如果原串p的最外层是被字符left和字符right包括的，那么返回true，否则返回false。
*/
bool isCoveredWith(const std::wstring& p, const wchar_t left,
                   const wchar_t right) {
  if (p.length() < 2 || p[0] != left || p[p.length() - 1] != right)
    return false;
  for (size_t i = 0, z = 0, a = 0, j = 0; i < p.length(); i++) {
    if (p[i] == L'\\')
      z = !z;
    else if (p[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (p[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
      j++;
    else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
      j--;
    if (p[i] == right && a == 0 && j == 0) {
      if (i != p.length() - 1) return false;
      return true;
    }
  }
  return false;
}
/*
Exp命名空间存储了关于表达式解析部分的实现。
*/
namespace Exp {
// 中缀表达式数组的别名。
typedef std::vector<std::wstring> ExpType;
/*
enum NodeType;
存储节点的类型。
Exp：此节点是一个表达式。
Value：此节点是一个值。
*/
typedef enum NodeType { Exp = 0, Value = 1 } NodeType;
/*
class Node;
对表达式树节点的定义。
*/
typedef class Node {
  std::shared_ptr<Node> _l;
  std::shared_ptr<Node> _r;

 public:
  // 该节点的值。当tp为Value时，该成员存储值的字面量；当tp为Exp时，该成员存储运算符。
  std::wstring val;
  // 该节点的类型。
  NodeType tp;
  /*
  const Node& l() const;
  获得节点左边的子节点。
  */
  const Node& l() const { return *_l; }
  /*
  const Node& r() const;
  获得节点右边的子节点。
  */
  const Node& r() const { return *_r; }
  Node() { tp = Value; }
  /*
  Node(const std::wstring& _val, const Node& __l, const Node& __r);
  将_val作为本节点的操作符，__l作为左侧子节点，__r作为右侧子节点，构造一个表达式节点。
  */
  Node(const std::wstring& _val, const Node& __l, const Node& __r)
      : _l(new Node(__l)), _r(new Node(__r)), val(_val), tp(Exp) {}
  /*
  explicit Node(const std::wstring& _val);
  将_val作为本节点的值的字面量，构造一个值节点。
  */
  explicit Node(const std::wstring& _val) : val(_val), tp(Value) {}
} Node;
/*
int getprio(const std::wstring& op);
获得运算符op的优先级。
注意：返回-1表示不存在此操作符。
*/
int getprio(const std::wstring& op) {
  if (op == L"+") return 1;
  if (op == L",") return 0;
  if (op == L"-") return 1;
  if (op == L"*") return 3;
  if (op == L"/") return 3;
  if (op == L"%") return 3;
  if (op == L":") return 2;
  if (op == L"?") return 0;
  if (op == L"==") return 1;
  if (op == L"===") return 1;
  if (op == L"!==") return 1;
  if (op == L"=") return 0;
  if (op == L"+=") return 0;
  if (op == L"-=") return 0;
  if (op == L"*=") return 0;
  if (op == L"/=") return 0;
  if (op == L"%=") return 0;
  if (op == L"|=") return 0;
  if (op == L"&=") return 0;
  if (op == L"^=") return 0;
  if (op == L">>=") return 0;
  if (op == L">>>=") return 0;
  if (op == L"<<=") return 0;
  if (op == L"++") return 0;
  if (op == L"--") return 0;
  if (op == L"!=") return 1;
  if (op == L">") return 1;
  if (op == L"<") return 1;
  if (op == L">=") return 1;
  if (op == L"<=") return 1;
  if (op == L"&&") return 0;
  if (op == L"||") return 0;
  if (op == L"!") return 0;
  if (op == L">>") return 4;
  if (op == L"<<") return 4;
  if (op == L"^") return 4;
  if (op == L"&") return 4;
  if (op == L"~") return 4;
  if (op == L"|") return 4;
  if (op == L"!") return 4;
  if (op == L">>>") return 4;
  return -1;
}
/*
ExpType unbrace(ExpType c);
删除已分割的中缀表达式存储数组c的不必要的括号。
*/
ExpType unbrace(ExpType c) {
  while (c.size() > 1 && c[0] == L"(" && c[c.size() - 1] == L")") {
    bool flag = false;
    for (size_t i = 0, j = 0; i < c.size(); i++) {
      if (c[i] == L"(")
        j++;
      else if (c[i] == L")")
        j--;
      if (getprio(c[i]) != -1 && j == 0) {
        flag = true;
        break;
      }
    }
    if (flag) break;
    c.erase(c.begin());
    c.erase(c.end() - 1);
  }
  return c;
}
/*
ExpType split(const std::wstring& p);
将原串p转换为分割后的中缀表达式。
*/
ExpType split(const std::wstring& p) {
  size_t a = 0;
  int f = 0;
  bool x = false, z = false;
  ExpType ret;
  std::wstring temp;
  for (size_t i = 0; i < p.length(); i++) {
    switch (p[i]) {
      case L'\\':
      case L'\'':
      case L'\"': {
        if (p[i] == L'\\')
          z = !z;
        else if (p[i] == L'\"' && !z) {
          if (a == 0 || a == 1) a = !a;
        } else if (p[i] == L'\'' && !z) {
          if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
        } else
          z = 0;
        temp += p[i];
        break;
      }
      case L'>':
      case L'&':
      case L'^':
      case L':':
      case L'?':
      case L'|':
      case L'<': {
        if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          if (ret.empty()) throw Error::ExprError;
          if (ret[ret.size() - 1][ret[ret.size() - 1].length() - 1] == p[i])
            ret[ret.size() - 1] += p[i];
          else
            ret.push_back({p[i]});
          x = false;
        } else
          temp += p[i];
        break;
      }
      case L'=': {
        if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          if (ret.empty()) throw Error::ExprError;
          if (Exp::getprio(ret[ret.size() - 1]) != -1)
            ret[ret.size() - 1] += p[i];
          else
            ret.push_back({p[i]});
          x = false;
        } else
          temp += p[i];
        break;
      }
      case L'+':
      case L'-': {
        if (a == 0 && f == 0 && (!ret.empty()) &&
            ret[ret.size() - 1] != std::wstring({p[i]}) &&
            ret[ret.size() - 1] != L")")
          ret[ret.size() - 1] += p[i];
        else if (a == 0 && f == 0 && x) {
          if (ret.empty()) throw Error::ExprError;
          ret[ret.size() - 1] += p[i];
        } else if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
            x = true;
          }
          ret.push_back({p[i]});
        } else
          temp += p[i];
        break;
      }
      case L'~':
      case L'!':
      case L'*':
      case L'/':
      case L',':
      case L'%': {
        if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          x = true;
          ret.push_back({p[i]});
        } else
          temp += p[i];
        break;
      }
      case L'(': {
        if (temp != L"" && a == 0 && f == 0) {
          temp += L'(';
          x = true;
          f++;
          break;
        } else if (a == 0 && f == 0) {
          if (temp != L"") ret.push_back(temp), temp = L"";
          f++;
          ret.push_back(L"(");
        } else {
          if (a == 0) f++;
          temp += L'(';
        }
        break;
      }
      case L')': {
        if (temp != L"" && a == 0 && f == 1 && x) {
          temp += L')';
          x = false;
          f--;
        } else if (a == 0 && f == 1) {
          if (temp != L"") ret.push_back(temp), temp = L"";
          f--;
          ret.push_back(L")");
        } else {
          if (a == 0) f--;
          temp += L')';
        }
        break;
      }
      case L'{':
      case L'[': {
        if (a == 0) f++;
        temp += p[i];
        break;
      }
      case L'}':
      case L']': {
        if (a == 0) f--;
        temp += p[i];
        break;
      }
      default: {
        temp += p[i];
        break;
      }
    }
    if (p[i] != L'\\') z = false;
  }
  if (f != 0) throw Error::ExprError;
  if (a != 0) throw Error::ExprError;
  if (temp != L"") ret.push_back(temp);
  if (Exp::getprio(ret[ret.size() - 1]) != -1 && ret[ret.size() - 1] != L")")
    throw Error::ExprError;
  return ret;
}
/*
Node gen(const ExpType& q);
由分割后的中缀表达式q得到表达式树。
*/
Node gen(const ExpType& q) {
  int minpr = 99999;
  size_t minindex = 0;
  const ExpType& c = unbrace(q);
  if (c.empty()) return Node();
  if (c.size() == 1) return Node(c[0]);
  size_t j = 0;
  for (size_t i = 0; i < c.size(); i++) {
    if (c[i] == L"(")
      j++;
    else if (c[i] == L")")
      j--;
    int s = getprio(c[i]);
    if (s != -1 && j == 0 && s <= minpr) {
      minpr = s;
      minindex = i;
    }
  }
  if (j != 0) throw Error::ExprError;
  if (minpr == 99999) {
    std::wstring temp;
    for (size_t i = 0; i < q.size(); i++) temp += q[i];
    return Node(temp);
  }
  return Node(c[minindex], gen(ExpType(c.cbegin(), c.cbegin() + minindex)),
              gen(ExpType(c.cbegin() + minindex + 1, c.cend())));
}
/*
bool isexp(const std::wstring& p);
如果原串p是一个表达式，那么返回true，否则返回false。
*/
bool isexp(const std::wstring& p) {
  if (isCoveredWith(p, L'(', L')')) return true;
  for (size_t i = 0, j = 0, a = 0, z = 0; i < p.length(); i++) {
    if (p[i] == L'\\')
      z = !z;
    else if (p[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (p[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
      j++;
    else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
      j--;
    else if (Exp::getprio({p[i]}) != -1 &&
             (i == 0 || (p[i - 1] != L'e' && p[i - 1] != L'E')) &&
             (i != 0 || p[i] != L'-') && a == 0 && j == 0)
      return true;
  }
  return false;
}
}  // namespace Exp

/*
wchar_t Unicode2String(const std::wstring& str);
将由std::wstring表示的十六进制Unicode字符码str转换为wchar_t。
*/
wchar_t Unicode2String(const std::wstring& str) {
  if (str == L"") return L'\0';
  return (wchar_t)std::stoi(str, 0, 16);
}
/*
std::wstring clearnull(const std::wstring& x);
删除原串x的空白字符。
*/
std::wstring clearnull(const std::wstring& x) {
  std::wstring tmp;
  bool flag = false;
  for (size_t i = 0, a = 0, z = 0, j = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (x[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if (x[i] == L'\r') continue;
    if (x[i] == L'\t' && a == 0) continue;
    if (x[i] == L'\n' && a == 0 && j == 0)
      continue;
    else if (x[i] == L' ' && a == 0 && j == 0 && flag) {
      flag = false;
      continue;
    } else if ((x[i] == L'[' || x[i] == L'{' || x[i] == L'(') && a == 0)
      j++;
    else if ((x[i] == L']' || x[i] == L'}' || x[i] == L')') && a == 0)
      j--;
    if (x[i] == L'[' || x[i] == L'(' || x[i] == L'{' || x[i] == L' ')
      flag = true;
    else
      flag = false;
    tmp += x[i];
  }
  return tmp;
}
/*
std::vector<std::wstring> codeSplit(const std::wstring& x);
将多行代码原串x分割为语句列表并去除空白字符。
*/
std::vector<std::wstring> codeSplit(const std::wstring& x) {
  std::wstring p;
  for (size_t i = 0, a = 0, j = 0, z = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (x[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if (x[i] == L'#' && a == 0) {
      while (i < x.length() && p[i] != L'\n') i++;
      continue;
    }
    if (x[i] == L'\n' && a == 0 && j == 0)
      p += ';';
    else if ((x[i] == L'[' || x[i] == L'{' || x[i] == L'(') && a == 0)
      j++;
    else if ((x[i] == L']' || x[i] == L'}' || x[i] == L')') && a == 0)
      j--;
    if (x[i] != L'\n')
      p += x[i];
    else if (a == 0 && j != 0)
      p += x[i];
  }
  std::vector<std::wstring> ret;
  std::wstring temp;
  for (size_t i = 0, j = 0, a = 0, z = 0; i < p.length(); i++) {
    if (p[i] == L'\\')
      z = !z;
    else if (p[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (p[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if (p[i] == L';' && a == 0 && j == 0)
      ret.push_back(temp), temp = L"";
    else if ((p[i] == L'[' || p[i] == L'{' || p[i] == L'(') && a == 0)
      j++;
    else if ((p[i] == L']' || p[i] == L'}' || p[i] == L')') && a == 0)
      j--;
    if (p[i] == L';' && a == 0 && j == 0)
      continue;
    else
      temp += p[i];
  }
  if (temp != L"") ret.push_back(temp);
  for (size_t i = 0; i < ret.size(); i++) {
    size_t j = 0;
    while (j < ret[i].length() && ret[i][j] == L' ') j++;
    ret[i] = ret[i].substr(j);
  }
  std::vector<std::wstring> fin;
  for (size_t i = 0; i < ret.size(); i++) {
    if (ret[i] != L"" && ret[i] != L";") fin.push_back(ret[i]);
  }
  return fin;
}
/*
typedef enum class _ValueType;
真正用于表示变量类型的枚举类。
不推荐使用此类。
*/
typedef enum class _ValueType {
  _Null = 0,        // 空值
  _Boolean = 1,     // 布尔值
  _Number = 2,      // 数字
  _String = 3,      // 字符串
  _Array = 4,       // 数组
  _Object = 5,      // 对象
  _Function = 6,    // 函数
  _Statement = 7,   // 语句块
  _Expression = 8,  // 表达式
} _ValueType;
// 以下是类型指示语法糖的定义。
typedef struct _NullType {
  constexpr operator _ValueType() const { return _ValueType::_Null; }
} _NullType;
typedef struct _BooleanType {
  constexpr operator _ValueType() const { return _ValueType::_Boolean; }
} _BooleanType;
typedef struct _NumberType {
  constexpr operator _ValueType() const { return _ValueType::_Number; }
} _NumberType;
typedef struct _StringType {
  constexpr operator _ValueType() const { return _ValueType::_String; }
} _StringType;
typedef struct _ArrayType {
  constexpr operator _ValueType() const { return _ValueType::_Array; }
} _ArrayType;
typedef struct _ObjectType {
  constexpr operator _ValueType() const { return _ValueType::_Object; }
} _ObjectType;
typedef struct _StatementType {
  constexpr operator _ValueType() const { return _ValueType::_Statement; }
} _StatementType;
typedef struct _FunctionType {
  constexpr operator _ValueType() const { return _ValueType::_Function; }
} _FunctionType;
typedef struct _ExpressionType {
  constexpr operator _ValueType() const { return _ValueType::_Expression; }
} _ExpressionType;
/*
constexpr _NullType Null;
constexpr _BooleanType Boolean;
constexpr _NumberType Number;
constexpr _StringType String;
constexpr _ArrayType Array;
constexpr _ObjectType Object;
constexpr _StatementType Statement;
constexpr _FunctionType Function;
constexpr _ExpressionType Expression;

语法糖。用于var类的访问重载。
可以利用它们来代替_ValueType。
*/
// 对于类型指示符的定义。
constexpr _NullType Null;              // 空值
constexpr _BooleanType Boolean;        // 布尔值
constexpr _NumberType Number;          // 数字
constexpr _StringType String;          // 字符串
constexpr _ArrayType Array;            // 数组
constexpr _ObjectType Object;          // 对象
constexpr _FunctionType Function;      // 函数
constexpr _StatementType Statement;    // 语句块
constexpr _ExpressionType Expression;  // 表达式
/*
std::vector<std::wstring> splitBy(const std::wstring& x,
                                        const wchar_t& delim);
以字符delim来分割原串x。
*/
std::vector<std::wstring> splitBy(const std::wstring& x,
                                        const wchar_t delim) {
  std::vector<std::wstring> ret;
  std::wstring temp;
  for (size_t i = 0, a = 0, j = 0, z = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (x[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if ((x[i] == L'(' || x[i] == L'{' || x[i] == L'[') && a == 0)
      j++;
    else if ((x[i] == L')' || x[i] == L'}' || x[i] == L']') && a == 0)
      j--;
    if (x[i] == delim && a == 0 && j == 0) {
      ret.push_back(temp), temp = L"";
    } else
      temp += x[i];
  }
  if (temp != L"") ret.push_back(temp);
  return ret;
}
/*
struct var;
用于存储变量内容。
*/
typedef struct var {
  /*
  struct StmtVal;
  负责语句块的存储。
  */
  typedef struct StmtVal {
    // 语句列表。
    std::vector<std::wstring> value;
    StmtVal() {}
    /*
    explicit StmtVal(const std::wstring& x);
    以x作为语句列表并构造类。
    */
    explicit StmtVal(const std::vector<std::wstring>& x) : value(x) {}
  } StmtVal;
  /*
  struct FuncValue;
  用于存储函数体。
  */
  typedef struct FuncValue {
    /*
    struct ArgItem;
    用于存储单个参数。
    */
    typedef struct ArgItem {
      // 参数名。
      std::wstring name;
      // 默认值。为空表示无默认值。
      std::wstring value;
      ArgItem() {}
      /*
      ArgItem(const std::wstring& _name, const std::wstring& _value =
      L""); 以_name作为参数名，以_value作为默认值（如果有），构造类。
      */
      ArgItem(const std::wstring& _name, const std::wstring& _value = L"")
          : name(_name), value(_value) {}
      /*
      std::wstring toString() const;
      将内容转换回原串。
      */
      std::wstring toString() const {
        return name + (value != L"" ? L"=" + value : L"");
      }
      /*
      bool operator==(const ArgItem& comp) const;
      操作符重载。比较两个参数是否相同。
      */
      bool operator==(const ArgItem& comp) const {
        return name == comp.name && value == comp.value;
      }
    } ArgItem;
    // 参数表。
    std::vector<ArgItem> args;
    // 函数体。
    StmtVal block;
    FuncValue() {}
    /*
    FuncValue(const std::vector<ArgItem>& _args, const StmtVal&
    _block); 以_args作为参数表，_block作为函数体，构建类。
    */
    FuncValue(const std::vector<ArgItem>& _args, const StmtVal& _block)
        : args(_args), block(_block) {}
  } FuncValue;
  // 常量标记（当此成员为true时，变量只读）。
  bool constant;
  // 变量类型。
  _ValueType tp;
  var() : constant(true), tp(Null) {}
  /*
  var(const std::nullptr_t&,bool c = true);
  创建一个空值，常量标记为c。
  */
  var(const std::nullptr_t&, bool c = true) : constant(c), tp(Null) {}
  /*
  var(bool x,bool c = true);
  由x创建一个布尔值，常量标记为c。
  */
  var(bool x, bool c = true)
      : constant(c), tp(Boolean), BooleanValue(x) {}
  /*
  var(double x,bool c = true);
  var(int x,bool c = true);
  由x创建一个数字，常量标记为c。
  */
  var(double x, bool c = true)
      : constant(c), tp(Number), NumberValue(x) {}
  var(int x, bool c = true)
      : constant(c), tp(Number), NumberValue(x) {}
  /*
  var(const std::string& x,bool c = true);
  var(const wchar_t* const x, bool c = true);
  由x创建一个字符串，常量标记为c。
  */
  var(const std::wstring& x, bool c = true)
      : constant(c), tp(String), StringValue(x) {}
  var(const wchar_t* const x, bool c = true)
      : constant(c), tp(String), StringValue(x) {}
  /*
  var(const std::vector<var>& x, bool c = true);
  由x创建一个数组，常量标记为c。
  */
  var(const std::vector<var>& x, bool c = true)
      : constant(c), tp(Array), ArrayValue(x) {}
  /*
  var(const std::map<std::wstring, var>& x, bool c = true);
  由x创建一个对象，常量标记为c。
  */
  var(const std::map<std::wstring, var>& x, bool c = true) {
    ObjectValue = x;
    tp = Object;
    constant = c;
  }
  /*
  var(const StmtVal& x);
  由x创建一个语句块。
  */
  var(const StmtVal& x) : constant(true), tp(Statement), StatementValue(x) {}
  /*
  var(const FuncValue& x, bool c = true);
  由x创建一个函数，常量标记为c。
  */
  var(const FuncValue& x, bool c = true)
      : constant(c), tp(Function), FunctionValue(x) {}
  /*
  var(const std::vector<std::wstring>& x, bool c = true);
  由x创建一个表达式，常量标记为c。
  */
  var(const Exp::Node& x)
      : constant(true), tp(Expression), ExpressionValue(x) {}
  /*
  std::nullptr_t operator()(const _NullType&) const;
  bool& operator()(const _BooleanType&);
  double& operator()(const _NumberType&);
  std::wstring& operator()(const _StringType&);
  std::vector<var>& operator()(const _ArrayType&);
  std::map<std::wstring, var>& operator()(const _ObjectType&);
  ...
  以类型标识符来获得内容的引用。
  当类型不符合时，会抛出Error::TypeError。
  */
  std::nullptr_t operator()(const _NullType&) const {
    if (tp == Null)
      return nullptr;
    else
      throw Error::TypeError;
  }
  bool& operator()(const _BooleanType&) {
    if (tp == Boolean)
      return BooleanValue;
    else
      throw Error::TypeError;
  }
  double& operator()(const _NumberType&) {
    if (tp == Number)
      return NumberValue;
    else
      throw Error::TypeError;
  }
  std::wstring& operator()(const _StringType&) {
    if (tp == String)
      return StringValue;
    else
      throw Error::TypeError;
  }
  std::vector<var>& operator()(const _ArrayType&) {
    if (tp == Array)
      return ArrayValue;
    else
      throw Error::TypeError;
  }
  std::map<std::wstring, var>& operator()(const _ObjectType&) {
    if (tp == Object)
      return ObjectValue;
    else
      throw Error::TypeError;
  }
  StmtVal& operator()(const _StatementType&) {
    if (tp == Statement)
      return StatementValue;
    else
      throw Error::TypeError;
  }
  FuncValue& operator()(const _FunctionType&) {
    if (tp == Function)
      return FunctionValue;
    else
      throw Error::TypeError;
  }
  Exp::Node& operator()(const _ExpressionType&) {
    if (tp == Expression)
      return ExpressionValue;
    else
      throw Error::TypeError;
  }
  /*
  std::nullptr_t operator()(const _NullType&) const;
  const bool& operator()(const _BooleanType&) const;
  const double& operator()(const _NumberType&) const;
  const std::wstring& operator()(const _StringType&) const;
  const std::vector<var>& operator()(const _ArrayType&) const;
  const std::map<std::wstring, var>& operator()(const _ObjectType&) const;
  ...
  以类型标识符来获得内容的常量值。
  当类型不符合时，会抛出Error::TypeError。
  */
  const bool& operator()(const _BooleanType&) const {
    if (tp == Boolean)
      return BooleanValue;
    else
      throw Error::TypeError;
  }
  const double& operator()(const _NumberType&) const {
    if (tp == Number)
      return NumberValue;
    else
      throw Error::TypeError;
  }
  const std::wstring& operator()(const _StringType&) const {
    if (tp == String)
      return StringValue;
    else
      throw Error::TypeError;
  }
  const std::vector<var>& operator()(const _ArrayType&) const {
    if (tp == Array)
      return ArrayValue;
    else
      throw Error::TypeError;
  }
  const std::map<std::wstring, var>& operator()(const _ObjectType&) const {
    if (tp == Object)
      return ObjectValue;
    else
      throw Error::TypeError;
  }
  const StmtVal& operator()(const _StatementType&) const {
    if (tp == Statement)
      return StatementValue;
    else
      throw Error::TypeError;
  }
  const FuncValue& operator()(const _FunctionType&) const {
    if (tp == Function)
      return FunctionValue;
    else
      throw Error::TypeError;
  }
  const Exp::Node& operator()(const _ExpressionType&) const {
    if (tp == Expression)
      return ExpressionValue;
    else
      throw Error::TypeError;
  }
  /*
  std::set<FuncValue>& constructor();
  const std::set<FuncValue>& constructor() const;
  获得继承函数集合。
  */
  std::set<FuncValue>& constructor() { return _constructor; }
  const std::set<FuncValue>& constructor() const { return _constructor; }
  /*
  std::wstring toString() const;
  转换变量到字面量。
  */
  std::wstring toString() const {
    std::wstring tmp;
    switch (tp) {
      case Null:
        return L"null";
      case Number: {
        if (NumberValue == (int)NumberValue)
          return std::to_wstring((int)NumberValue);
        return std::to_wstring(NumberValue);
      }
      case Boolean:
        return BooleanValue ? L"true" : L"false";
      case String: {
        tmp = L"\"";
        for (size_t i = 0; i < StringValue.length(); i++) {
          switch (StringValue[i]) {
            case L'\b': {
              tmp += L"\\b";
              break;
            }
            case L'\f': {
              tmp += L"\\f";
              break;
            }
            case L'\r': {
              tmp += L"\\r";
              break;
            }
            case L'\t': {
              tmp += L"\\t";
              break;
            }
            case L'\v': {
              tmp += L"\\v";
              break;
            }
            case L'\0': {
              tmp += L"\\0";
              break;
            }
            case L'\a': {
              tmp += L"\\a";
              break;
            }
            case L'\n': {
              tmp += L"\\n";
              break;
            }
            case L'\\':
            case L'\'':
            case L'\"': {
              tmp += std::wstring(L"\\") + StringValue[i];
              break;
            }
            default: {
              tmp += StringValue[i];
              break;
            }
          }
        }
        return tmp + L"\"";
      }
      case Array: {
        tmp = L"[";
        for (size_t i = 0; i < ArrayValue.size(); i++) {
          tmp += ArrayValue[i].toString();
          if (i + 1 < ArrayValue.size()) tmp += L",";
        }
        return tmp + L"]";
      }
      case Object: {
        tmp = L"{";
        for (std::map<std::wstring, var>::const_iterator it =
                 ObjectValue.cbegin();
             it != ObjectValue.cend(); it++) {
          if (it->first == L"__constructor__") continue;
          tmp += var(it->first).toString() + L":";
          tmp += it->second.toString();
          if ((++std::map<std::wstring, var>::const_iterator(it)) !=
                  ObjectValue.cend() &&
              (++std::map<std::wstring, var>::const_iterator(it))->first !=
                  L"__constructor__")
            tmp += L",";
        }
        return tmp + L"}";
      }
      case Function: {
        tmp = L"func(";
        for (size_t i = 0; i < FunctionValue.args.size(); i++) {
          tmp += FunctionValue.args[i].toString() + L",";
        }
        if (FunctionValue.args.size() == 0)
          tmp += L"){";
        else
          tmp = tmp.substr(0, tmp.length() - 1) + L"){";
         for (size_t i = 0; i < FunctionValue.block.value.size(); i++) {
           if (FunctionValue.block.value[i] != L"")
             tmp += FunctionValue.block.value[i] + L";";
         }
        return tmp + L"}";
      }
      default: {
        return L"null";
      }
    }
  }
  /*
  var convert(const _ValueType& op) const;
  将变量转换到类型op。
  如果转换失败则抛出Error::ConvError。
  */
  var convert(const _ValueType& type) const {
    if (tp == type)
      return *this;
    else if (type == String)
      return toString();
    else {
      switch (tp) {
        case Number: {
          if (type == Boolean) return (bool)NumberValue;
          break;
        }
        case Boolean: {
          if (type == Number) return (int)BooleanValue;
          break;
        }
        case String: {
          if (type == Array) {
            std::vector<var> ret(StringValue.length());
            for (size_t i = 0; i < StringValue.length(); i++) {
              ret[i] = {StringValue[i]};
            }
            return ret;
          } else if (type == Number) {
            if (StringValue.find_first_of('.') == std::wstring::npos &&
                StringValue.find_first_of('e') == std::wstring::npos)
              return std::stoi(StringValue, 0, 0);
            else
              return std::stod(StringValue);
          }
          break;
        }
        case Array: {
          if (type == Object) {
            std::map<std::wstring, var> ret;
            for (size_t i = 0; i < ArrayValue.size(); i++) {
              ret[std::to_wstring(i)] = ArrayValue[i];
            }
            return ret;
          }
          break;
        }
        case Object: {
          if (type == Array) {
            std::vector<var> ret;
            for (std::map<std::wstring, var>::const_iterator i =
                     ObjectValue.cbegin();
                 i != ObjectValue.cend(); i++) {
              ret.push_back(i->first);
            }
            return ret;
          }
          break;
        }
        default:
          break;
      }
    }
    throw Error::ConvError;
  }
  /*
  var opcall(const std::wstring& op) const;
  进行单目运算符运算，运算符为op。
  如果不能计算则抛出Error::CalcError。
  如果不是已知的运算符则抛出Error::OpError。
  */
  var opcall(const std::wstring& op) const {
    if (op == L"~") {
      switch (tp) {
        case Number:
          return ~(int)NumberValue;
        default:
          throw Error::CalcError;
      }
    } else if (op == L"-") {
      switch (tp) {
        case Number:
          return -NumberValue;
        case Boolean:
          return -(int)BooleanValue;
        default:
          throw Error::CalcError;
      }
    } else if (op == L"+") {
      switch (tp) {
        case Number:
          return NumberValue;
        case Boolean:
          return BooleanValue;
        default:
          throw Error::CalcError;
      }
    } else if (op == L"!") {
      switch (tp) {
        case Number:
          return !(bool)NumberValue;
        case Boolean:
          return !BooleanValue;
        default:
          return false;
      }
    } else
      throw Error::OpError;
  }
  /*
  var opcall(const std::wstring& op, const var& value) const;
  进行双目运算符运算，运算符为op，操作数为value。
  如果不能计算则抛出Error::CalcError。
  如果不是已知的运算符则抛出Error::OpError。
  */
  var opcall(const std::wstring& op, const var& value) const {
    if (op == L"==" || op == L"!=" || op == L">=" || op == L"<=" ||
        op == L">" || op == L"<") {
      try {
        return opcmp(op, value.convert(tp));
      } catch (...) {
        return false;
      }
    } else if (op == L"===" || op == L"!==" || op == L"&&" || op == L"||") {
      return opcmp(op, value);
    } else {
      const var conv = value.convert(tp);
      if (op == L"+") {
        var ret;
        if (tp == Number) return NumberValue + conv.NumberValue;
        if (tp == String) return StringValue + conv.StringValue;
        if (tp == Array) {
          ret.tp = Array;
          ret.ArrayValue = ArrayValue;
          for (size_t i = 0; i < conv.ArrayValue.size(); i++)
            ret.ArrayValue.push_back(conv.ArrayValue[i]);
          return ret;
        }
        if (tp == Object) {
          ret.tp = Object;
          ret.ObjectValue = ObjectValue;
          for (std::map<std::wstring, var>::const_iterator i =
                   conv.ObjectValue.cbegin();
               i != conv.ObjectValue.cend(); i++)
            ret.ObjectValue[i->first] = i->second;

          return ret;
        }
        throw Error::CalcError;
      } else if (op == L"-") {
        if (tp == Number) return NumberValue - conv.NumberValue;
        throw Error::CalcError;
      } else if (op == L"*") {
        if (tp == Number) return NumberValue * conv.NumberValue;
        throw Error::CalcError;
      } else if (op == L"/") {
        if (tp == Number) {
          if (conv.NumberValue == 0) throw Error::DivError;
          return NumberValue / conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L"%") {
        if (tp == Number) {
          if (conv.NumberValue == 0) throw Error::DivError;
          return fmod(NumberValue, conv.NumberValue);
        }
        throw Error::CalcError;
      } else if (op == L"&") {
        if (tp == Number) {
          return (int)NumberValue & (int)conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L"|") {
        if (tp == Number) {
          return (int)NumberValue | (int)conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L"^") {
        if (tp == Number) {
          return (int)NumberValue ^ (int)conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L"<<") {
        if (tp == Number) {
          if (conv.NumberValue >= 32) return 0;
          return (int)NumberValue << (int)conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L">>") {
        if (tp == Number) {
          if (conv.NumberValue >= 32) return 0;
          return (int)NumberValue >> (int)conv.NumberValue;
        }
        throw Error::CalcError;
      } else if (op == L">>>") {
        if (tp == Number) {
          return double((size_t)NumberValue >> (size_t)conv.NumberValue);
        }
        throw Error::CalcError;
      } else
        throw Error::OpError;
    }
  }
  /*
  bool operator==(const var& op) const;
  同opcall(L"==",op).BooleanValue。
  */
  bool operator==(const var& op) const {
    return opcall(L"==", op).BooleanValue;
  }

 private:
  bool opcmp(const std::wstring& op, const var& value) const {
    if (op == L"===" || op == L"==") {
      if (tp != value.tp) return false;
      switch (tp) {
        case Number:
          return NumberValue == value.NumberValue;
        case Boolean:
          return BooleanValue == value.BooleanValue;
        case String:
          return StringValue == value.StringValue;
        case Array:
          return ArrayValue == value.ArrayValue;
        case Object:
          return ObjectValue == value.ObjectValue;
        case Function:
          return FunctionValue.args == value.FunctionValue.args &&
                 FunctionValue.block.value == value.FunctionValue.block.value;
        case Null:
          return tp == value.tp;
        case Statement:
          return StatementValue.value == value.StatementValue.value;
        default:
          return false;
      }
    } else if (op == L"!==")
      return !opcmp(L"===", value);
    else if (op == L"!=")
      return !opcmp(L"!=", value);
    else if (op == L">") {
      switch (tp) {
        case Number:
          return NumberValue > value.NumberValue;
        case String:
          return StringValue > value.StringValue;
        default:
          return false;
      }
    } else if (op == L"<") {
      switch (tp) {
        case Number:
          return NumberValue < value.NumberValue;
        case String:
          return StringValue < value.StringValue;
        default:
          return true;
      }
    } else if (op == L">=")
      return !opcmp(L"<", value);
    else if (op == L"<=")
      return !opcmp(L">", value);
    else if (op == L"&&") {
      return var(true).opcall(L"==", *this).BooleanValue &&
             var(true).opcall(L"==", value).BooleanValue;
    } else if (op == L"||") {
      return var(true).opcall(L"==", *this).BooleanValue ||
             var(true).opcall(L"==", value).BooleanValue;
    } else
      throw Error::OpError;
  }
  std::wstring StringValue;
  std::vector<var> ArrayValue;
  std::map<std::wstring, var> ObjectValue;
  StmtVal StatementValue;
  FuncValue FunctionValue;
  double NumberValue;
  bool BooleanValue;
  Exp::Node ExpressionValue;
  std::set<FuncValue> _constructor;
} var;
/*
var parse(const std::wstring &x, bool constant = false);
将原串x解析为变量，并将常量标记设定为constant。
如果表达式语法错误可能导致Error::ExprError。
*/
var parse(const std::wstring& x, bool constant = false) {
  std::wstring p = clearnull(x);
  if (p == L"") return var(nullptr, constant);
  if (!Exp::isexp(p)) {
    try {
      for (size_t i = 0; i < p.length(); i++) {
        if (p[i] == L'.') {
          if (p[i + 1] >= L'0' && p[i + 1] <= L'9')
            continue;
          else
            throw nullptr;
        }
      }
      if (p.find_first_of('.') != std::wstring::npos ||
          p.find_first_of('e') != std::wstring::npos)
        return var(std::stod(p), constant);
      else
        return var(std::stoi(p, 0, 0), constant);
    } catch (...) {
      if (p == L"null") return var(nullptr, constant);
      if (p == L"true" || p == L"false") return var(p == L"true", constant);
      if (isCoveredWith(p, L'\'', L'\'') || isCoveredWith(p, L'\"', L'\"')) {
        std::wstring tmp = p.substr(1, p.length() - 2), ret = L"";
        for (size_t i = 0; i < tmp.length(); i++) {
          if (tmp[i] == L'\n' || tmp[i] == L'\r') continue;
          if (tmp[i] == L'\\') {
            switch (tmp[i++]) {
              case L'\n':
              case L'\r':
                break;
              case L'\"':
              case L'\\':
              case L'\'': {
                ret += tmp[i];
                break;
              }
              case L'a': {
                ret += L'\a';
                break;
              }
              case L'b': {
                ret += L'\b';
                break;
              }
              case L'f': {
                ret += L'\f';
                break;
              }
              case L'r': {
                ret += L'\r';
                break;
              }
              case L't': {
                ret += L'\t';
                break;
              }
              case L'v': {
                ret += L'\t';
                break;
              }
              case L'n': {
                ret += L'\n';
                break;
              }
              case L'0': {
                ret += L'\0';
                break;
              }
              case L'u': {
                ret += Unicode2String(tmp.substr(i + 1, 4));
                i += 4;
                break;
              }
              default: {
                ret += p[i];
                break;
              }
            }
          } else
            ret += tmp[i];
        }
        return var(ret, constant);
      }
      if (p.substr(0, 4) == L"func" && p[p.length() - 1] == L'}') {
        std::vector<var::FuncValue::ArgItem> arg;
        std::wstring name_temp, value_temp;
        std::wstring cont_temp;
        size_t i;
        for (i = p.find_first_of('(') + 1; i < p.length(); i++) {
          if (p[i] == ')') break;
          if (p[i] == L',') {
            arg.push_back(var::FuncValue::ArgItem(name_temp, value_temp));
            name_temp.clear(), value_temp.clear();
          } else if (p[i] == L'=') {
            i++;
            for (size_t j = 0, a = 0, z = 0; i < p.length(); i++) {
              if (p[i] == L'\\')
                z = !z;
              else if (p[i] == L'\"' && !z) {
                if (a == 0 || a == 1) a = !a;
              } else if (p[i] == L'\'' && !z) {
                if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
              } else
                z = 0;
              if (p[i] == ')' && j == 0 && a == 0) {
                break;
              }
              if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
                j++;
              else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
                j--;
              if (p[i] == L',' && j == 0 && a == 0)
                break;
              else
                value_temp += p[i];
            }
            if (p[i] == ')') break;
            arg.push_back(var::FuncValue::ArgItem(name_temp, value_temp));
            name_temp.clear(), value_temp.clear();
          } else
            name_temp += p[i];
        }
        if (name_temp != L"") {
          arg.push_back(var::FuncValue::ArgItem(name_temp, value_temp));
          name_temp.clear(), value_temp.clear();
        }
        while (i < p.length() && p[i] != L'{') i++;
        if (i == p.length()) throw Error::ExprError;
        i++;
        for (size_t a = 0, j = 0, z = 0; i < p.length(); i++) {
          if (p[i] == L'\\')
            z = !z;
          else if (p[i] == L'\"' && !z) {
            if (a == 0 || a == 1) a = !a;
          } else if (p[i] == L'\'' && !z) {
            if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
          } else
            z = 0;
          if (p[i] == L'}' && j == 0 && a == 0) break;
          if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
            j++;
          else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
            j--;
          cont_temp += p[i];
        }
        return var::FuncValue(arg, var::StmtVal(codeSplit(cont_temp)));
      }
      if (isCoveredWith(p, L'{', L'}') || isCoveredWith(p, L'[', L']')) {
        std::vector<std::wstring> temp =
            splitBy(clearnull(p.substr(1, p.length() - 2)), ',');
        std::map<std::wstring, var> ret;
        std::vector<var> ret2;
        bool isobject = (p[0] == L'{');
        for (size_t i = 0; i < temp.size(); i++) {
          if (isobject) {
            std::vector<std::wstring> temp2 = splitBy(clearnull(temp[i]), ':');
            if (temp2.size() != 2)
              return var(
                  var::StmtVal(codeSplit(p.substr(1, p.length() - 2))));
            var t = parse(temp2[0]);
            if (t.tp != String) throw Error::ExprError;
            ret[parse(temp2[0])(String)] = parse(temp2[1], false);
          } else
            ret2.push_back(parse(temp[i], false));
        }
        if (isobject)
          return var(ret, constant);
        else
          return var(ret2, constant);
      }
      return var(Exp::gen(Exp::split(p)));
    }
  }
  return var(Exp::gen(Exp::split(p)));
}
};  // namespace Var
#endif
