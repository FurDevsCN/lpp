/*
L++ was under the MIT license.
Copyright(c) 2022 FurDevsCN.
*/
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>

#include "./include/parse.h"
#include "./include/var.h"

/*
struct Args;
用于解析关键字参数原串到参数列表并存储。
*/
typedef struct Args {
  // 参数列表
  std::vector<std::wstring> value;
  /*
  std::wstring toString() const;
  转换参数列表到原串。
  */
  std::wstring toString() const {
    std::wstring x;
    for (size_t i = 0; i < value.size(); i++) x += value[i] + L",";
    return x.substr(0, x.length() - 1);
  }
  Args() {}
  /*
  explicit Args(const std::vector<std::wstring> &__value);
  以参数列表__value构造类。
  */
  explicit Args(const std::vector<std::wstring>& __value) : value(__value) {}
  /*
  explicit Args(const std::wstring &p);
  解析原串p为参数列表并构造类。
  */
  explicit Args(const std::wstring& p) {
    std::wstring temp = L"";
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
      if (p[i] == L',' && j == 0 && a == 0)
        value.push_back(temp), temp = L"";
      else
        temp += p[i];
    }
    if (temp != L"") value.push_back(temp);
    for (size_t i = 0, a = 0; i < value.size(); i++) {
      a = 0;
      while (a < value[i].length() && value[i][a] == L' ') a++;
      value[i] = value[i].substr(a);
    }
  }
} Args;
struct Handler;
/*
struct Error;
用于存储不可被捕捉的错误。
*/
typedef struct Error {
  // 错误内容。
  std::wstring msg;
  // 错误调用栈。
  std::list<Handler> stack;
  Error() {}
  /*
  explicit Error(const std::wstring& _msg);
  以_msg为错误内容构造类。调用栈会被Handler自动处理所以无需提供。
  */
  explicit Error(const std::wstring& _msg) : msg(_msg) {}
} Error;
/*
enum RetType;
存储返回值类型。
*/
typedef enum RetType {
  Calc_Value = 0,  // 关键字/表达式返回的值。
  Ret_Value = 1,   // 特指return类型的关键字返回的值。
  Throw_Value = 2  // 特指throw类型的关键字返回的值。
} RetType;
/*
struct NextVal;
存储要传递给下一个关键字的内容。
*/
typedef struct NextVal {
  // 下一个语句的关键字的指示。如果下一个语句的关键字和此不符合，则会丢弃Next_Value。
  std::wstring cmd;
  // 是否限制下一个语句的关键字。当此成员为true且下一个语句关键字不为cmd时，将导致语法错误。
  bool limit;
  // 传递的内容。
  Var::var val;
  NextVal() : limit(false) {}
  /*
  NextVal(const std::wstring& __cmd, const Var::var& __val,
             bool __limit);
  以__cmd作为关键字指示，__val作为传递信息，__limit作为限制标志，构造类。
  */
  NextVal(const std::wstring& __cmd, const Var::var& __val, bool __limit)
      : cmd(__cmd), limit(__limit), val(__val) {}
} NextVal;
/*
struct RetVal;
用于存储关键字返回值。
*/
typedef struct RetVal {
  // 要返回的值。
  Var::var value;
  // 返回值的类型。
  RetType tp;
  std::list<Handler> stack;
  RetVal() : tp(Calc_Value) {}
  /*
  RetVal(const RetType& __tp, const Var::var& __value);
  以__tp作为类型，__value作为值，构建对象。
  */
  RetVal(const RetType& __tp, const Var::var& __value)
      : value(__value), tp(__tp) {
    value.constant = true;
  }
} RetVal;
// 对于RetVal(Calc_Value,...)的包装。
const RetVal Calc(const Var::var& __value) {
  return RetVal(Calc_Value, __value);
}
// 对于RetVal(Ret_Value,...)的包装。
const RetVal Ret(const Var::var& __value) { return RetVal(Ret_Value, __value); }
// 对于RetVal(Throw_Value,...)的包装。
const RetVal Throw(const Var::var& __value) {
  return RetVal(Throw_Value, __value);
}
/*
struct NativeFunc;
NativeFunc用于存储单个内嵌函数/成员。
*/
typedef struct NativeFunc {
  // 表示此内嵌函数/成员支持哪个类型（可多选），留空则表示均支持。
  std::set<Var::_ValueType> use;
  // 要返回的函数。当isvar为true时，此函数会在寻找变量时被直接执行（此时，func不应设定参数，否则会导致抛出Error）。
  Var::var::FuncValue func;
  // 表示此类用于容纳内嵌函数还是成员。当此成员为true时，则表示是成员，否则是函数。
  bool isvar;
  NativeFunc() {}
  /*
  NativeFunc(const std::set<Var::_ValueType>& _use,
             const Var::var::FuncValue& _func, bool _isvar = false);
  以_use作为支持列表，_func作为实际函数，_isvar作为成员标记，构造类。
  */
  NativeFunc(const std::set<Var::_ValueType>& _use,
             const Var::var::FuncValue& _func, bool _isvar = false)
      : use(_use), func(_func), isvar(_isvar) {}
} NativeFunc;
// 对内嵌函数/成员表的定义。
typedef std::map<std::wstring, NativeFunc> Native;
/*
class ResultObj;
getObj函数返回的寻找结果。
*/
typedef class ResultObj {
  Var::var* value;
  Var::var const_value;
  Var::var* parent;
  Var::var const_parent;
  bool parentref;
  bool ref;

 public:
  // 判断是否是引用（即是否获得到已存在的变量）。
  bool isref() const { return ref; }
  // 判断父变量是否为引用（即是否获得到已存在的父变量）。
  bool ispref() const { return parentref; }
  // 获得变量引用。在isref()为false时会自动返回临时变量，操作不会影响原变量。
  Var::var& getValue() { return (isref() ? *value : const_value); }
  // 获得父引用。在isref()为false时会自动返回临时变量，操作不会影响原变量。
  Var::var& getParent() { return (ispref() ? *parent : const_parent); }
  // 获得变量引用。在isref()为false时会自动返回临时变量，操作不会影响原变量。
  const Var::var& getValue() const { return (isref() ? *value : const_value); }
  // 获得父引用。在isref()为false时会自动返回临时变量，操作不会影响原变量。
  const Var::var& getParent() const {
    return (ispref() ? *parent : const_parent);
  }
  ResultObj() : value(nullptr), parent(nullptr), parentref(false), ref(false) {}
  /*
  ResultObj(const Var::var& x, const Var::var& p = nullptr);
  ResultObj(const Var::var& x, Var::var* p);
  ResultObj(Var::var* x, Var::var* p);
  以x作为内容，p作为父节点，构造类。
  */
  ResultObj(const Var::var& x, const Var::var& p = nullptr)
      : const_value(x),
        const_parent(p),
        parentref(false),
        ref(false) {}  // 常量
  //TODO:改成引用
  // ResultObj(const Var::var& x, Var::var* p)
  //     : const_value(x),
  //       parent(p),
  //       parentref(true),
  //       ref(false) {}  // 常量，但父节点是变量（内嵌函数）
  // ResultObj(Var::var* x, Var::var* p)
  //     : value(x), parent(p), parentref(true), ref(true) {}  // 变量
} ResultObj;
// 指定L++要使用的Parser。
typedef Lpp::Lpp Parser;
// 对作用域类型的定义。
typedef std::map<std::wstring, Var::var> ScopeType;
/*
struct Handler;
负责L++上下文访问实现。
*/
typedef struct Handler {
  // 对关键字类型的定义。
  typedef std::function<RetVal(const Parser&, Handler&)> CmdType;
  // 对关键字表类型的定义。
  typedef std::map<std::wstring, CmdType> TableType;
  // 取得关键字指示。
  NextVal& next() { return _next; }
  // 取得当前作用域。
  ScopeType& scope() const { return *_scope; }
  // 取得全局作用域。
  ScopeType& all_scope() const { return *_all_scope; }
  // 取得this对象。
  Var::var& this_scope() const { return *_this_scope; }
  // 取得内嵌函数表。
  const Native& native() const { return _native; }
  // 取得关键字表。
  const TableType& cmd() const { return _cmd; }
  Handler() {}
  /*
  Handler(ScopeType& __scope, ScopeType& __all_scope, Var::var& __this_scope,
          const NextVal& next, const TableType& __cmd, const Native& __native);
  分别以__scope,__all_scope,__this_scope作为当前/全局/this作用域，以__next作为关键字指示，__cmd作为关键字表，__native作为内嵌函数表，构造一个上下文。
  */
  Handler(ScopeType& __scope, ScopeType& __all_scope, Var::var& __this_scope,
          const NextVal& next, const TableType& __cmd, const Native& __native)
      : _scope(&__scope),
        _all_scope(&__all_scope),
        _this_scope(&__this_scope),
        _next(next),
        _cmd(__cmd),
        _native(__native) {}
  /*
  RetVal operator()(const Parser& value);
  等同exec(value)。
  */
  RetVal operator()(const Parser& value) { return exec(value); }
  /*
  RetVal exec(const Parser& value)
  执行value，value是一个解析完毕的语句。
  */
  RetVal exec(const Parser& value) {
    try {
      RetVal t;
      if (isKeyword(value.name)) {
        // 是关键字调用的情况下，会执行关键字。
        if (next().cmd != value.name && next().limit)
          throw Error(L"Invalid statement");
        if (next().cmd != value.name) next() = NextVal();
        t = cmd().at(value.name)(value, *this);
        t.stack.push_back(*this);
        return t;
      } else if (isKeyword(L"")) {
        // 不是关键字调用的情况下，会执行默认（即键为L""的）函数。
        if (next().cmd != L"" && next().limit)
          throw Error(L"Invalid statement");
        t = cmd().at(L"")(value, *this);
        t.stack.push_back(*this);
        return t;
      } else
        throw Error(L"Invalid statement");  // 否则，抛出错误。
    } catch (const Error& x) {
      Error y = x;
      y.stack.push_back(*this);
      throw y;
    }
  }
  /*
  bool isKeyword(const std::wstring& x) const;
  判断x是否是关键字。
  */
  bool isKeyword(const std::wstring& x) const {
    return cmd().find(x) != cmd().cend() && cmd().at(x);
  }
  /*
  bool isIdentifier(const std::wstring& x) const;
  判断x是否为合法标识符。
  */
  bool isIdentifier(const std::wstring& x) const {
    if (isKeyword(x)) return false;
    bool flag = false;
    for (size_t i = 0; i < x.length(); i++) {
      if (i == 1) flag = true;
      if (x[i] >= L'0' && x[i] <= L'9') {
        if (!flag) return false;
      } else if ((x[i] >= L'a' && x[i] <= L'z') ||
                 (x[i] >= L'A' && x[i] <= L'Z') || x[i] == L'_' || x[i] == L'$')
        continue;
      else
        return false;
    }
    return true;
  }
  /*
  bool isStatement(const Parser& a) const;
  判断x是否为语句。
  */
  bool isStatement(const Parser& a) const {
    try {
      const Args temp(a.args);
      return isKeyword(a.name) ||
             (!Var::Exp::isexp(a.toString()) &&
              temp.value.size() == 1 &&
              Var::isCoveredWith(temp.value[0], L'(', L')'));
    } catch (...) {
      return false;
    }
  }
  /*
  RetVal runstmt(const Var::var::StmtVal& stmt, ScopeType& temp_scope) const;
  执行语句段stmt。指定temp_scope作为临时作用域。
  */
  RetVal runstmt(const Var::var::StmtVal& stmt, ScopeType& temp_scope) const {
    Handler p(temp_scope, all_scope(), this_scope(), NextVal(), cmd(),
              native());
    for (size_t i = 0; i < stmt.value.size(); i++) {
      const RetVal res = p(Parser(stmt.value[i]));
      if (res.tp != Calc_Value) return res;
      scope() = update_scope(scope(), temp_scope);
    }
    return RetVal();
  }
  /*
  Var::var runfunc(const Var::var::FuncValue& func, ScopeType& use_scope,
                   const std::vector<Var::var>& arguments);
  以给定的函数作用域use_scope和参数arguments执行函数func。
  */
  Var::var runfunc(const Var::var::FuncValue& func, ScopeType& use_scope,
                   const std::vector<Var::var>& arguments) {
    funcarg_set(use_scope, func.args, arguments);
    Handler p(use_scope, all_scope(), this_scope(), NextVal(), cmd(), native());
    for (size_t i = 0; i < func.block.value.size(); i++) {
      const RetVal res = p(Parser(func.block.value[i]));
      if (res.tp != Calc_Value) return res.value;
    }
    return nullptr;
  }
  /*
  ResultObj expr(const Var::var& exp);
  计算exp的值。
  注意：经Var::parse过后的值可能是表达式，此函数可以计算表达式。
  */
  ResultObj expr(const Var::var& exp) {
    if (exp.tp == Var::Statement) {
      throw Error(L"Cannot construct a statement");
    } else if (exp.tp != Var::Expression) {
      switch (exp.tp) {
        case Var::Object: {
          std::map<std::wstring, Var::var> x;
          for (std::map<std::wstring, Var::var>::const_iterator i =
                   exp(Var::Object).cbegin();
               i != exp(Var::Object).cend(); i++) {
            x[i->first] =
                expr(i->second).getValue();  // calculate values of the object
            x[i->first].constant = false;
          }
          return Var::var(x);
        }
        case Var::Array: {
          std::vector<Var::var> x(exp(Var::Array).size());
          for (size_t i = 0; i < exp(Var::Array).size(); i++) {
            x[i] = expr(exp(Var::Array)[i])
                       .getValue();  // calculate members of the array
            x[i].constant = false;
          }
          return Var::var(x);
        }
        case Var::Function: {
          bool using_optional_arg = false;
          for (size_t i = 0; i < exp(Var::Function).args.size(); i++) {
            if (!isIdentifier(exp(Var::Function).args[i].name))
              throw Error(L"the name of the argument is invalid");
            if (exp(Var::Function).args[i].name != L"")
              using_optional_arg = true;
            if (exp(Var::Function).args[i].name == L"" && using_optional_arg)
              throw Error(L"the argument list syntax is invalid");
          }
          return exp;
        }
        default:
          return exp;
      }
    }  // return var(exp);
    const Var::Exp::Node& p = exp(Var::Expression);
    if (p.tp == Var::Exp::Value) {
      if (p.val == L"")
        return Var::var(nullptr);
      else {
        if (!isStatement(Parser(p.val))) {
          if (!Var::Exp::isexp(p.val) && p.val[0] != L'-' &&
              p.val[0] != L'+') {
            // TODO:getObj放到这里
            return getObj(p.val, true, false);
          } else if (p.val[0] == L'-' || p.val[0] == L'+') {
            return expr(Var::parse(p.val.substr(1)))
                .getValue()
                .opcall({p.val[0]});
          } else
            throw Error(L"Unknown operand");
        } else {
          const RetVal w(exec(Parser(p.val)));
          if (w.tp != Calc_Value)
            throw w;
          else
            return w.value;
        }
      }
    }
    // expression
    if (p.val == L"?") {
      if (p.r().tp != Var::Exp::Exp || p.r().val != L":") {
        throw Error(L"the right of ? must be a : with 2 values");
      }
      if (Var::var(true).opcall(L"==", expr(p.l()).getValue())(Var::Boolean)) {
        return expr(p.r().l());
      } else {
        return expr(p.r().r());
      }
    } else if (p.val == L":") {
      throw Error(L": with ? expression forbidden");
    } else if (p.val == L"||") {
      return Var::var(Var::var(true).opcall(L"==",
                                   expr(p.l()).getValue())(Var::Boolean) ||
             Var::var(true).opcall(L"==", expr(p.l()).getValue())(Var::Boolean));
    } else if (p.val == L"&&") {
      return Var::var(Var::var(true).opcall(L"==",
                                   expr(p.l()).getValue())(Var::Boolean) &&
             Var::var(true).opcall(L"==", expr(p.l()).getValue())(Var::Boolean));
    } else if (p.val[p.val.length() - 1] == L'=' && p.val != L"===" &&
               p.val != L"!==" && p.val != L"==" && p.val != L"!=" &&
               p.val != L">=" && p.val != L"<=") {
      ResultObj q;
      if (p.l().tp != Var::Exp::Value)
        throw Error(L"lvalue cannot be a constant");
      q = (p.val == L"=") ? getObj(p.l().val, false, true)
                          : getObj(p.l().val, true, true);
      if (!q.isref() || q.getValue().constant == true) {
        throw Error(L"lvalue cannot be a constant");
      }
      if (p.val == L"=") {
        q.getValue() = expr(p.r()).getValue();
        q.getValue().constant = false;
      } else {
        q.getValue() = q.getValue().opcall(p.val.substr(0, p.val.length() - 1),
                                           expr(p.r()).getValue());
        q.getValue().constant = false;
      }
      return q.getValue();
    } else if (p.val == L"++" || p.val == L"--") {
      ResultObj q;
      bool prefix = (p.l().val == L"");
      q = prefix ? getObj(p.l().val, true, true)
                 : getObj(p.r().val, true, true);
      if (q.isref() != true || q.getValue().constant == true) {
        throw Error(L"lvalue cannot be a constant");
      }
      if (prefix) {
        q.getValue() = q.getValue().opcall(p.val.substr(0, 1), 1);
        q.getValue().constant = false;
        return q.getValue();
      } else {
        Var::var temp = q.getValue();
        q.getValue() = q.getValue().opcall(p.val.substr(0, 1), 1);
        q.getValue().constant = false;
        return temp;
      }
    } else if (p.val == L",") {
      return expr(p.l()), expr(p.r());
    }
    return expr(p.l()).getValue().opcall(p.val, expr(p.r()).getValue());
  }

 private:
  static std::wstring getFirstName(const std::wstring& p) {
    std::wstring temp;
    for (size_t i = 0, a = 0, j = 0, z = 0; i < p.length(); i++) {
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
      if (p[i] == L'.' && a == 0 && j == 0) break;
      if (p[i] == L'[' && a == 0 && j == 1) {
        if (temp == L"" && (i <= 0 || p[i - 1] != L']'))
          temp += p[i];
        else
          break;
      } else
        temp += p[i];
    }
    return temp;
  }
  ResultObj getObj(const std::wstring& n, bool nonewobject, bool nonative) {
    if (getFirstName(n) == L"") throw Error(L"Syntax error");
    if (isStatement(Parser(Var::clearnull(n)))) {
      const RetVal s = (*this)(Parser(n));
      if (s.tp != Calc_Value)
        throw s;
      else
        return ResultObj(s.value);
    }
    if (getFirstName(n) == n) {
      if (!isIdentifier(n) && n != L"arguments" &&
          (Var::Exp::isexp(n) || Var::parse(n).tp != Var::Expression)) {
        return expr(Var::parse(n));
      } else if (n == L"this") {
        return ResultObj(&this_scope(), &this_scope());
      } else if (scope().find(n) != scope().cend()) {
        return ResultObj(&scope()[n], &this_scope());
      } else if (all_scope().find(n) != all_scope().cend()) {
        return ResultObj(&all_scope()[n], &all_scope());
      } else {
        if (Var::Exp::isexp(n)) {
          return expr(Var::parse(n));
        } else if (nonewobject) {
          throw Error(n + L" is not defined");
          // return ResultObj(Var::var(nullptr), scope, scope, false);
        } else {
          scope()[n].constant = false;
          scope()[n].tp = Var::Null;
          return ResultObj(&scope()[n], &scope());
        }
      }
    } else {
      std::wstring fst_nme = getFirstName(n);
      if (fst_nme == L"this") {
        return get_var_index(n.substr(fst_nme.length()),
                             ResultObj(&this_scope(), &this_scope()),
                             nonewobject, nonative);
      } else if (scope().find(fst_nme) != scope().cend()) {
        return get_var_index(n.substr(fst_nme.length()),
                             ResultObj(&scope()[fst_nme], &scope()),
                             nonewobject, nonative);
      } else if (all_scope().find(fst_nme) != all_scope().cend()) {
        return get_var_index(n.substr(fst_nme.length()),
                             ResultObj(&all_scope()[fst_nme], &all_scope()),
                             nonewobject, nonative);
      } else
        return get_var_index(n.substr(fst_nme.length()),
                             expr(Var::parse(fst_nme)), nonewobject, nonative);
    }
  }
  static ScopeType update_scope(ScopeType now_scope,
                                const ScopeType& temp_scope) {
    for (ScopeType::const_iterator x = now_scope.cbegin();
         x != now_scope.cend(); x++) {
      if (temp_scope.find(x->first) == temp_scope.cend())
        now_scope.erase(x->first);
      else
        now_scope[x->first] = temp_scope.at(x->first);
    }
    return now_scope;
  }
  static std::vector<std::wstring> get_name_split(const std::wstring& p) {
    std::vector<std::wstring> visit;
    std::wstring temp;
    for (size_t i = 0, a = 0, j = 0, z = 0; i < p.length(); i++) {
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
      if ((p[i] == L'[' || p[i] == L']') && a == 0 && j == (p[i] == L'[')) {
        if (temp != L"") visit.push_back(temp), temp = L"";
      } else if (p[i] == L'.' && j == 0 && a == 0) {
        i++;
        while (p[i] != L'[' && p[i] != L'.' && i < p.length()) {
          temp += p[i];
          i++;
        }
        visit.push_back(L"\"" + temp + L"\"");
        temp = L"";
        i--;
      } else
        temp += p[i];
    }
    if (temp != L"") visit.push_back(temp);
    return visit;
  }
  void funcarg_set(ScopeType& use_scope,
                   const std::vector<Var::var::FuncValue::ArgItem>& arg,
                   const std::vector<Var::var>& give) {
    std::vector<Var::var> temp;
    for (size_t i = 0; i < arg.size(); i++) {
      Var::var t;
      if (give.size() > i) {
        t = expr(give[i]).getValue();
        temp.push_back(t);
        use_scope[arg[i].name] = t;
      } else {
        if (arg[i].value == L"") throw Error(L"too few arguments given");
        t = expr(Var::parse(arg[i].value)).getValue();
        use_scope[arg[i].name] = t;
      }
    }
    use_scope[L"arguments"] = Var::var(temp, true);
  }
  ResultObj get_var_index(const std::wstring& p, ResultObj object,
                          bool nonewobject, bool no_native) {
    const std::vector<std::wstring> visit = get_name_split(p);
    bool ref = object.isref();
    Var::var *now_object = nullptr, *parent_object = nullptr;
    Var::var now_const_object, parent_const_object;
    if (ref) {
      now_object = &object.getValue();
      parent_object = &object.getParent();
    } else {
      now_const_object = object.getValue();
      parent_const_object = object.getParent();
    }
    bool this_keep = (&object.getValue() == &this_scope());
    bool prref = object.ispref();
    for (size_t i = 0; i < visit.size(); i++) {
      Var::var visit_temp = expr(Var::parse(visit[i])).getValue();
      std::wstring find_str;
      if (visit_temp.tp == Var::String)
        find_str = visit_temp(Var::String);
      else
        find_str = visit_temp.toString();
      if (find_str[0] == L'_' && !this_keep)
        throw Error(find_str + L" is private within this context");
      if (find_str != L"this")
        this_keep = false;
      else
        continue;
      if (native().find(find_str) != native().cend()) {
        // FIXME:检查是否有bug
        const NativeFunc& f = native().at(find_str);
        ScopeType v;
        if (ref &&
            (f.use.empty() || f.use.find(now_object->tp) != f.use.cend()) &&
            (f.isvar || !no_native)) {
          prref = true;
          parent_object = now_object;
          if (f.isvar)
            now_const_object = Handler(scope(), all_scope(), *parent_object,
                                       NextVal(), cmd(), native())
                                   .runfunc(f.func, v, {});
          else
            now_const_object = f.func;
          ref = false;
          continue;
        } else if ((f.use.empty() ||
                   f.use.find(now_const_object.tp) != f.use.cend()) &&
                       (f.isvar || !no_native)) {
          prref = false;
          parent_const_object = now_const_object;
          if (f.isvar)
            now_const_object =
                Handler(scope(), all_scope(), parent_const_object, NextVal(),
                        cmd(), native())
                    .runfunc(f.func, v, {});
          else
            now_const_object = f.func;
          ref = false;
          continue;
        }
      }
      if (ref) {
        prref = true;
        parent_object = now_object;
        switch (now_object->tp) {
          case Var::String: {
            try {
              visit_temp = visit_temp.convert(Var::Number);
              if (visit_temp(Var::Number) < 0) throw nullptr;
            } catch (...) {
              now_const_object = nullptr;
              ref = false;
              break;
            }
            now_const_object = std::wstring(
                1, (*now_object)(Var::String)[(size_t)visit_temp(Var::Number)]);
            ref = false;
            break;
          }
          case Var::Object: {
            if ((*now_object)(Var::Object).find(find_str) ==
                (*now_object)(Var::Object).cend()) {
              if (nonewobject) {
                now_const_object = Var::var(nullptr, false);
                ref = false;
              } else {
                (*now_object)(Var::Object)[find_str] = Var::var(nullptr, false);
                now_object = &(*now_object)(Var::Object)[find_str];
              }
            } else
              now_object = &(*now_object)(Var::Object)[find_str];
            break;
          }
          case Var::Array: {
            try {
              visit_temp = visit_temp.convert(Var::Number);
              if (visit_temp(Var::Number) < 0) throw nullptr;
            } catch (...) {
              now_const_object = nullptr;
              ref = false;
              break;
            }
            if ((size_t)visit_temp(Var::Number) >=
                (*now_object)(Var::Array).size()) {
              if (!nonewobject) {
                now_const_object = Var::var(nullptr, false);
                ref = false;
              } else {
                (*now_object)(Var::Array)
                    .resize((size_t)visit_temp(Var::Number) + 1);
                for (size_t i = 0; i < (*now_object)(Var::Array).size(); i++)
                  (*now_object)(Var::Array)[i].constant = false;
              }
            }
            now_object =
                &(*now_object)(Var::Array)[(size_t)visit_temp(Var::Number)];
            break;
          }
          default: {
            now_const_object = nullptr;
            ref = false;
            break;
          }
        }
        break;
      } else {
        prref = false;
        parent_const_object = now_const_object;
        switch (now_const_object.tp) {
          case Var::String: {
            try {
              visit_temp = visit_temp.convert(Var::Number);
              if (visit_temp(Var::Number) < 0) throw nullptr;
            } catch (...) {
              now_const_object = nullptr;
              break;
            }
            if (now_const_object(Var::String).size() >=
                (size_t)visit_temp(Var::Number))
              now_const_object = nullptr;
            else
              now_const_object = std::wstring(
                  1, now_const_object(
                         Var::String)[(size_t)visit_temp(Var::Number)]);
            break;
          }
          case Var::Object: {
            if (now_const_object(Var::Object).find(find_str) ==
                now_const_object(Var::Object).cend())
              now_const_object = nullptr;
            else {
              now_const_object = now_const_object(Var::Object)[find_str];
            }
            break;
          }
          case Var::Array: {
            try {
              visit_temp = visit_temp.convert(Var::Number);
              if (visit_temp(Var::Number) < 0) throw nullptr;
            } catch (...) {
              now_const_object = nullptr;
              break;
            }
            if ((size_t)visit_temp(Var::Number) >=
                now_const_object(Var::Array).size())
              now_const_object = nullptr;
            else
              now_const_object =
                  now_const_object(Var::Array)[(size_t)visit_temp(Var::Number)];
            break;
          }
          default: {
            now_const_object = nullptr;
            break;
          }
        }
        break;
      }
    }
    if (ref)
      return ResultObj(now_object, parent_object);
    else if (prref)
      return ResultObj(now_const_object, parent_const_object);
    else
      return ResultObj(now_const_object, parent_const_object);
  }

  ScopeType* _scope;
  ScopeType* _all_scope;
  Var::var* _this_scope;
  NextVal _next;
  TableType _cmd;
  Native _native;
} Handler;