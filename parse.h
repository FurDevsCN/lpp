/*
L++ programming language was under the MIT license.
copyright(c) 2021 nu11ptr team.
*/
#ifndef PARSE_MODULE
#define PARSE_MODULE
#include <functional>

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
        if (a == 0)
          a = 1;
        else if (a == 1)
          a = 0;
      } else if (x[i] == L'\'' && !z) {
        if (a == 0)
          a = 2;
        else if (a == 2)
          a = 0;
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
  const std::wstring toString()
      const {  // Converts the command to std::wstring.
    return name + (args.empty() ? L"" : L" ") + arg2str();
  }
  bool operator==(const Lpp_base &x)
      const {  // Compares two commands.operator!=(...) == !(operator==(...))
    if (x.name == name && x.args == args) return true;
    return false;
  }
  Lpp_base() {}  // The default constructor.
} Lpp_base;
typedef enum Return_Type {
  Expression_Calc_Value = 0,
  Function_Return_Value = 1,
  Throw_Return_Value = 2,
} Return_Type;
typedef struct Return_Value {
  Return_Type tp;                                 // Returned value's type.
  Lpp_base base;                                  // The parsed command.
  Variable::var value;                            // Returned value.
  Variable::var scope;                            // Returned scope.
  Variable::var all_scope;                        // Returned global scope.
  Variable::var this_scope;                       // Returned this object.
  Return_Value() { tp = Expression_Calc_Value; }  // The default constructor.
  Return_Value(const Lpp_base &b, const Return_Type &t, const Variable::var &v,
               const Variable::var &sc, const Variable::var &alsc,
               const Variable::var &thsc) {
    base = b;
    tp = t;
    value = v;
    scope = sc;
    all_scope = alsc;
    this_scope = thsc;
  }  // The constructor.
} Return_Value;
typedef struct Lpp : public Lpp_base {
  typedef std::function<Return_Value(const Lpp &, Variable::var &,
                                     Variable::var &, Variable::var &)>
      CmdType;
  std::map<std::wstring, CmdType> cmd;
  Lpp() : Lpp_base() {}
  Lpp(const std::wstring &x, const std::map<std::wstring, CmdType> &_cmd)
      : Lpp_base(x) {
    cmd = _cmd;
  }  //= Lpp_base(const std::wstring&);
  Return_Value eval(const Variable::var &scope) {
    Variable::var temp = scope;
    temp.isConst = false;
    temp.tp = Variable::Object;
    return eval(temp, temp, temp);
  }
  Return_Value eval(Variable::var &scope, Variable::var &all_scope,
                    Variable::var &this_scope) {
    if (cmd.find(name) != cmd.end() && cmd[name]) {
      return cmd[name](*this, scope, all_scope, this_scope);
    } else if (cmd.find(L"") != cmd.end() && cmd[L""]) {
      return cmd[L""](*this, scope, all_scope, this_scope);
    } else {
      return Return_Value(*this, Throw_Return_Value,
                          Variable::var(L"EvalError"), scope, all_scope,
                          this_scope);
    }
  }

  typedef enum Object_Type {
    is_pointer = 0,
    is_const_value = 1,
    is_native_function = 2,
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
    bool getIsConst() { return isConst; }
    Variable::var &getValue() { return *value; }
    Variable::var &getParent() { return *parent; }
    Variable::var getConstValue() { return const_value; }
    Variable::var getConstParent() { return const_parent; }
    Variable::var &getThis() { return *this_object; }
    Object_Type tp;
    Return_Object() {
      tp = is_const_value;
      value = nullptr;
      isConst = true;
      parent = nullptr;
    }
    Return_Object(const Variable::var &x, Variable::var *p, Variable::var *s) {
      tp = is_const_value;
      const_value = x;
      isConst = true;
      parent = p;
      if (p != nullptr) const_parent = *p;
      this_object = s;
    }
    Return_Object(const Variable::var &x, const Variable::var &p,
                  Variable::var *s) {
      tp = is_const_value;
      const_value = x;
      isConst = true;
      const_parent = p;
      this_object = s;
    }
    Return_Object(Variable::var *x, Variable::var *p, Variable::var *s) {
      tp = is_pointer;
      value = x;
      const_value = *x;
      isConst = false;
      parent = p;
      if (p != nullptr) const_parent = *p;
      this_object = s;
    }
    Return_Object(const std::nullptr_t &x, const Variable::var &w,
                  Variable::var p, Variable::var *s) {
      tp = is_native_function;
      const_value = w;
      isConst = true;
      const_parent = p;
      this_object = s;
    }
    Return_Object(const std::nullptr_t &x, const Variable::var &w,
                  Variable::var *p, Variable::var *s) {
      tp = is_native_function;
      const_value = w;
      isConst = false;
      parent = p;
      if (parent != nullptr) const_parent = *p;
      this_object = s;
    }
  } Return_Object;
  static const std::vector<std::wstring> splitBy(const std::wstring &x,
                                                 const char &delim) {
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
    //{
    // "var",   "if",       "switch", "while",   "for",
    // "break", "continue", "throw",  "catch",   "new",
    // "const", "eval",     "typeof", "default", "try"};
    for (size_t i = 0; i < keyword.size(); i++) {
      if (x == keyword[i]) return true;
    }
    return false;
  }
  const bool isIdentifier(const std::wstring &x) const {
    if (x == L"arguments" || x[0] == L'_' || isKeyword(x)) return false;
    bool flag = false;
    for (size_t i = 0; i < x.length(); i++) {
      if (i == 1) flag = true;
      if (x[i] >= L'0' && x[i] <= L'9') {
        if (!flag) return false;
      } else if ((x[i] >= L'a' && x[i] <= L'z') || x[i] == L'_')
        continue;
      else
        return false;
    }
    return true;
  }
  const bool isStatement(const std::wstring &x, const Variable::var &scope,
                         const Variable::var &all_scope,
                         const Variable::var &this_scope) const {
    Lpp_base a = Lpp_base(x);
    try {
      return isKeyword(a.name) ||
             (!a.args.empty() &&
              exp_calc(Variable::parse(a.args[0]), scope, all_scope, this_scope)
                      .tp == Variable::Array);
    } catch (...) {
      return false;
    }
  }
  Return_Object get_object(const std::wstring &n, Variable::var &scope,
                           Variable::var &all_scope, Variable::var &this_scope,
                           const size_t &count_dont_parse,
                           const bool &nonewobject) const {
    if (get_first_name(n) == L"") {
      throw member_not_exist;
    } else if (get_first_name(n) == n && count_dont_parse == 1) {
      return Return_Object(&scope, &this_scope, &this_scope);
    } else if (get_first_name(n) == n) {
      if (n == L"this") {
        return Return_Object(this_scope, &this_scope, &this_scope);
      } else if (scope.ObjectValue.find(n) != scope.ObjectValue.end()) {
        return Return_Object(&scope.ObjectValue[n], &this_scope, &this_scope);
      } else if (all_scope.ObjectValue.find(n) != all_scope.ObjectValue.end()) {
        return Return_Object(&all_scope.ObjectValue[n], &this_scope,
                             &this_scope);
      } else {
        try {
          if (Variable::parse(n).tp == Variable::Expression) {
            if (nonewobject == true)
              throw nullptr;
            else {
              scope.ObjectValue[n] = Variable::var(nullptr, false);
              return Return_Object(&scope.ObjectValue[n], &this_scope,
                                   &this_scope);
            }
          };
          return Return_Object(
              exp_calc(Variable::parse(n), scope, all_scope, this_scope),
              &scope, &scope);
        } catch (...) {
          throw member_not_exist;
        }
      }
    } else {
      std::wstring fst_nme = get_first_name(n);
      Variable::var tmp_obj;
      if (fst_nme == L"this") {
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(&this_scope, &this_scope, &this_scope), scope,
            all_scope, this_scope, count_dont_parse, nonewobject, true);
      } else if (scope.ObjectValue.find(fst_nme) != scope.ObjectValue.end()) {
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(&scope.ObjectValue[fst_nme], &scope, &scope), scope,
            all_scope, this_scope, count_dont_parse, nonewobject, false);
      } else if (all_scope.ObjectValue.find(fst_nme) !=
                 all_scope.ObjectValue.end()) {
        return get_var_index(n.substr(fst_nme.length()),
                             Return_Object(&all_scope.ObjectValue[fst_nme],
                                           &all_scope, &all_scope),
                             scope, all_scope, this_scope, count_dont_parse,
                             nonewobject, false);
      } else
        return get_var_index(
            n.substr(fst_nme.length()),
            Return_Object(exp_calc(Variable::parse(fst_nme), scope, all_scope,
                                   this_scope),
                          &scope, &scope),
            scope, all_scope, this_scope, count_dont_parse, nonewobject, false);
    }
  }
  const Variable::var exp_calc(const Variable::var &exp,
                               const Variable::var &scope,
                               const Variable::var &all_scope,
                               const Variable::var &this_scope,
                               const bool newObjectIsConst = false) const {
    Variable::var _scope = scope;
    Variable::var _all_scope = all_scope;
    Variable::var _this_scope = this_scope;
    return exp_calc(exp, _scope, _all_scope, _this_scope, newObjectIsConst);
  }
  const Variable::var exp_calc(const Variable::var &exp, Variable::var &scope,
                               Variable::var &all_scope,
                               Variable::var &this_scope,
                               const bool newObjectIsConst = false) const {
    std::vector<std::wstring> p = exp.ExpressionValue;
    std::vector<Variable::var> st;
    std::wstring op;
    Variable::var res;
    if (exp.tp != Variable::Expression) {
      if (exp.tp == Variable::Object || exp.tp == Variable::Array) {
        bool ret_const = exp.isConst;
        switch ((size_t)exp.tp) {
          case Variable::Object: {
            std::map<std::wstring, Variable::var> x;
            bool flag = false;
            for (std::map<std::wstring, Variable::var>::const_iterator i =
                     exp.ObjectValue.cbegin();
                 i != exp.ObjectValue.cend(); i++) {
              flag = i->second.isConst;
              x[i->first] = exp_calc(i->second, scope, all_scope,
                                     this_scope);  // calc values of the object
              x[i->first].isConst = flag;
            }
            return Variable::var(x, ret_const);
          }
          case Variable::Array: {
            std::vector<Variable::var> x(exp.ArrayValue.size());
            for (size_t i = 0; i < exp.ArrayValue.size(); i++) {
              x[i] = exp_calc(exp.ArrayValue[i], scope, all_scope,
                              this_scope);  // calc members of the array
              x[i].isConst = false;
            }
            return Variable::var(x, ret_const);
          }
        }
      } else
        return exp;
    }  // return var(exp);
    if (exp.ExpressionValue.size() == 0) return Variable::var(nullptr);
    if (exp.ExpressionValue.size() == 1 &&
        !isStatement(exp.ExpressionValue[0], scope, all_scope, this_scope) &&
        exp.ExpressionValue[0][0] != L'-') {
      Return_Object o = get_object(exp.ExpressionValue[0], scope, all_scope,
                                   this_scope, 0, true);
      if (o.tp == is_const_value || o.tp == is_native_function)
        return o.getConstValue();
      else
        return o.getValue();
    }  // is a variable
    for (std::vector<std::wstring>::const_reverse_iterator i = p.crbegin();
         i != p.crend(); i++) {
      if (Variable::get_op_priority(*i) != -1) {  // operator
        op = (*i);
        if (op == L"+")
          res = Variable::var(st[st.size() - 1] + st[st.size() - 2]);
        else if (op == L"-")
          res = Variable::var(st[st.size() - 1] - st[st.size() - 2]);
        else if (op == L"*")
          res = Variable::var(st[st.size() - 1] * st[st.size() - 2]);
        else if (op == L"/")
          res = Variable::var(st[st.size() - 1] / st[st.size() - 2]);
        else if (op == L"%")
          res = Variable::var(st[st.size() - 1] % st[st.size() - 2]);
        else if (op == L"==")
          res = Variable::var(st[st.size() - 1] == st[st.size() - 2]);
        else if (op == L"=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = st[st.size() - 2];
          q.getValue().isConst = newObjectIsConst;
          res = Variable::var(q.getValue());
        } else if (op == L"+=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() + st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"-=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() - st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"*=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() * st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"/=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() / st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"%=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() % st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"&=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() & st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"|=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() - st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"^=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue() ^ st[st.size() - 2];
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L">>=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue().rightmove_signed(st[st.size() - 2]);
          res = Variable::var(q.getValue());
        } else if (op == L"<<=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue().leftmove(st[st.size() - 2]);
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L">>>=") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          q.getValue() = q.getValue().rightmove_unsigned(st[st.size() - 2]);
          q.getValue().isConst = false;
          res = Variable::var(q.getValue());
        } else if (op == L"++") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          Variable::var backup = q.getValue();
          q.getValue() = q.getValue() + 1;
          q.getValue().isConst = false;
          res = backup;
        } else if (op == L"--") {
          Return_Object q = get_object(st[st.size() - 1].StringValue, scope,
                                       all_scope, this_scope, 0, false);
          if (q.tp != is_pointer || q.getValue().isConst) {
            throw Variable::ExprErr(L"Set value failed");
          }
          Variable::var backup = q.getValue();
          q.getValue() = q.getValue() - 1;
          q.getValue().isConst = false;
          res = backup;
        } else if (op == L",") {
          res = Variable::var(st[st.size() - 2]);
        } else if (op == L"<")
          res = Variable::var(st[st.size() - 1] < st[st.size() - 2]);
        else if (op == L">")
          res = Variable::var(st[st.size() - 1] > st[st.size() - 2]);
        else if (op == L"<=")
          res = Variable::var(st[st.size() - 1] <= st[st.size() - 2]);
        else if (op == L">=")
          res = Variable::var(st[st.size() - 1] >= st[st.size() - 2]);
        else if (op == L"!=")
          res = Variable::var(st[st.size() - 1] != st[st.size() - 2]);
        else if (op == L"&&")
          res = Variable::var(st[st.size() - 1] && st[st.size() - 2]);
        else if (op == L"||")
          res = Variable::var(st[st.size() - 1] || st[st.size() - 2]);
        else if (op == L"!")
          res = Variable::var(!st[st.size() - 1]);
        else if (op == L"&")
          res = Variable::var(st[st.size() - 1] & st[st.size() - 2]);
        else if (op == L"|")
          res = Variable::var(st[st.size() - 1] | st[st.size() - 2]);
        else if (op == L"^")
          res = Variable::var(st[st.size() - 1] ^ st[st.size() - 2]);
        else if (op == L"~")
          res = Variable::var(~st[st.size() - 1]);
        else if (op == L">>")
          res = st[st.size() - 1].rightmove_signed(st[st.size() - 2]);
        else if (op == L">>>")
          res = st[st.size() - 1].rightmove_unsigned(st[st.size() - 2]);
        else if (op == L"<<")
          res = st[st.size() - 1].leftmove(st[st.size() - 2]);
        else
          throw Variable::ExprErr(L"Unknown operator");
        if (op != L"!" && op != L"~" && op != L"++" && op != L"--")
          st.pop_back();
        st.pop_back();
        st.push_back(res);
      } else {
        if ((*i)[0] == L'-' && (*i) != L"--") {
          //*-1
          res = Variable::var(exp_calc(Variable::parse((*i).substr(1)), scope,
                                       all_scope, this_scope) *
                              -1);
          st.push_back(res);
        } else {
          if (i + 1 != p.crend() &&
              (*(i + 1) == L"=" || *(i + 1) == L"+=" || *(i + 1) == L"-=" ||
               *(i + 1) == L"*=" || *(i + 1) == L"/=" || *(i + 1) == L"%=" ||
               *(i + 1) == L"&=" || *(i + 1) == L"|=" || *(i + 1) == L"^=" ||
               *(i + 1) == L">>=" || *(i + 1) == L"<<=" ||
               *(i + 1) == L">>>=" || *(i + 1) == L"++" || *(i + 1) == L"--")) {
            st.push_back(Variable::var(*i));
          } else {
            Return_Value temp = Lpp(*i, cmd).eval(scope, all_scope, this_scope);
            if (temp.tp == Throw_Return_Value)
              throw Variable::SyntaxErr(L"Eval failed");
            else
              st.push_back(temp.value);
          }
        }
      }
    }
    if (st.size() != 1) throw Variable::ExprErr(L"st.size()!=1");
    return st[0];
  }
  static const Variable::var update_scope(Variable::var scope,
                                          Variable::var temp_scope) {
    for (std::map<std::wstring, Variable::var>::const_iterator x =
             scope.ObjectValue.cbegin();
         x != scope.ObjectValue.cend(); x++) {
      if (temp_scope.ObjectValue.find(x->first) == temp_scope.ObjectValue.end())
        scope.ObjectValue[x->first].remove();
      else
        scope.ObjectValue[x->first] = temp_scope.ObjectValue[x->first];
    }
    scope.update();
    return scope;
  }

 private:
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
      if (p[i] == L'[' && a == 0 && j == 1) {
        if (temp != L"") visit.push_back(temp), temp = L"";
      } else if (p[i] == L']' && a == 0 && j == 0) {
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
  Return_Object get_var_index(const std::wstring &p, Return_Object &&object,
                              Variable::var &scope, Variable::var &all_scope,
                              Variable::var &this_scope,
                              const size_t &count_dont_parse,
                              const bool &nonewobject,
                              const bool &startwiththis) const {
    const std::vector<std::wstring> visit = get_name_split(p);
    Object_Type fin = object.tp;
    Variable::var *now_object = &object.getValue(), *parent_object = nullptr,
                  *this_object = &this_scope;
    Variable::var now_const_object = object.getConstValue(),
                  parent_const_object = nullptr;
    bool dont_return_nonconst = startwiththis;
    bool this_keep = startwiththis;
    bool isConst = false;  // only for is_native_function
    for (size_t i = 0; i < visit.size() - count_dont_parse; i++) {
      Variable::var visit_temp =
          exp_calc(Variable::parse(visit[i]), scope, all_scope, *this_object);
      std::wstring find_str;
      if (visit_temp.tp == Variable::String)
        find_str = visit_temp.StringValue;
      else
        find_str = visit_temp.toString();
      if (find_str[0] == L'_' && !this_keep) throw member_cant_visit;
      if (find_str == L"this") {
        fin = is_pointer;
        dont_return_nonconst = true;
        now_object = this_object;
        continue;
      } else {
        this_keep = false;
        dont_return_nonconst = false;
      }
      if (find_str == L"keys") {
        now_const_object.ArrayValue.clear();
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Object) throw member_not_exist;
            for (std::map<std::wstring, Variable::var>::iterator x =
                     now_object->ObjectValue.begin();
                 x != now_object->ObjectValue.end(); x++)
              if (x->first[0] != L'_')
                now_const_object.ArrayValue.push_back(
                    Variable::var(x->first, false));
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Object) throw member_not_exist;
            for (std::map<std::wstring, Variable::var>::iterator x =
                     now_const_object.ObjectValue.begin();
                 x != now_const_object.ObjectValue.end(); x++)
              if (x->first[0] != L'_')
                now_const_object.ArrayValue.push_back(
                    Variable::var(x->first, false));
            break;
          }
          case is_native_function: {
            throw member_not_exist;
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
              now_const_object =
                  Variable::var((int)now_object->ArrayValue.size());
            } else {
              now_const_object =
                  Variable::var((int)now_object->StringValue.length());
            }
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Array &&
                now_const_object.tp != Variable::String)
              throw member_not_exist;
            if (now_const_object.tp == Variable::Array) {
              now_const_object =
                  Variable::var((int)now_const_object.ArrayValue.size());
            } else {
              now_const_object =
                  Variable::var((int)now_const_object.StringValue.length());
            }
            break;
          }
          case is_native_function: {
            throw member_not_exist;
          }
        }
        fin = is_const_value;
        continue;
      } else if (find_str == L"isConst") {
        switch (fin) {
          case is_pointer: {
            now_const_object = Variable::var(now_object->isConst);
            break;
          }
          case is_const_value: {
            now_const_object = Variable::var(true);
            break;
          }
          case is_native_function: {
            now_const_object = Variable::var(true);
            break;
          }
        }
        fin = is_const_value;
        continue;
      } else if (find_str == L"substr") {
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::String) throw member_not_exist;
            parent_object = now_object;
            isConst = false;
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::String) throw member_not_exist;
            parent_const_object = now_const_object;
            isConst = true;
            break;
          }
          case is_native_function: {
            throw member_not_exist;
          }
        }
        now_const_object = Variable::parse(L"{__native__ \"substr\";}");
        fin = is_native_function;
        continue;
      } else if (find_str == L"join") {
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Array) throw member_not_exist;
            parent_object = now_object;
            isConst = false;
            break;
          }
          case is_const_value: {
            if (now_const_object.tp != Variable::Array) throw member_not_exist;
            parent_const_object = now_const_object;
            isConst = true;
            break;
          }
          case is_native_function: {
            throw member_not_exist;
          }
        }
        now_const_object = Variable::parse(L"{__native__ \"join\";}");
        fin = is_native_function;
        continue;
      } else if (find_str == L"pop" || find_str == L"push" ||
                 find_str == L"insert" || find_str == L"resize") {
        switch (fin) {
          case is_pointer: {
            if (now_object->tp != Variable::Array) throw member_not_exist;
            parent_object = now_object;
            isConst = false;
            now_const_object = Variable::parse(std::wstring(L"{__native__ \"") +
                                               find_str + L"\";}");
            break;
          }
          case is_const_value:
          case is_native_function: {
            throw member_not_exist;
          }
        }
        fin = is_native_function;
        continue;
      } else if (find_str == L"convert" || find_str == L"toString") {
        switch (fin) {
          case is_pointer: {
            parent_const_object = *now_object;
            break;
          }
          case is_native_function:
          case is_const_value: {
            parent_const_object = now_const_object;
            break;
          }
        }
        isConst = true;
        fin = is_native_function;
        now_const_object = Variable::parse(std::wstring(L"{__native__ \"") +
                                           find_str + L"\";}");
        continue;
      }
      switch (fin) {
        case is_pointer: {
          this_object = now_object;
          switch (now_object->tp) {
            case Variable::String: {
              if (visit_temp.tp != Variable::Int) {
                throw member_not_exist;
              } else {
                now_const_object = Variable::var(std::wstring(
                    1, now_object->StringValue[(size_t)visit_temp.IntValue]));
              }
              fin = is_const_value;
              break;
            }
            case Variable::Object: {
              if (now_object->ObjectValue.find(find_str) ==
                  now_object->ObjectValue.end()) {
                if (!nonewobject)
                  now_object->ObjectValue[find_str] =
                      Variable::var(nullptr, now_object->isConst);
                else
                  throw member_not_exist;
              }
              now_object = &now_object->ObjectValue[find_str];
              break;
            }
            case Variable::Array: {
              try {
                visit_temp = visit_temp.convert(Variable::Int);
                if ((size_t)visit_temp.IntValue >=
                    now_object->ArrayValue.size()) {
                  if (!nonewobject) {
                    now_object->ArrayValue.resize((size_t)visit_temp.IntValue +
                                                  1);
                    for (size_t i = 0; i < now_object->ArrayValue.size(); i++)
                      now_object->ArrayValue[i].isConst = false;
                  } else
                    throw member_not_exist;
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
                if (now_const_object.StringValue.size() >=
                    (size_t)visit_temp.IntValue)
                  throw nullptr;
                now_const_object = Variable::var(std::wstring(
                    1,
                    now_const_object.StringValue[(size_t)visit_temp.IntValue]));
              } catch (...) {
                now_const_object = Variable::var();
              }
              break;
            }
            case Variable::Object: {
              if (now_const_object.ObjectValue.find(find_str) ==
                  now_const_object.ObjectValue.end())
                throw member_not_exist;
              now_const_object = now_const_object.ObjectValue[find_str];
              break;
            }
            case Variable::Array: {
              try {
                visit_temp = visit_temp.convert(Variable::Int);
                if ((size_t)visit_temp.IntValue >=
                    now_const_object.ArrayValue.size())
                  throw nullptr;
                now_const_object =
                    now_const_object.ArrayValue[(size_t)visit_temp.IntValue];
              } catch (...) {
                now_const_object = Variable::var();
              }
              break;
            }
            default:
              throw member_not_exist;
          }
          break;
        }
        case is_native_function: {
          throw member_not_exist;
        }
      }
      dont_return_nonconst = false;
    }
    switch (fin) {
      case is_const_value: {
        return Return_Object(now_const_object, parent_const_object,
                             this_object);
      }
      case is_native_function: {
        if (isConst)
          return Return_Object(nullptr, now_const_object, parent_const_object,
                               this_object);
        else
          return Return_Object(nullptr, now_const_object, parent_object,
                               this_object);
      }
      case is_pointer: {
        if (!dont_return_nonconst)
          return Return_Object(now_object, parent_object, this_object);
        else
          return Return_Object(*now_object, parent_const_object, this_object);
      }
    }
    return Return_Object();
  }
} Lpp;
}  // namespace Lpp
#endif
