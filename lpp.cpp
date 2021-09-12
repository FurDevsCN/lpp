/*
L++ programming language was under the MIT license.
Copyright(c) 2021 nu11ptr team.
*/
/*
compile flags:
ENABLE_EXT : add hotpatch "load" and "import".
*/
// your compile flag here.
#include <fstream>
#include <iostream>

#include "./parse.h"
#define VERSION_INFO "1.2.1-20210912_beta"
typedef class str_factory {
  std::wstring fmt;

 public:
  str_factory() {}
  str_factory(std::wstring f) {
    fmt = f;
    return;
  }
  std::wstring format(std::vector<std::wstring> value = {}) {
    bool flag = false;
    std::wstring fin;
    for (size_t i = 0; i < fmt.length(); i++) {
      if (fmt[i] == L'\\') flag = !flag;
      if (fmt[i] == L'{' && !flag) {
        size_t j = i;
        while (j < fmt.length() && fmt[j] != L'}') j++;
        fin += value.at(std::stoi(fmt.substr(i + 1, j - 1)));
        i = j;
        continue;
      }
      if (!flag) fin += fmt[i];
    }
    return fin;
  }
} str_factory;
const std::map<std::wstring, Lpp::Lpp::CmdType> getFunc() {
  std::map<std::wstring, Lpp::Lpp::CmdType> temp;
  temp[L"return"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() == 0)
      return Lpp::Return_Value(cmd, Lpp::Function_Return_Value,
                               Variable::var(nullptr), scope, all_scope,
                               this_scope);
    try {
      return Lpp::Return_Value(cmd, Lpp::Function_Return_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope),
                               scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp[L"throw"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    }
    try {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope),
                               scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp[L"var"] = temp[L"const"] =
      [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
         Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    try {
      Variable::var s;
      for (size_t i = 0; i < cmd.args.size(); i++) {
        std::vector<std::wstring> exp = cmd.splitBy(cmd.args[i], L'=');
        if (exp.size() == 1 && cmd.isIdentifier(exp[0]) &&
            scope.ObjectValue.find(exp[0]) == scope.ObjectValue.end()) {
          scope.ObjectValue[exp[0]] =
              Variable::var(nullptr, cmd.name == L"const");
          continue;
        }
        if (exp.size() != 2 || !cmd.isIdentifier(exp[0]))
          throw Variable::SyntaxErr(L"Expression invaild");
        if (scope.ObjectValue.find(exp[0]) != scope.ObjectValue.end())
          throw nullptr;
        scope.ObjectValue[exp[0]] =
            cmd.exp_calc(Variable::parse(exp[1]), scope, all_scope, this_scope);
        scope.ObjectValue[exp[0]].isConst = (cmd.name == L"const");
      }
    } catch (Variable::SyntaxErr&) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
    return Lpp::Return_Value(
        cmd, Lpp::Expression_Calc_Value,
        cmd.exp_calc(Variable::var(nullptr), scope, all_scope, this_scope),
        scope, all_scope, this_scope);
  };
  temp[L"delete"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
    size_t count = 0;
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    for (size_t i = 0; i < cmd.args.size(); i++) {
      Lpp::Lpp::Return_Object y;
      try {
        y = cmd.get_object(cmd.args[i], scope, all_scope, this_scope, 0, true);
        if (y.tp == Lpp::Lpp::is_const_value || y.getValue().isConst)
          throw nullptr;
        y.getValue().remove();
      } catch (...) {
        continue;
      }
      y.getParent().update();
      count++;
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var((int)count), scope, all_scope,
                             this_scope);
  };
  temp[L"if"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                   Variable::var& all_scope,
                   Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() < 2 || cmd.args.size() > 3)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var exp;
    Variable::var fn;
    Variable::var temp_scope = scope;
    Lpp::Return_Value res;
    try {
      exp = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                         this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
    if (Variable::var(true) == exp) {
      try {
        fn = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
        if (cmd.args[1] == L"{}") fn.tp = Variable::Function;
        if (fn.tp != Variable::Function) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var(L"SyntaxError"), scope,
                                 all_scope, this_scope);
      }
      for (size_t i = 0; i < fn.FunctionValue.value.size(); i++) {
        res = Lpp::Lpp(fn.FunctionValue.value[i], cmd.cmd)
                  .eval(temp_scope, all_scope, this_scope);
        if (res.tp != Lpp::Expression_Calc_Value) {
          res.scope = scope;
          return res;
        }
        scope = cmd.update_scope(scope, temp_scope);
      }
    } else if (cmd.args.size() == 3 && cmd.args[2] != L"") {
      try {
        fn = cmd.exp_calc(Variable::parse(cmd.args[2]), scope, all_scope,
                          this_scope);
        if (cmd.args[2] == L"{}") fn.tp = Variable::Function;
        if (fn.tp != Variable::Function) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var(L"SyntaxError"), scope,
                                 all_scope, this_scope);
      }
      for (size_t i = 0; i < fn.FunctionValue.value.size(); i++) {
        res = Lpp::Lpp(fn.FunctionValue.value[i], cmd.cmd)
                  .eval(temp_scope, all_scope, this_scope);
        if (res.tp != Lpp::Expression_Calc_Value) {
          res.scope = scope;
          return res;
        }
        scope = cmd.update_scope(scope, temp_scope);
      }
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(Variable::var(true) == exp), scope,
                             all_scope, this_scope);
  };
  temp[L"switch"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var x;
    Variable::var exp;
    Variable::var func;
    bool break_flag = false;
    try {
      exp = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                         this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
    try {
      x = Variable::parse(cmd.args[1]);
      if (x.tp != Variable::Array || x.ArrayValue.size() < 1) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    }
    for (size_t i = 0; i < x.ArrayValue.size(); i++) {
      if (i + 1 != x.ArrayValue.size()) {
        Variable::var val;
        try {
          if (x.ArrayValue[i].tp != Variable::Expression ||
              x.ArrayValue[i].ExpressionValue.size() != 3 ||
              x.ArrayValue[i].ExpressionValue[0] != L"=")
            throw nullptr;
          val =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[1]),
                           scope, all_scope, this_scope);
          func =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[2]),
                           scope, all_scope, this_scope);
          if (x.ArrayValue[i].ExpressionValue[2] == L"{}")
            func.tp = Variable::Function;
          if (func.tp != Variable::Function) throw nullptr;
        } catch (...) {
          return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                   Variable::var(L"SyntaxError"), scope,
                                   all_scope, this_scope);
        }
        if (exp == val) {
          Variable::var temp_scope = scope;
          Lpp::Return_Value res;
          for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
            if (Lpp::Lpp_base(func.FunctionValue.value[i]).name == L"break") {
              break_flag = 1;
              break;
            } else
              res = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd)
                        .eval(temp_scope, all_scope, this_scope);
            if (res.tp != Lpp::Expression_Calc_Value) {
              res.scope = scope;
              return res;
            }
            scope = cmd.update_scope(scope, temp_scope);
          }
        }
      } else {
        try {
          if (x.ArrayValue[i].tp != Variable::Expression ||
              x.ArrayValue[i].ExpressionValue.size() != 3 ||
              x.ArrayValue[i].ExpressionValue[0] != L"=" ||
              x.ArrayValue[i].ExpressionValue[1] != L"default")
            throw nullptr;
          func =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[2]),
                           scope, all_scope, this_scope);
          if (x.ArrayValue[i].ExpressionValue[2] == L"{}")
            func.tp = Variable::Function;
          if (func.tp != Variable::Function) throw nullptr;
        } catch (...) {
          return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                   Variable::var(nullptr), scope, all_scope,
                                   this_scope);
        }
        Variable::var temp_scope = scope;
        Lpp::Return_Value res;
        for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
          if (Lpp::Lpp_base(func.FunctionValue.value[i]).name == L"break") {
            break_flag = 1;
            break;
          } else
            res = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd)
                      .eval(temp_scope, all_scope, this_scope);
          if (res.tp != Lpp::Expression_Calc_Value) {
            res.scope = scope;
            return res;
          }
          scope = cmd.update_scope(scope, temp_scope);
        }
      }
      if (break_flag) break;
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };
  temp[L"while"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var exp;
    Variable::var func;
    try {
      exp = Variable::parse(cmd.args[0]);
      func = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
      if (cmd.args[1] == L"{}") func.tp = Variable::Function;
      if (func.tp != Variable::Function) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
    while (true) {
      Variable::var exp_res;
      try {
        exp_res = cmd.exp_calc(exp, scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var(L"ExpressionError"), scope,
                                 all_scope, this_scope);
      }
      if (Variable::var(true) == exp_res) {
        Variable::var temp_scope = scope;
        Lpp::Return_Value res;
        bool break_flag = 0;
        for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
          Lpp::Lpp l = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd);
          if (l.name == L"continue") {
            break;
          } else if (l.name == L"break") {
            break_flag = 1;
            break;
          } else
            res = l.eval(temp_scope, all_scope, this_scope);
          if (res.tp != Lpp::Expression_Calc_Value) {
            res.scope = scope;
            return res;
          }
          scope = cmd.update_scope(scope, temp_scope);
        }
        if (break_flag) break;
      } else
        break;
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };
  temp[L"for"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2 || cmd.args[0][0] != L'(' ||
        cmd.args[0][cmd.args[0].length() - 1] != L')')
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    std::wstring start;
    Variable::var exp;
    std::wstring routine;
    Variable::var func;
    Variable::var temp_scope;
    Lpp::Return_Value res;
    Variable::var exp_res;
    bool break_flag = false;
    try {
      std::vector<std::wstring> temp =
          cmd.splitBy(cmd.args[0].substr(1, cmd.args[0].length() - 2), L';');
      func = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
      if (temp.size() == 2) temp.push_back(L"");
      if (temp.size() != 3) throw nullptr;
      start = temp[0];
      exp = Variable::parse(temp[1] == L"" ? L"true" : temp[1]);
      routine = temp[2];
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    }
    temp_scope = scope;
    res = Lpp::Lpp(start, cmd.cmd).eval(temp_scope, all_scope, this_scope);
    if (res.tp != Lpp::Expression_Calc_Value) {
      res.scope = scope;
      return res;
    }
    while (true) {
      try {
        exp_res = cmd.exp_calc(exp, temp_scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var(L"ExpressionError"), scope,
                                 all_scope, this_scope);
      }
      if (Variable::var(true) == exp_res) {
        for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
          Lpp::Lpp l = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd);
          if (l.name == L"continue") {
            break;
          } else if (l.name == L"break") {
            break_flag = 1;
            break;
          } else
            res = l.eval(temp_scope, all_scope, this_scope);
          if (res.tp != Lpp::Expression_Calc_Value) {
            res.scope = scope;
            return res;
          }
          scope = cmd.update_scope(scope, temp_scope);
        }
        if (break_flag) break;
      } else
        break;
      res = Lpp::Lpp(routine, cmd.cmd).eval(temp_scope, all_scope, this_scope);
      if (res.tp != Lpp::Expression_Calc_Value) {
        res.scope = scope;
        return res;
      }
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };
  temp[L"new"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> Lpp::Return_Value {
    Variable::var func;
    try {
      if (cmd.args.size() < 1 || cmd.args.size() > 2) throw nullptr;
      func = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                          this_scope);
      if (cmd.args[0] == L"{}") func.tp = Variable::Function;
      if (func.tp != Variable::Function) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    }
    Variable::var temp_scope;
    Variable::var temp_this_scope;
    temp_scope.isConst = false;
    temp_scope.tp = Variable::Object;
    temp_this_scope.isConst = false;
    temp_this_scope.tp = Variable::Object;
    if (cmd.args.size() >= 2) {
      try {
        temp_scope.ObjectValue[L"arguments"] = cmd.exp_calc(
            Variable::parse(cmd.args[1]), scope, all_scope, this_scope);
        if (temp_scope.ObjectValue[L"arguments"].tp != Variable::Array)
          throw nullptr;
        temp_scope.ObjectValue[L"arguments"].isConst = true;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var(L"SyntaxError"), scope,
                                 all_scope, this_scope);
      }
    } else
      temp_scope.ObjectValue[L"arguments"] =
          Variable::var(std::vector<Variable::var>(), true);
    for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
      Lpp::Return_Value res;
      res = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd)
                .eval(temp_scope, all_scope, temp_this_scope);
      if (res.tp == Lpp::Function_Return_Value) {
        break;
      } else if (res.tp == Lpp::Throw_Return_Value) {
        res.this_scope = this_scope;
        res.scope = scope;
        return res;
      }
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value, temp_this_scope,
                             scope, all_scope, this_scope);
  };
  temp[L"try"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var hook;
    Variable::var fn;
    Variable::var temp_scope = scope;
    Lpp::Return_Value res;
    try {
      const std::vector<std::wstring> a = cmd.splitBy(cmd.args[1], L'=');
      if (a.size() != 3 || a[1] != L"catch") throw nullptr;
      hook = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                          this_scope);
      if (cmd.args[0] == L"{}") hook.tp = Variable::Function;
      fn = cmd.exp_calc(Variable::parse(a[2]), scope, all_scope, this_scope);
      if (a[2] == L"{}") hook.tp = Variable::Function;
      if (hook.tp != Variable::Function || fn.tp != Variable::Function)
        throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    }
    for (size_t i = 0; i < hook.FunctionValue.value.size(); i++) {
      res = Lpp::Lpp(hook.FunctionValue.value[i], cmd.cmd)
                .eval(temp_scope, all_scope, this_scope);
      if (res.tp == Lpp::Throw_Return_Value) {
        temp_scope.ObjectValue[L"err"] = res.value;
        temp_scope.ObjectValue[L"err"].isConst = true;
        for (size_t i = 0; i < fn.FunctionValue.value.size(); i++) {
          res = Lpp::Lpp(fn.FunctionValue.value[i], cmd.cmd)
                    .eval(temp_scope, all_scope, this_scope);
          if (res.tp != Lpp::Expression_Calc_Value) {
            res.scope = scope;
            return res;
          }
          scope = cmd.update_scope(scope, temp_scope);
        }
        return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                 Variable::var(nullptr), scope, all_scope,
                                 this_scope);
      } else if (res.tp == Lpp::Function_Return_Value) {
        res.scope = scope;
        return res;
      }
      scope = cmd.update_scope(scope, temp_scope);
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };
  temp[L"typeof"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    try {
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(Variable::getTypeStr(
                                   cmd.exp_calc(Variable::parse(cmd.args[0]),
                                                scope, all_scope, this_scope)
                                       .tp)),
                               scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp[L"eval"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var op = cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                    all_scope, this_scope);
    if (op.tp != Variable::String)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"SyntaxError"), scope, all_scope,
                               this_scope);
    return Lpp::Lpp(op.StringValue, cmd.cmd).eval(scope, all_scope, this_scope);
  };
  temp[L"break"] = temp[L"continue"] = temp[L"default"] = temp[L"catch"] =
      temp[L"__native__"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                               Variable::var& all_scope,
                               Variable::var& this_scope) -> Lpp::Return_Value {
    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                             Variable::var(L"SyntaxError"), scope, all_scope,
                             this_scope);
  };
  temp[L""] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                 Variable::var& all_scope,
                 Variable::var& this_scope) -> Lpp::Return_Value {
    try {
      if (cmd.isStatement(cmd.toString(), scope, all_scope, this_scope)) {
        Variable::var func_temp = cmd.exp_calc(Variable::parse(cmd.name), scope,
                                               all_scope, this_scope);
        if (func_temp.tp != Variable::Function && cmd.args.size() != 0) {
          return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                   Variable::var(L"SyntaxError"), scope,
                                   all_scope, this_scope);
        } else if (func_temp.tp == Variable::Function && cmd.args.size() == 1) {
          Variable::var args_temp = cmd.exp_calc(Variable::parse(cmd.args[0]),
                                                 scope, all_scope, this_scope);
          if (args_temp.tp != Variable::Array) {
            return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                     Variable::var(L"SyntaxError"), scope,
                                     all_scope, this_scope);
          }
          // test native
          if (func_temp.FunctionValue.value.size() == 1) {
            Lpp::Lpp_base x = Lpp::Lpp_base(func_temp.FunctionValue.value[0]);
            if (x.name == L"__native__" && x.args.size() == 1) {
              // native function
              Lpp::Lpp::Return_Object fn_native;
              try {
                fn_native = cmd.get_object(cmd.name, scope, all_scope,
                                           this_scope, 0, true);
              } catch (...) {
                return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                         Variable::var(L"ExpressionError"),
                                         scope, all_scope, this_scope);
              }
              if (fn_native.tp == Lpp::Lpp::is_native_function) {
                Variable::var f = Variable::parse(x.args[0]);
                if (f.tp != Variable::String)
                  return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                           Variable::var(L"SyntaxError"), scope,
                                           all_scope, this_scope);
                if (f.StringValue == L"substr") {
                  if (args_temp.ArrayValue.size() < 1 ||
                      args_temp.ArrayValue.size() > 2)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  if (args_temp.ArrayValue[0].tp != Variable::Int ||
                      (args_temp.ArrayValue.size() == 2 &&
                       args_temp.ArrayValue[1].tp != Variable::Int))
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    if (args_temp.ArrayValue[0].IntValue < 0 ||
                        args_temp.ArrayValue[0].IntValue >=
                            fn_native.getConstParent().StringValue.length())
                      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                               Variable::var(L""), scope,
                                               all_scope, this_scope);
                    size_t start = (size_t)args_temp.ArrayValue[0].IntValue;
                    if (args_temp.ArrayValue.size() == 1)
                      return Lpp::Return_Value(
                          cmd, Lpp::Expression_Calc_Value,
                          Variable::var(
                              fn_native.getConstParent().StringValue.substr(
                                  start)),
                          scope, all_scope, this_scope);
                    if (args_temp.ArrayValue.size() == 2) {
                      if (args_temp.ArrayValue[1].IntValue < 0)
                        return Lpp::Return_Value(
                            cmd, Lpp::Expression_Calc_Value, Variable::var(L""),
                            scope, all_scope, this_scope);
                      size_t count = (size_t)args_temp.ArrayValue[1].IntValue;
                      return Lpp::Return_Value(
                          cmd, Lpp::Expression_Calc_Value,
                          Variable::var(
                              fn_native.getConstParent().StringValue.substr(
                                  start, count)),
                          scope, all_scope, this_scope);
                    }
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == L"join") {
                  try {
                    if (args_temp.ArrayValue.size() != 1) throw nullptr;
                    Variable::var sp = args_temp.ArrayValue[0];
                    if (sp.tp != Variable::String) throw nullptr;
                    std::wstring ret;
                    for (size_t i = 0;
                         i < fn_native.getConstParent().ArrayValue.size();
                         i++) {
                      if (i + 1 !=
                          fn_native.getConstParent().ArrayValue.size()) {
                        ret += fn_native.getConstParent()
                                   .ArrayValue[i]
                                   .toString() +
                               sp.StringValue;
                      } else
                        ret +=
                            fn_native.getConstParent().ArrayValue[i].toString();
                    }
                    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                             Variable::var(ret), scope,
                                             all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == L"pop") {
                  if (args_temp.ArrayValue.size() != 0) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  }
                  try {
                    if (fn_native.getParent().ArrayValue.empty()) throw nullptr;
                    fn_native.getParent().ArrayValue.pop_back();
                    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                             Variable::var(nullptr), scope,
                                             all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == L"push") {
                  if (args_temp.ArrayValue.size() != 1)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    Variable::var obj = args_temp.ArrayValue[0];
                    obj.isConst = false;
                    fn_native.getParent().ArrayValue.push_back(obj);
                    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                             Variable::var(nullptr), scope,
                                             all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"ExpressionError"),
                                             scope, all_scope, this_scope);
                  }
                } else if (f.StringValue == L"convert") {
                  if (args_temp.ArrayValue.size() != 1 ||
                      args_temp.ArrayValue[0].tp != Variable::String)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    return Lpp::Return_Value(
                        cmd, Lpp::Expression_Calc_Value,
                        fn_native.getConstParent().convert(Variable::getStrType(
                            args_temp.ArrayValue[0].StringValue)),
                        scope, all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == L"toString") {
                  return Lpp::Return_Value(
                      cmd, Lpp::Expression_Calc_Value,
                      Variable::var(fn_native.getConstParent().toString()),
                      scope, all_scope, this_scope);
                } else if (f.StringValue == L"insert") {
                  if (args_temp.ArrayValue.size() != 2 ||
                      args_temp.ArrayValue[0].tp != Variable::Int ||
                      args_temp.ArrayValue[0].IntValue < 0)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    if (fn_native.getParent().ArrayValue.size() <=
                        (size_t)args_temp.ArrayValue[0].IntValue)
                      fn_native.getParent().ArrayValue.resize(
                          (size_t)args_temp.ArrayValue[0].IntValue + 1);
                    // args_temp.ArrayValue[1].isConst = false;
                    fn_native.getParent().ArrayValue.insert(
                        fn_native.getParent().ArrayValue.begin() +
                            ((size_t)args_temp.ArrayValue[0].IntValue),
                        args_temp.ArrayValue[1]);
                    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                             Variable::var(nullptr), scope,
                                             all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == L"resize") {
                  if (args_temp.ArrayValue.size() != 1 ||
                      args_temp.ArrayValue[0].tp != Variable::Int)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    fn_native.getParent().ArrayValue.resize(
                        (size_t)args_temp.ArrayValue[0].IntValue);
                    for (size_t i = 0;
                         i < fn_native.getParent().ArrayValue.size(); i++)
                      fn_native.getParent().ArrayValue[i].isConst = false;
                    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                             Variable::var(nullptr), scope,
                                             all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var(L"EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else
                  return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                           Variable::var(L"EvalError"), scope,
                                           all_scope, this_scope);
              }
            }
          }
          // not native
          Variable::var temp_scope;
          Variable::var* parent;
          try {
            parent =
                &cmd.get_object(cmd.name, scope, all_scope, this_scope, 0, true)
                     .getThis();
            if (parent->tp != Variable::Object) parent = &scope;
          } catch (...) {
            parent = &scope;
          }
          temp_scope.isConst = false;
          temp_scope.tp = Variable::Object;
          temp_scope.ObjectValue[L"arguments"] = args_temp;
          temp_scope.ObjectValue[L"arguments"].isConst = true;
          for (size_t i = 0; i < func_temp.FunctionValue.value.size(); i++) {
            Lpp::Return_Value res;
            res = Lpp::Lpp(func_temp.FunctionValue.value[i], cmd.cmd)
                      .eval(temp_scope, all_scope, *parent);
            if (res.tp == Lpp::Function_Return_Value) {
              return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                       res.value, scope, all_scope, this_scope);
            } else if (res.tp == Lpp::Throw_Return_Value) {
              res.this_scope = this_scope;
              res.scope = scope;
              return res;
            }
          }
          return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                   Variable::var(nullptr), scope, all_scope,
                                   this_scope);
        }
      }
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               cmd.exp_calc(Variable::parse(cmd.toString()),
                                            scope, all_scope, this_scope),
                               scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  return temp;
}
int main(int argc, char** argv) {
  std::map<std::wstring, Lpp::Lpp::CmdType> behav = getFunc();
  /*
  behav[L"echo"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
  all_scope, this_scope);
    }
    try {x
      std::wcout << cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                                this_scope)
                       .toString()
                << std::endl;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var(L"ExpressionError"), scope,
                               all_scope, this_scope);
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };  // extend hotpatch
  */
#ifdef ENABLE_EXT
  behav[L"load"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    Variable::var load_mod;
    try {
      load_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                              this_scope);
      if (load_mod.tp != Variable::String) throw nullptr;
      Variable::Fn_temp s;
      s.value.push_back(L"return (__load__ " + load_mod.toString() +
                        L",arguments)");
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(s), scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    }
  };  // extend command:load
  behav[L"__load__"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                          Variable::var& all_scope,
                          Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    Variable::var load_mod;
    Variable::var arg;
    try {
      load_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                              this_scope);
      arg = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                         this_scope);
      if (load_mod.tp != Variable::String || arg.tp != Variable::Array)
        throw nullptr;
      std::wstring run = load_mod.StringValue + L" ";
      for (size_t i = 0; i < arg.ArrayValue.size(); i++) {
        if (arg.ArrayValue[i].tp == Variable::String)
          run += arg.ArrayValue[i].StringValue + L" ";
        else
          run += arg.ArrayValue[i].toString() + L" ";
      }
      return Lpp::Return_Value(
          cmd, Lpp::Expression_Calc_Value,
          Variable::var(
              system(Variable::WString2String(run.substr(0, run.length() - 1))
                         .c_str())),
          scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    }
  };  // extend internal command:__load__
  behav[L"import"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                        Variable::var& all_scope,
                        Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    Variable::var import_mod;
    try {
      import_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                                this_scope);
      if (import_mod.tp != Variable::String) throw nullptr;
      std::wifstream s(Variable::WString2String(import_mod.StringValue));
      if (!s.is_open()) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::parse(L"EvalError"), scope,
                                 all_scope, this_scope);
      }
      Variable::var temp_scope;
      temp_scope.isConst = false;
      temp_scope.tp = Variable::Object;
      while (!s.eof()) {
        std::wstring m, cm;
        std::getline(s, m);
        cm = m + L"\n";
        while (!s.eof()) {
          size_t a = 0, j = 0;
          for (size_t i = 0, z = 0; i < cm.length(); i++) {
            if (cm[i] == L'\\')
              z = !z;
            else if (cm[i] == L'\"' && !z) {
              if (a == 0 || a == 1) a = !a;
            } else if (cm[i] == L'\'' && !z) {
              if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
            } else
              z = 0;
            if (cm[i] == L'#' && a == 0) {
              while (i < cm.length() && cm[i] != L'\n') i++;
              i++;
            }
            if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0)
              j++;
            if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0)
              j--;
          }
          if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
            std::wstring temp;
            std::getline(s, temp);
            cm += temp + L"\n";
          } else
            break;
        }
        if (s.eof()) break;
        std::vector<std::wstring> w = Variable::code_split(cm);
        Lpp::Return_Value x;
        for (size_t i = 0; i < w.size(); i++) {
          x = Lpp::Lpp(w[i], cmd.cmd).eval(temp_scope);
          temp_scope = x.scope;
          if (x.tp == Lpp::Throw_Return_Value) {
            return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                     Variable::parse(L"EvalError"), scope,
                                     all_scope, this_scope);
          }
        }
      }
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(temp_scope), scope, all_scope,
                               this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse(L"SyntaxError"), scope,
                               all_scope, this_scope);
    }
  };  // extend command:import
#endif
  std::locale::global(std::locale(""));
  std::vector<std::wstring> arg(argc - 1);
  std::vector<std::wstring> program_arg;
  for (size_t i = 1; i < (size_t)argc; i++) {
    arg[i - 1] = Variable::String2WString(argv[i]);
  }
  if (!arg.empty())
    program_arg = std::vector<std::wstring>(arg.begin(), arg.end());
  if (arg.size() == 0 || arg[0] == L"-h" || arg[0] == L"--help") {
    std::wcout << L"lpp (" << VERSION_INFO << L")" << std::endl;
    std::wcout << L"Copyright(c) nu11ptr team 2021." << std::endl;
    std::wcout << L"This program was under the MIT license." << std::endl;
    std::wcout << L"usage:lpp [option...] (filename) (arguments...)"
               << std::endl;
    std::wcout << L"  -i, --interactive     open a interactive shell."
               << std::endl;
    std::wcout << L"  -h, --help            display this help." << std::endl;
    std::wcout << L"  filename              run script." << std::endl;
    std::wcout << L"  -v,--version          show the build version of this "
                  L"interpreter."
               << std::endl;
    std::wcout << L"  -                     run script from stdin."
               << std::endl;
    return 0;
  } else if (arg[0] == L"-v" || arg[0] == L"--version") {
    std::wcout << L"lpp (" << VERSION_INFO << L")" << std::endl;
    std::wcout << L"This program was under the MIT license." << std::endl;
    std::wcout << L"Copyright(c) nu11ptr team 2021." << std::endl;
    return 0;
  } else if (arg[0] == L"-") {
    program_arg = std::vector<std::wstring>(arg.begin() + 1, arg.end());
    program_arg.insert(program_arg.begin(), Variable::String2WString(argv[0]));
    size_t line = 0;
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f;
    f.isConst = false;
    f.tp = Variable::Object;
    f.ObjectValue[L"arguments"] = var_arg;
    while (!std::wcin.eof()) {
      std::wstring m, cm;
      std::getline(std::wcin, m);
      line++;
      cm = m + L"\n";
      while (!std::wcin.eof()) {
        size_t a = 0, j = 0;
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == L'\\')
            z = !z;
          else if (cm[i] == L'\"' && !z) {
            if (a == 0 || a == 1) a = !a;
          } else if (cm[i] == L'\'' && !z) {
            if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
          } else
            z = 0;
          if (cm[i] == L'#' && a == 0) {
            while (i < cm.length() && cm[i] != L'\n') i++;
            i++;
          }
          if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0) j++;
          if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::getline(std::wcin, temp);
          line++;
          cm += temp + L"\n";
        } else
          break;
      }
      if (std::wcin.eof()) break;
      std::vector<std::wstring> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::wcout
              << str_factory(L"Uncaught {0}").format({x.value.toString()})
              << std::endl;
          std::wcout << str_factory(L"at {0};#stdin:{1}")
                            .format({x.base.toString(), std::to_wstring(line)})
                     << std::endl;
          return 1;
        } else if (x.tp == Lpp::Function_Return_Value) {
          try {
            x.value = x.value.convert(Variable::Int);
            return (int)x.value.IntValue;
          } catch (...) {
            return 0;
          }
        }
      }
    }
  } else if (arg[0] == L"-i" || arg[0] == L"--interactive") {
    std::wcout << L"lpp interpreter (" << VERSION_INFO << L")" << std::endl;
    std::wcout << L"type \"copyright\" or \"help\" for more informations."
               << std::endl;
    behav[L"help"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                        Variable::var& all_scope,
                        Variable::var& this_scope) -> Lpp::Return_Value {
      std::wcout << L"How to use L++ in interactive mode" << std::endl;
      std::wcout << L"Use exit or EOF to exit." << std::endl;
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(nullptr), scope, all_scope,
                               this_scope);
    };  // extend hotpatch about help
    behav[L"copyright"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                             Variable::var& all_scope,
                             Variable::var& this_scope) -> Lpp::Return_Value {
      std::wcout << L"Copyright (c) 2021 nu11ptr team." << std::endl;
      std::wcout << L"This program was under the MIT license." << std::endl;
      std::wcout << L"Thanks for using L++." << std::endl;
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(nullptr), scope, all_scope,
                               this_scope);
    };  // extend hotpatch about copyright
    behav[L"exit"] =
        [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
           Variable::var& this_scope) -> Lpp::Return_Value { exit(0); };
    // std::wcout << strmap[L"lpp_terminal_welcome"].format() << std::endl;
    program_arg = std::vector<std::wstring>(arg.begin() + 1, arg.end());
    program_arg.insert(program_arg.begin(), Variable::String2WString(argv[0]));
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f;
    f.isConst = false;
    f.tp = Variable::Object;
    f.ObjectValue[L"arguments"] = var_arg;
    while (!std::wcin.eof()) {
      std::wstring m, cm;
      std::wcout << L"> " << std::flush;
      std::wcin.clear();
      std::getline(std::wcin, m);
      cm = m + L"\n";
      while (!std::wcin.eof()) {
        size_t a = 0, j = 0;
        std::wcin.clear();
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == L'\\')
            z = !z;
          else if (cm[i] == L'\"' && !z) {
            if (a == 0 || a == 1) a = !a;
          } else if (cm[i] == L'\'' && !z) {
            if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
          } else
            z = 0;
          if (cm[i] == L'#' && a == 0) {
            while (i < cm.length() && cm[i] != L'\n') i++;
            i++;
          }
          if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0) j++;
          if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::wcout << L"... " << std::flush;
          std::getline(std::wcin, temp);
          cm += temp + L"\n";
        } else
          break;
        a = 0;
        j = 0;
      }
      if (std::wcin.eof()) return 0;
      std::vector<std::wstring> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      bool inerr = false;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::wcout
              << str_factory(L"Uncaught {0}").format({x.value.toString()})
              << std::endl;
          inerr = true;
          // x.value = Variable::var(nullptr, false);
        }
        if (x.base.name == L"copyright" || x.base.name == L"help") inerr = true;
      }
      if (!inerr) std::wcout << x.value.toString() << std::endl;
      m = L"";
      cm = L"";
    }
  } else if (arg.size() > 0) {
    std::wifstream s(argv[1]);
    if (!s.is_open()) {
      std::wcout
          << str_factory(L"{0}: No such file or directory").format({arg[0]})
          << std::endl;
      return 1;
    }
    program_arg = std::vector<std::wstring>(arg.begin() + 1, arg.end());
    program_arg.insert(program_arg.begin(), arg[0]);
    size_t line = 0;
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f;
    f.isConst = false;
    f.tp = Variable::Object;
    f.ObjectValue[L"arguments"] = var_arg;
    while (!s.eof()) {
      std::wstring m, cm;
      std::getline(s, m);
      line++;
      cm = m + L"\n";
      while (!s.eof()) {
        size_t a = 0, j = 0;
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == L'\\')
            z = !z;
          else if (cm[i] == L'\"' && !z) {
            if (a == 0 || a == 1) a = !a;
          } else if (cm[i] == L'\'' && !z) {
            if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
          } else
            z = 0;
          if (cm[i] == L'#' && a == 0) {
            while (i < cm.length() && cm[i] != L'\n') i++;
            i++;
          }
          if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0) j++;
          if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::getline(s, temp);
          line++;
          cm += temp + L"\n";
        } else
          break;
      }
      if (s.eof()) break;
      std::vector<std::wstring> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::wcout
              << str_factory(L"Uncaught {0}").format({x.value.toString()})
              << std::endl;
          std::wcout << str_factory(L"at {0};#{1}:{2}")
                            .format({x.base.toString(), arg[0],
                                     std::to_wstring(line)})
                     << std::endl;
          return 1;
        } else if (x.tp == Lpp::Function_Return_Value) {
          try {
            x.value = x.value.convert(Variable::Int);
            return (int)x.value.IntValue;
          } catch (...) {
            return 0;
          }
        }
      }
    }
    s.close();
  }
  return 0;
}
