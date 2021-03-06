/*
L++ programming language was under the MIT license.
copyright(c) 2021 nu11ptr team.
*/
#ifndef PARSE_MODULE
#define PARSE_MODULE
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "./var.h"
namespace Lpp {
typedef class Lpp_base {  // The base of L programming language.
  void splitargs(const std::wstring
                     &p) {  // Internal function.Splits std::wstring to args.
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
        args.push_back(temp), temp = L"";
      else
        temp += p[i];
    }
    if (temp != L"") args.push_back(temp);
    for (size_t i = 0, a = 0; i < args.size(); i++) {
      a = 0;
      while (a < args[i].length() && args[i][a] == L' ') a++;
      args[i] = args[i].substr(a);
    }
  }
  const std::wstring arg2str() const {
    std::wstring x;
    for (size_t i = 0; i < args.size(); i++) x += args[i] + L",";
    return x.substr(0, x.length() - 1);
  }

 public:
  std::wstring name;                 // The command's name.
  std::vector<std::wstring> args;    // The command's arguments.
  Lpp_base(const std::wstring &x) {  // The constructor.
    if (x == L"" || x[0] == L'#') return;
    size_t i = 0;
    for (size_t a = 0, j = 0, z = 0; i < x.length(); i++) {
      if (i == 0)
        while (x[i] == L' ') i++;
      if (x[i] == L'\\')
        z = !z;
      else if (x[i] == L'\"' && !z) {
        if (a == 0 || a == 1) a = !a;
      } else if (x[i] == L'\'' && !z) {
        if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
      } else
        z = 0;
      if (x[i] == L'#' && a == 0) {
        while (i < x.length() && x[i] != L'\n') i++;
      } else if (x[i] == L'\n' || x[i] == L'\t')
        continue;  // p+=x[i];
      if ((x[i] == L'(' || x[i] == L'{' || x[i] == L'[') && a == 0)
        j++;
      else if ((x[i] == L')' || x[i] == L'}' || x[i] == L']') && a == 0)
        j--;
      if (x[i] == L' ' && j == 0 && a == 0)
        break;
      else
        name += x[i];
    }
    if (i != x.length()) splitargs(x.substr(i + 1));
  }
  Lpp_base(const std::wstring &__name,
           const std::vector<std::wstring> &__args) {
    name = __name;
    args = __args;
  }
  const std::wstring toString()
      const {  // Converts the command to std::wstring.
    return name + (args.empty() ? L"" : L" ") + arg2str();
  }
  const bool operator==(const Lpp_base &x)
      const {  // Compares two commands.operator!=(...) == !(operator==(...))
    if (x.name == name && x.args == args) return true;
    return false;
  }
  Lpp_base() {}  // The default constructor.
} Lpp_base;
typedef enum Return_Type {
  Calc_Value = 0,
  Ret_Value = 1,
  Throw_Value = 2,
} Return_Type;
typedef struct Return_Value {
  Return_Type tp;
  Variable::var value;
  std::wstring msg;
  Return_Value() { tp = Calc_Value; }
  Return_Value(const std::wstring &__msg, const Return_Type &__type,
               const Variable::var &__value) {
    msg = __msg;
    tp = __type;
    value = __value;
    value.isConst = true;
  }
  Return_Value(const Return_Type &__type, const Variable::var &__value) {
    msg = L"";
    tp = __type;
    value = __value;
    value.isConst = true;
  }
} Return_Value;
typedef struct Exec_Info : public Return_Value {
  Lpp_base cmd;
  Variable::var scope;
  Variable::var all_scope;
  Variable::var this_scope;
  Exec_Info() { tp = Calc_Value; }
  Exec_Info(const Return_Value &__base, const Lpp_base &__cmd,
            const Variable::var &__scope, const Variable::var &__all_scope,
            const Variable::var &__this_scope) {
    msg = __base.msg;
    cmd = __cmd;
    tp = __base.tp;
    value = __base.value;
    value.isConst = true;
    scope = __scope;
    all_scope = __all_scope;
    this_scope = __this_scope;
  }
} Exec_Info;
typedef struct Lpp : public Lpp_base {
  typedef std::function<const Return_Value(const Lpp &, Variable::var &,
                                           Variable::var &, Variable::var &)>
      CmdType;
  std::map<std::wstring, CmdType> cmd;
  Lpp() : Lpp_base() {}
  Lpp(const std::wstring &x, const std::map<std::wstring, CmdType> &_cmd)
      : Lpp_base(x) {
    cmd = _cmd;
  }  //= Lpp_base(const std::wstring&);
  const Exec_Info eval(const Variable::var &scope) {
    Variable::var temp = scope;
    temp.isConst = true;
    temp.tp = Variable::Object;
    return eval(temp, temp, temp);
  }
  const Exec_Info eval(Variable::var &scope, Variable::var &all_scope,
                       Variable::var &this_scope) {
    Return_Value info;
    if (cmd.find(name) != cmd.cend() && cmd[name]) {
      info = cmd[name](*this, scope, all_scope, this_scope);
      return Exec_Info(info, *this, scope, all_scope, this_scope);
    } else if (cmd.find(L"") != cmd.cend() && cmd[L""]) {
      info = cmd[L""](*this, scope, all_scope, this_scope);
      return Exec_Info(info, *this, scope, all_scope, this_scope);
    } else {
      return Exec_Info(Return_Value(Throw_Value, L"EvalError"), *this, scope,
                       all_scope, this_scope);
    }
  }

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
    const Variable::var &getConstValue() {
      if (tp == is_const_value)
        return const_value;
      else
        return *value;
    }
    const Variable::var &getConstParent() {
      if (tp == is_const_value)
        return const_parent;
      else
        return *parent;
    }
    Variable::var &getThis() { return *this_object; }
    Object_Type tp;
    Return_Object() {
      tp = is_const_value;
      value = nullptr;
      parent = nullptr;
      lastIsThis = false;
    }
    Return_Object(const Variable::var &x, Variable::var &p, Variable::var &s,
                  const bool l) {
      tp = is_const_value;
      const_value = x;
      parent = &p;
      // if (p != nullptr) const_parent = *p;
      this_object = &s;
      lastIsThis = l;
    }
    Return_Object(const Variable::var &x, const Variable::var &p,
                  Variable::var &s, const bool l) {
      tp = is_const_value;
      const_value = x;
      const_parent = p;
      this_object = &s;
      lastIsThis = l;
    }
    Return_Object(Variable::var &x, Variable::var &p, Variable::var &s,
                  const bool l) {
      tp = is_pointer;
      value = &x;
      parent = &p;
      // if (p != nullptr) const_parent = *p;
      this_object = &s;
      lastIsThis = l;
    }
    Return_Object(const std::nullptr_t &x, const Variable::var &w,
                  const Variable::var &p, Variable::var &s, const bool l) {
      tp = is_const_value;
      isNative = true;
      const_value = w;
      const_parent = p;
      this_object = &s;
      lastIsThis = l;
    }
  } Return_Object;
  static const std::vector<std::wstring> getKeys(
      const std::map<std::wstring, CmdType> &x) {
    std::vector<std::wstring> ret;
    for (std::map<std::wstring, CmdType>::const_iterator i = x.cbegin();
         i != x.cend(); i++)
      ret.push_back(i->first);
    return ret;
  }
  const bool isKeyword(const std::wstring &x) const {
    const std::vector<std::wstring> &keyword = getKeys(cmd);
    for (size_t i = 0; i < keyword.size(); i++) {
      if (x == keyword[i]) return true;
    }
    return false;
  }
  const bool isIdentifier(const std::wstring &x,
                          const bool allowprivate = false) const {
    if (x == L"arguments" || ((!allowprivate) && x[0] == L'_') || isKeyword(x))
      return false;
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
    try {
      return Variable::parse(x).tp == Variable::Expression;
    } catch (...) {
      return false;
    }
  }
  const bool isStatement(const Lpp_base &a) const {
    try {
      return isKeyword(a.name) ||
             (a.args.size() == 1 &&
              Variable::parse(a.args[0]).tp == Variable::Array);
    } catch (...) {
      return false;
    }
  }
  static const std::pair<std::wstring, std::wstring> splitStmt(
      const std::wstring &p) {
    std::wstring f;
    for (size_t i = p.length() - 1, a = 0, j = 1, z = 0; i > 0; i--) {
      if (i > 0 && p[i - 1] == L'\\')
        z = !z;
      else if (p[i] == L'\"' && !z) {
        if (a == 0 || a == 1) a = !a;
      } else if (p[i] == L'\'' && !z) {
        if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
      } else
        z = 0;
      if ((p[i] == L'[' || p[i] == L'{' || p[i] == L'(') && a == 0)
        j++;
      else if ((p[i] == L']' || p[i] == L'}' || p[i] == L')') && a == 0)
        j--;
      if ((p[i] == L'{' || p[i] == L'[' || p[i] == L'(') && a == 0 && j == 1) {
        f = p[i] + f;
        break;
      } else
        f = p[i] + f;
    }
    return std::make_pair(p.substr(0, p.length() - f.length()), f);
  }
  void funcarg_set(Variable::var &use_scope, Variable::var &scope,
                   Variable::var &all_scope, Variable::var &this_scope,
                   const std::vector<Variable::var::Func_temp::Arg_Item> &arg,
                   const Variable::var &give) const {
    if (give.tp != Variable::Array)
      throw Variable::SyntaxErr(L"give.tp != Variable::Array");
    for (size_t i = 0; i < arg.size(); i++) {
      if (give.ArrayValue.size() > i) {
        use_scope.ObjectValue[arg[i].name] =
            exp_calc(give.ArrayValue[i], scope, all_scope, this_scope);
      } else {
        if (arg[i].value == L"")
          throw Variable::SyntaxErr(L"too few arguments");
        use_scope.ObjectValue[arg[i].name] = exp_calc(
            Variable::parse(arg[i].value), scope, all_scope, this_scope);
      }
    }
  }
  Return_Object get_object(const std::wstring &n, Variable::var &scope,
                           Variable::var &all_scope, Variable::var &this_scope,
                           const bool nonewobject, const bool nonative) const {
    if (get_first_name(n) == L"") {
      throw member_not_exist;
    }
    if (isStatement(Lpp_base(Variable::clearnull(n)))) {
      Return_Value s = Lpp(n, cmd).eval(scope, all_scope, this_scope);
      if (s.tp != Calc_Value)
        throw s;
      else
        return Return_Object(s.value, scope, scope, false);
    }
    if (get_first_name(n) == n) {
      if (!isIdentifier(n) && n != L"arguments" &&
          (Variable::isExpression(n) ||
           Variable::parse(n).tp != Variable::Expression)) {
        return Return_Object(
            exp_calc(Variable::parse(n), scope, all_scope, this_scope), scope,
            scope, false);
      } else if (n == L"this") {
        return Return_Object(this_scope, this_scope, this_scope, true);
      } else if (scope.ObjectValue.find(n) != scope.ObjectValue.cend()) {
        return Return_Object(scope.ObjectValue[n], this_scope, this_scope,
                             false);
      } else if (all_scope.ObjectValue.find(n) !=
                 all_scope.ObjectValue.cend()) {
        return Return_Object(all_scope.ObjectValue[n], all_scope, all_scope,
                             false);
      } else {
        if (Variable::isExpression(n)) {
          return Return_Object(
              exp_calc(Variable::parse(n), scope, all_scope, this_scope), scope,
              scope, false);
        } else if (nonewobject) {
          throw member_not_exist;
          //return Return_Object(Variable::var(nullptr), scope, scope, false);
        } else {
          scope.ObjectValue[n].isConst = false;
          scope.ObjectValue[n].tp = Variable::Null;
          return Return_Object(scope.ObjectValue[n], scope, scope, false);
        }
      }
    } else {
      std::wstring fst_nme = get_first_name(n);
      Variable::var tmp_obj;
      if (fst_nme == L"this") {
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(this_scope, this_scope, this_scope, false), scope,
            all_scope, this_scope, nonewobject, true, nonative);
      } else if (scope.ObjectValue.find(fst_nme) != scope.ObjectValue.cend()) {
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(scope.ObjectValue[fst_nme], scope, scope, false),
            scope, all_scope, this_scope, nonewobject, false, nonative);
      } else if (all_scope.ObjectValue.find(fst_nme) !=
                 all_scope.ObjectValue.cend()) {
        return get_var_index(n.substr(fst_nme.length()),
                             Return_Object(all_scope.ObjectValue[fst_nme],
                                           all_scope, all_scope, false),
                             scope, all_scope, this_scope, nonewobject, false,
                             nonative);
      } else
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(exp_calc(Variable::parse(fst_nme), scope, all_scope,
                                   this_scope),
                          scope, scope, false),
            scope, all_scope, this_scope, nonewobject, false, nonative);
    }
  }
  void RunStmt(Variable::var &stmt, Variable::var &scope,
               Variable::var &all_scope, Variable::var &this_scope,
               Variable::var &temp_scope, const Variable::var &exclude,
               const bool enablebreak, const bool enablecontinue) const {
    if (stmt.tp != Variable::StmtBlock || scope.tp != Variable::Object ||
        all_scope.tp != Variable::Object || this_scope.tp != Variable::Object ||
        temp_scope.tp != Variable::Object) {
      throw Variable::SyntaxErr(L"run settings is invalid");
    }
    size_t a = 0;
    std::map<std::wstring, CmdType> s = cmd;
    if (enablebreak) {
      s[L"break"] = [&a](const Lpp &cmd, Variable::var &scope,
                         Variable::var &all_scope,
                         Variable::var &this_scope) -> Return_Value {
        a = 2;  // break_type=break
        return Return_Value(Calc_Value, nullptr);
      };
    }
    if (enablecontinue) {
      s[L"continue"] = [&a](const Lpp &cmd, Variable::var &scope,
                            Variable::var &all_scope,
                            Variable::var &this_scope) -> Return_Value {
        a = 1;  // break_type=continue
        return Return_Value(Calc_Value, nullptr);
      };
    }
    for (size_t i = 0; i < stmt.StmtValue.value.size(); i++) {
      a = 0;
      Lpp temp = Lpp(stmt.StmtValue.value[i], s);
      Exec_Info res = temp.eval(temp_scope, all_scope, this_scope);
      if (res.tp != Calc_Value) {
        res.scope = scope;
        throw res;
      }
      if (a != 0) {
        throw a;
      }
      scope = update_scope(scope, temp_scope);
    }
    temp_scope = update_scope2(scope, temp_scope, exclude);
  }
  const Variable::var RunFunc(Variable::var &func, Variable::var &scope,
                              Variable::var &all_scope,
                              Variable::var &this_scope,
                              const Variable::var &arguments,
                              const bool innew) const {
    if (func.tp != Variable::Function || scope.tp != Variable::Object ||
        all_scope.tp != Variable::Object || this_scope.tp != Variable::Object ||
        arguments.tp != Variable::Array) {
      throw Variable::SyntaxErr(L"run settings is invalid");
    }
    Variable::var temp_scope;
    Variable::var *parent = &this_scope;
    temp_scope.isConst = false;
    temp_scope.tp = Variable::Object;
    temp_scope.ObjectValue[L"arguments"] = arguments;
    temp_scope.ObjectValue[L"arguments"].isConst = true;
    try {
      funcarg_set(temp_scope, scope, all_scope, this_scope,
                  func.FunctionValue.args, arguments);
    } catch (...) {
      throw Variable::ExprErr(L"failed to initalize argument list.");
    }
    std::map<std::wstring, CmdType> s = cmd;
    for (size_t i = 0; i < func.FunctionValue.block.value.size(); i++) {
      Exec_Info res;
      res = Lpp(func.FunctionValue.block.value[i], s)
                .eval(temp_scope, all_scope, *parent);
      if (res.tp == Ret_Value) {
        return res.value;
      } else if (res.tp == Throw_Value) {
        throw res;
      }
    }
    return nullptr;
  }
  const Variable::var exp_calc(const Variable::var &exp, Variable::var &scope,
                               Variable::var &all_scope,
                               Variable::var &this_scope,
                               const bool newObjectIsConst = false) const {
    std::vector<std::wstring> p = exp.ExpressionValue;
    std::vector<Variable::var> st;
    std::wstring op;
    Variable::var res;
    bool is_single = false;
    if (exp.tp == Variable::StmtBlock)
      throw Variable::ExprErr(L"this type cannot be parsed.");
    if (exp.tp != Variable::Expression) {
      switch (exp.tp) {
        case Variable::Object: {
          std::map<std::wstring, Variable::var> x;
          for (std::map<std::wstring, Variable::var>::const_iterator i =
                   exp.ObjectValue.cbegin();
               i != exp.ObjectValue.cend(); i++) {
            x[i->first] = exp_calc(i->second, scope, all_scope,
                                   this_scope);  // calc values of the object
            x[i->first].isConst = false;
          }
          return x;
        }
        case Variable::Array: {
          std::vector<Variable::var> x(exp.ArrayValue.size());
          for (size_t i = 0; i < exp.ArrayValue.size(); i++) {
            x[i] = exp_calc(exp.ArrayValue[i], scope, all_scope,
                            this_scope);  // calc members of the array
            x[i].isConst = false;
          }
          return x;
        }
        case Variable::Function: {
          bool using_optional_arg = false;
          for (size_t i = 0; i < exp.FunctionValue.args.size(); i++) {
            if (!isIdentifier(exp.FunctionValue.args[i].name))
              throw Variable::ExprErr(L"function's argument name is invalid.");
            if (exp.FunctionValue.args[i].name != L"")
              using_optional_arg = true;
            if (exp.FunctionValue.args[i].name == L"" && using_optional_arg)
              throw Variable::ExprErr(
                  L"function's argument syntax is invalid.");
          }
          return exp;
        }
        default: {
          return exp;
        }
      }
    }  // return var(exp);
    if (exp.ExpressionValue.size() == 0) return nullptr;
    if (exp.ExpressionValue.size() == 1 &&
        !isStatement(Lpp_base(Variable::clearnull(exp.ExpressionValue[0]))) &&
        !Variable::isExpression(exp.ExpressionValue[0]) &&
        exp.ExpressionValue[0][0] != L'-') {  // variable
      if (get_first_name(exp.ExpressionValue[0]) == exp.ExpressionValue[0] &&
          !isIdentifier(exp.ExpressionValue[0]) &&
          exp.ExpressionValue[0] != L"arguments")
        throw Variable::ExprErr(L"Identifier is invalid");
      Return_Object o = get_object(exp.ExpressionValue[0], scope, all_scope,
                                   this_scope, true, false);
      return o.getConstValue();
    } else if (exp.ExpressionValue.size() == 1 &&
               isStatement(
                   Lpp_base(Variable::clearnull(exp.ExpressionValue[0])))) {
      const Exec_Info &i =
          Lpp(exp.ExpressionValue[0], cmd).eval(scope, all_scope, this_scope);
      if (i.tp != Calc_Value)
        throw i;
      else
        return i.value;
    }
    for (std::vector<std::wstring>::const_reverse_iterator i = p.crbegin();
         i != p.crend(); i++) {
      if (Variable::get_op_priority(*i) != -1) {  // operator
        op = (*i);
        if ((op[op.size() - 1] != L'=' || op == L"==" || op == L"!=" ||
             op == L">=" || op == L"<=") &&
            op != L"||" && op != L"&&" && op != L"++" && op != L"--" &&
            op != L"," && op != L"~" && op != L"!") {
          if ((op == L"-" || op == L"+") && st.size() > 0 && st.size() < 2) {
            res = exp_calc(st[st.size() - 1], scope, all_scope, this_scope)
                      .opcall_single(op);
            is_single = true;
          } else {
            if (st.size() < 2) throw Variable::ExprErr(L"Too few operands");
            // Return_Object l = get_object(st[st.size() -
            // 1].ExpressionValue[0], scope, all_scope, this_scope, true,false);
            // Variable::var tmp = l.getConstValue();
            res = exp_calc(st[st.size() - 1], scope, all_scope, this_scope)
                      .opcall(op, exp_calc(st[st.size() - 2], scope, all_scope,
                                           this_scope));
          }
        } else if (op[op.length() - 1] == L'=') {
          if (st.size() < 2) throw Variable::ExprErr(L"Too few operands");
          if (get_first_name(st[st.size() - 1].StringValue) ==
                  st[st.size() - 1].StringValue &&
              (!isIdentifier(st[st.size() - 1].StringValue))) {
            throw Variable::ExprErr(L"Identifier is invalid");
          }
          Return_Object q;
          if (op == L"=")
            q = get_object(st[st.size() - 1].StringValue, scope, all_scope,
                           this_scope, false, true);
          else
            q = get_object(st[st.size() - 1].StringValue, scope, all_scope,
                           this_scope, true, true);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          if (op == L"=") {
            if (q.lastThis()) {
              if (st[st.size() - 2].tp != Variable::Object)
                throw Variable::ExprErr(
                    L"this object cannot be covered by a non-object value");
            }
            q.getValue() =
                exp_calc(st[st.size() - 2], scope, all_scope, this_scope);
            q.getValue().isConst = newObjectIsConst;
            res = q.getValue();
          } else {
            Variable::var setvalue;
            std::wstring &&setop = op.substr(0, op.length() - 1);

            setvalue = q.getValue().opcall(
                setop,
                exp_calc(st[st.size() - 2], scope, all_scope, this_scope));
            q.getValue() = setvalue;
            q.getValue().isConst = false;
            res = q.getValue();
          }
        } else if (op == L"++" || op == L"--") {
          if (st.size() < 1) throw Variable::ExprErr(L"Too few operands");
          is_single = true;
          Return_Object &&q = get_object(st[st.size() - 1].StringValue, scope,
                                         all_scope, this_scope, true, true);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          Variable::var setvalue;
          std::wstring &&setop = op.substr(0, 1);
          setvalue = q.getValue().opcall(setop, 1);
          const Variable::var backup = q.getValue();
          q.getValue() = setvalue;
          q.getValue().isConst = false;
          res = backup;
        } else if (op == L",") {
          if (st.size() < 2) throw Variable::ExprErr(L"Too few operands");
          res = st[st.size() - 2];
        } else if (op == L"!" || op == L"~") {
          is_single = true;
          if (st.size() < 1) throw Variable::ExprErr(L"Too few operands");
          res = exp_calc(st[st.size() - 1], scope, all_scope, this_scope)
                    .opcall_single(op);
        } else if (op == L"&&") {
          if (st.size() < 2) throw Variable::ExprErr(L"Too few operands");
          if (Variable::var(true)
                  .opcall(L"==", exp_calc(st[st.size() - 1], scope, all_scope,
                                          this_scope))
                  .BooleanValue &&
              Variable::var(true)
                  .opcall(L"==", exp_calc(st[st.size() - 2], scope, all_scope,
                                          this_scope))
                  .BooleanValue) {
            res = true;
          } else
            res = false;
        } else if (op == L"||") {
          if (st.size() < 2) throw Variable::ExprErr(L"Too few operands");
          if (Variable::var(true)
                  .opcall(L"==", exp_calc(st[st.size() - 1], scope, all_scope,
                                          this_scope))
                  .BooleanValue ||
              Variable::var(true)
                  .opcall(L"==", exp_calc(st[st.size() - 2], scope, all_scope,
                                          this_scope))
                  .BooleanValue) {
            res = true;
          } else
            res = false;
        } else
          throw Variable::ExprErr(L"Unknown operator");
        if (!is_single) st.pop_back();
        st.pop_back();
        st.push_back(res);
      } else {
        if (((*i)[0] == L'-' || (*i)[0] == L'+') &&
            Variable::get_op_priority(*i) == -1) {
          st.push_back(exp_calc(Variable::parse((*i).substr(1)), scope,
                                all_scope, this_scope)
                           .opcall_single(std::wstring(1, (*i)[0])));
        } else if (i + 1 != p.crend() &&
                   (((*(i + 1))[(*(i + 1)).size() - 1] == L'=' &&
                     *(i + 1) != L"==" && *(i + 1) != L"!=" &&
                     *(i + 1) != L">=" && *(i + 1) != L"<=") ||
                    (*(i + 1) == L"++" || *(i + 1) == L"--"))) {
          st.push_back(*i);
        } else if ((*i)[0] == L'.') {
          if (i + 1 == p.crend()) {
            throw Variable::SyntaxErr(L"Member operator syntax is invalid");
          }
          st.push_back(Variable::var(
              std::vector<std::wstring>(1, L"(" + *(i + 1) + L")" + (*i))));
          i++;
        } else {
          st.push_back(Variable::parse(*i));
        }
      }
    }
    if (st.size() != 1) throw Variable::ExprErr(L"st.size()!=1");
    return exp_calc(st[0], scope, all_scope, this_scope);
  }
  static const Variable::var exclude_scope(Variable::var scope,
                                           Variable::var temp_scope) {
    for (std::map<std::wstring, Variable::var>::const_iterator x =
             temp_scope.ObjectValue.cbegin();
         x != temp_scope.ObjectValue.cend(); x++) {
      if (scope.ObjectValue.find(x->first) != scope.ObjectValue.cend())
        temp_scope.ObjectValue[x->first].remove();
    }
    temp_scope.update();
    return temp_scope;
  }
  static const std::wstring get_first_name(const std::wstring &p) {
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

 private:
  static const Variable::var update_scope(Variable::var scope,
                                          Variable::var temp_scope) {
    for (std::map<std::wstring, Variable::var>::const_iterator x =
             scope.ObjectValue.cbegin();
         x != scope.ObjectValue.cend(); x++) {
      if (temp_scope.ObjectValue.find(x->first) ==
          temp_scope.ObjectValue.cend())
        scope.ObjectValue[x->first].remove();
      else
        scope.ObjectValue[x->first] = temp_scope.ObjectValue[x->first];
    }
    scope.update();
    return scope;
  }
  static const Variable::var update_scope2(const Variable::var &scope,
                                           Variable::var temp_scope,
                                           const Variable::var &exclude) {
    for (std::map<std::wstring, Variable::var>::const_iterator x =
             temp_scope.ObjectValue.cbegin();
         x != temp_scope.ObjectValue.cend(); x++) {
      if (scope.ObjectValue.find(x->first) == scope.ObjectValue.cend() &&
          exclude.ObjectValue.find(x->first) == exclude.ObjectValue.cend())
        temp_scope.ObjectValue[x->first].remove();
    }
    temp_scope.update();
    return temp_scope;
  }
  static const std::vector<std::wstring> get_name_split(const std::wstring &p) {
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
  static const bool is_native(const std::wstring &x) {
    const std::vector<std::wstring> a = {L"substr", L"join",   L"pop",
                                         L"push",   L"resize", L"toString",
                                         L"split",  L"trim"};
    for (size_t i = 0; i < a.size(); i++) {
      if (a[i] == x) return true;
    }
    return false;
  }
  const Return_Object get_var_index(
      const std::wstring &p, Return_Object &&object, Variable::var &scope,
      Variable::var &all_scope, Variable::var &this_scope,
      const bool nonewobject, const bool startwiththis,
      const bool no_overload) const {
    const std::vector<std::wstring> visit = get_name_split(p);
    Object_Type fin = object.tp;
    Variable::var *now_object = nullptr, *parent_object = nullptr,
                  *this_object = nullptr;
    Variable::var now_const_object, parent_const_object;
    if (fin == is_const_value) {
      now_const_object = object.getConstValue();
      parent_const_object = object.getConstParent();
    } else {
      now_object = &object.getValue();
      parent_object = &object.getParent();
    }
    bool this_keep = startwiththis;
    bool lastisthis = startwiththis;
    // bool isConst = false;  // only for is_native_function
    bool isnative = false;
    if (object.tp == is_pointer && object.getValue().tp == Variable::Object)
      this_object = &object.getValue();
    else
      this_object = &this_scope;
    for (size_t i = 0; i < visit.size(); i++) {
      Variable::var visit_temp =
          exp_calc(Variable::parse(visit[i]), scope, all_scope, *this_object);
      std::wstring find_str;
      if (visit_temp.tp == Variable::String)
        find_str = visit_temp.StringValue;
      else
        find_str = visit_temp.toString();
      if (find_str[0] == L'_' && !this_keep) throw member_cant_visit;
      if (find_str == L"this") {
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Object) throw member_cant_visit;
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Object)
              throw member_cant_visit;
            break;
          }
        }
        fin = is_pointer;
        lastisthis = true;
        now_object = this_object;
        continue;
      } else {
        lastisthis = false;
        this_keep = false;
      }
      if (find_str == L"keys") {
        now_const_object.ArrayValue.clear();
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Object) throw member_not_exist;
            for (std::map<std::wstring, Variable::var>::const_iterator x =
                     now_object->ObjectValue.cbegin();
                 x != now_object->ObjectValue.cend(); x++)
              if (x->first[0] != L'_')
                now_const_object.ArrayValue.push_back(
                    Variable::var(x->first, false));
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Object) throw member_not_exist;
            for (std::map<std::wstring, Variable::var>::iterator x =
                     now_const_object.ObjectValue.begin();
                 x != now_const_object.ObjectValue.cend(); x++)
              if (x->first[0] != L'_')
                now_const_object.ArrayValue.push_back(
                    Variable::var(x->first, false));
            break;
          }
        }
        now_const_object.tp = Variable::Array;
        fin = is_const_value;
        continue;
      } else if (find_str == L"length") {
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Array &&
                now_object->tp != Variable::String)
              throw member_not_exist;
            if (now_object->tp == Variable::Array) {
              now_const_object = (int)now_object->ArrayValue.size();
            } else {
              now_const_object = (int)now_object->StringValue.length();
            }
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Array &&
                now_const_object.tp != Variable::String)
              throw member_not_exist;
            if (now_const_object.tp == Variable::Array) {
              now_const_object = (int)now_const_object.ArrayValue.size();
            } else {
              now_const_object = (int)now_const_object.StringValue.length();
            }
            break;
          }
        }
        fin = is_const_value;
        continue;
      } else if (find_str == L"isConst") {
        switch (fin) {
          case is_pointer: {
            now_const_object = now_object->isConst;
            break;
          }
          case is_const_value: {
            now_const_object = true;
            break;
          }
        }
        fin = is_const_value;
        continue;
      } else if (is_native(find_str) && (!no_overload)) {
        bool is_overloaded = false;
        switch (fin) {
          case is_pointer: {
            if (now_object->tp == Variable::Object &&
                now_object->ObjectValue.find(find_str) !=
                    now_object->ObjectValue.cend()) {
              parent_object = now_object;
              this_object = now_object;
              fin = is_pointer;
              isnative = false;
              is_overloaded = true;
              now_object = &now_object->ObjectValue[find_str];
            }
            break;
          }
          case is_const_value: {
            if (now_const_object.tp == Variable::Object &&
                now_const_object.ObjectValue.find(find_str) !=
                    now_const_object.ObjectValue.cend()) {
              parent_const_object = now_const_object;
              isnative = false;
              fin = is_const_value;
              is_overloaded = true;
              now_const_object = now_const_object.ObjectValue[find_str];
            }
            break;
          }
          default: {
            break;
          }
        }
        if (is_overloaded) continue;
        isnative = true;
        if (find_str == L"substr" || find_str == L"split" ||
            find_str == L"trim") {
          switch (fin) {
            case is_pointer: {
              if (now_object->tp != Variable::String) throw member_not_exist;
              parent_object = now_object;
              this_object = now_object;
              now_const_object = Variable::parse(L"function(){__native__ \"" +
                                                 find_str + L"\";}");
              break;
            }
            case is_const_value: {
              if (now_const_object.tp != Variable::String)
                throw member_not_exist;
              parent_const_object = now_const_object;
              now_const_object = Variable::parse(L"function(){__native__ \"" +
                                                 find_str + L"\"}");
              break;
            }
          }
          fin = is_const_value;
          continue;
        } else if (find_str == L"join") {
          switch (fin) {
            case is_pointer: {
              if (now_object->tp != Variable::Array) throw member_not_exist;
              parent_object = now_object;
              this_object = now_object;
              break;
            }
            case is_const_value: {
              if (now_const_object.tp != Variable::Array)
                throw member_not_exist;
              parent_const_object = now_const_object;
              break;
            }
          }
          now_const_object =
              Variable::parse(L"function(){__native__ \"join\";}");
          fin = is_const_value;
          continue;
        } else if (find_str == L"pop" || find_str == L"push" ||
                   find_str == L"insert" || find_str == L"resize") {
          switch (fin) {
            case is_pointer: {
              if (now_object->tp != Variable::Array) throw member_not_exist;
              parent_object = now_object;
              this_object = now_object;
              now_const_object =
                  Variable::parse(std::wstring(L"function(){__native__ \"") +
                                  find_str + L"\";}");
              break;
            }
            case is_const_value: {
              throw member_not_exist;
            }
          }
          fin = is_const_value;
          continue;
        } else if (find_str == L"toString") {
          switch (fin) {
            case is_pointer: {
              this_object = now_object;
              parent_const_object = *now_object;
              break;
            }
            case is_const_value: {
              parent_const_object = now_const_object;
              break;
            }
          }
          fin = is_const_value;
          now_const_object = Variable::parse(
              std::wstring(L"function(){__native__ \"") + find_str + L"\";}");
          continue;
        }
      } else {
        switch (fin) {
          case is_pointer: {
            this_object = now_object;
            switch (now_object->tp) {
              case Variable::String: {
                try {
                  visit_temp = visit_temp.convert(Variable::Int);
                  if (visit_temp.IntValue < 0) throw nullptr;
                } catch (...) {
                  throw member_not_exist;
                }
                now_const_object = std::wstring(
                    1, now_object->StringValue[(size_t)visit_temp.IntValue]);
                fin = is_const_value;
                break;
              }
              case Variable::Object: {
                parent_object = now_object;
                if (now_object->ObjectValue.find(find_str) ==
                    now_object->ObjectValue.cend()) {
                  if (!nonewobject) {
                    now_object->ObjectValue[find_str] =
                        Variable::var(nullptr, false);
                    now_object = &now_object->ObjectValue[find_str];
                  } else {
                    fin = is_const_value;
                    now_const_object = Variable::var(nullptr, false);
                  }
                } else {
                  now_object = &now_object->ObjectValue[find_str];
                }
                break;
              }
              case Variable::Array: {
                try {
                  visit_temp = visit_temp.convert(Variable::Int);
                  if (visit_temp.IntValue < 0) throw nullptr;
                } catch (...) {
                  throw member_cant_visit;
                }
                try {
                  if ((size_t)visit_temp.IntValue >=
                      now_object->ArrayValue.size()) {
                    if (!nonewobject) {
                      now_object->ArrayValue.resize(
                          (size_t)visit_temp.IntValue + 1);
                      for (size_t i = 0; i < now_object->ArrayValue.size(); i++)
                        now_object->ArrayValue[i].isConst = false;
                    } else {
                      fin = is_const_value;
                      now_const_object = Variable::var(nullptr, false);
                    }
                  }
                  now_object =
                      &now_object->ArrayValue[(size_t)visit_temp.IntValue];
                } catch (...) {
                  throw member_not_exist;
                }
                break;
              }
              default:
                throw member_not_exist;
            }
            break;
          }
          case is_const_value: {
            switch (now_const_object.tp) {
              case Variable::String: {
                try {
                  visit_temp = visit_temp.convert(Variable::Int);
                  if (visit_temp.IntValue < 0) throw member_cant_visit;
                } catch (...) {
                  throw member_cant_visit;
                }
                try {
                  if (now_const_object.StringValue.size() >=
                      (size_t)visit_temp.IntValue)
                    throw nullptr;
                  now_const_object = std::wstring(
                      1, now_const_object
                             .StringValue[(size_t)visit_temp.IntValue]);
                } catch (...) {
                  now_const_object = nullptr;
                }
                break;
              }
              case Variable::Object: {
                if (now_const_object.ObjectValue.find(find_str) ==
                    now_const_object.ObjectValue.cend())
                  now_const_object = nullptr;
                else {
                  parent_const_object = now_const_object;
                  now_const_object = Variable::var(
                      now_const_object.ObjectValue[find_str]);  // strange.
                }
                break;
              }
              case Variable::Array: {
                try {
                  visit_temp = visit_temp.convert(Variable::Int);
                  if (visit_temp.IntValue < 0) throw nullptr;
                } catch (...) {
                  throw member_cant_visit;
                }
                try {
                  if ((size_t)visit_temp.IntValue >=
                      now_const_object.ArrayValue.size())
                    throw nullptr;
                  now_const_object =
                      now_const_object.ArrayValue[(size_t)visit_temp.IntValue];
                } catch (...) {
                  now_const_object = nullptr;
                }
                break;
              }
              default:
                throw member_not_exist;
            }
            break;
          }
        }
      }
    }
    switch (fin) {
      case is_const_value: {
        if (isnative) {
          return Return_Object(nullptr, Variable::var(now_const_object),
                               Variable::var(parent_const_object), *this_object,
                               lastisthis);
        }
        return Return_Object(Variable::var(now_const_object),
                             Variable::var(parent_const_object), *this_object,
                             lastisthis);
      }
      case is_pointer: {
        if (isnative) {
          return Return_Object(nullptr, *now_object, *parent_object,
                               *this_object, lastisthis);
        }
        return Return_Object(*now_object, *parent_object, *this_object,
                             lastisthis);
      }
    }
    return Return_Object();
  }
} Lpp;
}  // namespace Lpp
#endif
