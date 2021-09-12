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
  std::string fmt;

 public:
  str_factory() {}
  str_factory(std::string f) {
    fmt = f;
    return;
  }
  std::string format(std::vector<std::string> value = {}) {
    bool flag = false;
    std::string fin;
    for (size_t i = 0; i < fmt.length(); i++) {
      if (fmt[i] == '\\') flag = !flag;
      if (fmt[i] == '{' && !flag) {
        size_t j = i;
        while (j < fmt.length() && fmt[j] != '}') j++;
        fin += value.at(std::stoi(fmt.substr(i + 1, j - 1)));
        i = j;
        continue;
      }
      if (!flag) fin += fmt[i];
    }
    return fin;
  }
} str_factory;
const std::map<std::string, Lpp::Lpp::CmdType> getFunc() {
  std::map<std::string, Lpp::Lpp::CmdType> temp;
  temp["return"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
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
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp["throw"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    }
    try {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope),
                               scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp["var"] = temp["const"] =
      [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
         Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    try {
      Variable::var s;
      for (size_t i = 0; i < cmd.args.size(); i++) {
        std::vector<std::string> exp = cmd.splitBy(cmd.args[i], '=');
        if (exp.size() == 1 && cmd.isIdentifier(exp[0]) &&
            scope.ObjectValue.find(exp[0]) == scope.ObjectValue.end()) {
          scope.ObjectValue[exp[0]] =
              Variable::var(nullptr, cmd.name == "const");
          continue;
        }
        if (exp.size() != 2 || !cmd.isIdentifier(exp[0]))
          throw Variable::SyntaxErr("Expression invaild");
        if (scope.ObjectValue.find(exp[0]) != scope.ObjectValue.end())
          throw nullptr;
        scope.ObjectValue[exp[0]] =
            cmd.exp_calc(Variable::parse(exp[1]), scope, all_scope, this_scope);
        scope.ObjectValue[exp[0]].isConst = (cmd.name == "const");
      }
    } catch (Variable::SyntaxErr&) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
    return Lpp::Return_Value(
        cmd, Lpp::Expression_Calc_Value,
        cmd.exp_calc(Variable::var(nullptr), scope, all_scope, this_scope),
        scope, all_scope, this_scope);
  };
  temp["delete"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    size_t count = 0;
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
  temp["if"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                  Variable::var& all_scope,
                  Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() < 2 || cmd.args.size() > 3)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
    if (Variable::var(true) == exp) {
      try {
        fn = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
        if (cmd.args[1] == "{}") fn.tp = Variable::Function;
        if (fn.tp != Variable::Function) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var("SyntaxError"), scope, all_scope,
                                 this_scope);
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
    } else if (cmd.args.size() == 3 && cmd.args[2] != "") {
      try {
        fn = cmd.exp_calc(Variable::parse(cmd.args[2]), scope, all_scope,
                          this_scope);
        if (cmd.args[2] == "{}") fn.tp = Variable::Function;
        if (fn.tp != Variable::Function) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var("SyntaxError"), scope, all_scope,
                                 this_scope);
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
  temp["switch"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
    try {
      x = Variable::parse(cmd.args[1]);
      if (x.tp != Variable::Array || x.ArrayValue.size() < 1) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    }
    for (size_t i = 0; i < x.ArrayValue.size(); i++) {
      if (i + 1 != x.ArrayValue.size()) {
        Variable::var val;
        try {
          if (x.ArrayValue[i].tp != Variable::Expression ||
              x.ArrayValue[i].ExpressionValue.size() != 3 ||
              x.ArrayValue[i].ExpressionValue[0] != "=")
            throw nullptr;
          val =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[1]),
                           scope, all_scope, this_scope);
          func =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[2]),
                           scope, all_scope, this_scope);
          if (x.ArrayValue[i].ExpressionValue[2] == "{}")
            func.tp = Variable::Function;
          if (func.tp != Variable::Function) throw nullptr;
        } catch (...) {
          return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                   Variable::var("SyntaxError"), scope,
                                   all_scope, this_scope);
        }
        if (exp == val) {
          Variable::var temp_scope = scope;
          Lpp::Return_Value res;
          for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
            if (Lpp::Lpp_base(func.FunctionValue.value[i]).name == "break") {
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
              x.ArrayValue[i].ExpressionValue[0] != "=" ||
              x.ArrayValue[i].ExpressionValue[1] != "default")
            throw nullptr;
          func =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[2]),
                           scope, all_scope, this_scope);
          if (x.ArrayValue[i].ExpressionValue[2] == "{}")
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
          if (Lpp::Lpp_base(func.FunctionValue.value[i]).name == "break") {
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
  temp["while"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var exp;
    Variable::var func;
    try {
      exp = Variable::parse(cmd.args[0]);
      func = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
      if (cmd.args[1] == "{}") func.tp = Variable::Function;
      if (func.tp != Variable::Function) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
    while (true) {
      Variable::var exp_res;
      try {
        exp_res = cmd.exp_calc(exp, scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var("ExpressionError"), scope,
                                 all_scope, this_scope);
      }
      if (Variable::var(true) == exp_res) {
        Variable::var temp_scope = scope;
        Lpp::Return_Value res;
        bool break_flag = 0;
        for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
          Lpp::Lpp l = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd);
          if (l.name == "continue") {
            break;
          } else if (l.name == "break") {
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
  temp["for"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                   Variable::var& all_scope,
                   Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2 || cmd.args[0][0] != '(' ||
        cmd.args[0][cmd.args[0].length() - 1] != ')')
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    std::string start;
    Variable::var exp;
    std::string routine;
    Variable::var func;
    Variable::var temp_scope;
    Lpp::Return_Value res;
    Variable::var exp_res;
    bool break_flag = false;
    try {
      std::vector<std::string> temp =
          cmd.splitBy(cmd.args[0].substr(1, cmd.args[0].length() - 2), ';');
      func = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                          this_scope);
      if (temp.size() == 2) temp.push_back("");
      if (temp.size() != 3) throw nullptr;
      start = temp[0];
      exp = Variable::parse(temp[1] == "" ? "true" : temp[1]);
      routine = temp[2];
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
                                 Variable::var("ExpressionError"), scope,
                                 all_scope, this_scope);
      }
      if (Variable::var(true) == exp_res) {
        for (size_t i = 0; i < func.FunctionValue.value.size(); i++) {
          Lpp::Lpp l = Lpp::Lpp(func.FunctionValue.value[i], cmd.cmd);
          if (l.name == "continue") {
            break;
          } else if (l.name == "break") {
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
  temp["new"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                   Variable::var& all_scope,
                   Variable::var& this_scope) -> Lpp::Return_Value {
    Variable::var func;
    try {
      if (cmd.args.size() < 1 || cmd.args.size() > 2) throw nullptr;
      func = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                          this_scope);
      if (cmd.args[0] == "{}") func.tp = Variable::Function;
      if (func.tp != Variable::Function) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
        temp_scope.ObjectValue["arguments"] = cmd.exp_calc(
            Variable::parse(cmd.args[1]), scope, all_scope, this_scope);
        if (temp_scope.ObjectValue["arguments"].tp != Variable::Array)
          throw nullptr;
        temp_scope.ObjectValue["arguments"].isConst = true;
      } catch (...) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::var("SyntaxError"), scope, all_scope,
                                 this_scope);
      }
    } else
      temp_scope.ObjectValue["arguments"] =
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
  temp["try"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                   Variable::var& all_scope,
                   Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var hook;
    Variable::var fn;
    Variable::var temp_scope = scope;
    Lpp::Return_Value res;
    try {
      const std::vector<std::string> a = cmd.splitBy(cmd.args[1], '=');
      if (a.size() != 3 || a[1] != "catch") throw nullptr;
      hook = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                          this_scope);
      if (cmd.args[0] == "{}") hook.tp = Variable::Function;
      fn = cmd.exp_calc(Variable::parse(a[2]), scope, all_scope, this_scope);
      if (a[2] == "{}") hook.tp = Variable::Function;
      if (hook.tp != Variable::Function || fn.tp != Variable::Function)
        throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    }
    for (size_t i = 0; i < hook.FunctionValue.value.size(); i++) {
      res = Lpp::Lpp(hook.FunctionValue.value[i], cmd.cmd)
                .eval(temp_scope, all_scope, this_scope);
      if (res.tp == Lpp::Throw_Return_Value) {
        temp_scope.ObjectValue["err"] = res.value;
        temp_scope.ObjectValue["err"].isConst = true;
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
  temp["typeof"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
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
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  temp["eval"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var op = cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                    all_scope, this_scope);
    if (op.tp != Variable::String)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("SyntaxError"), scope, all_scope,
                               this_scope);
    return Lpp::Lpp(op.StringValue, cmd.cmd).eval(scope, all_scope, this_scope);
  };
  temp["break"] = temp["continue"] = temp["default"] = temp["catch"] =
      temp["__native__"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                              Variable::var& all_scope,
                              Variable::var& this_scope) -> Lpp::Return_Value {
    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                             Variable::var("SyntaxError"), scope, all_scope,
                             this_scope);
  };
  temp[""] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                Variable::var& all_scope,
                Variable::var& this_scope) -> Lpp::Return_Value {
    try {
      if (cmd.isStatement(cmd.toString(), scope, all_scope, this_scope)) {
        Variable::var func_temp = cmd.exp_calc(Variable::parse(cmd.name), scope,
                                               all_scope, this_scope);
        if (func_temp.tp != Variable::Function && cmd.args.size() != 0) {
          return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                   Variable::var("SyntaxError"), scope,
                                   all_scope, this_scope);
        } else if (func_temp.tp == Variable::Function && cmd.args.size() == 1) {
          Variable::var args_temp = cmd.exp_calc(Variable::parse(cmd.args[0]),
                                                 scope, all_scope, this_scope);
          if (args_temp.tp != Variable::Array) {
            return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                     Variable::var("SyntaxError"), scope,
                                     all_scope, this_scope);
          }
          // test native
          if (func_temp.FunctionValue.value.size() == 1) {
            Lpp::Lpp_base x = Lpp::Lpp_base(func_temp.FunctionValue.value[0]);
            if (x.name == "__native__" && x.args.size() == 1) {
              // native function
              Lpp::Lpp::Return_Object fn_native;
              try {
                fn_native = cmd.get_object(cmd.name, scope, all_scope,
                                           this_scope, 0, true);
              } catch (...) {
                return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                         Variable::var("ExpressionError"),
                                         scope, all_scope, this_scope);
              }
              if (fn_native.tp == Lpp::Lpp::is_native_function) {
                Variable::var f = Variable::parse(x.args[0]);
                if (f.tp != Variable::String)
                  return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                           Variable::var("SyntaxError"), scope,
                                           all_scope, this_scope);
                if (f.StringValue == "substr") {
                  if (args_temp.ArrayValue.size() < 1 ||
                      args_temp.ArrayValue.size() > 2)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
                                             scope, all_scope, this_scope);
                  if (args_temp.ArrayValue[0].tp != Variable::Int ||
                      (args_temp.ArrayValue.size() == 2 &&
                       args_temp.ArrayValue[1].tp != Variable::Int))
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    if (args_temp.ArrayValue[0].IntValue < 0 ||
                        args_temp.ArrayValue[0].IntValue >=
                            fn_native.getConstParent().StringValue.length())
                      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                                               Variable::var(""), scope,
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
                            cmd, Lpp::Expression_Calc_Value, Variable::var(""),
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
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == "join") {
                  try {
                    if (args_temp.ArrayValue.size() != 1) throw nullptr;
                    Variable::var sp = args_temp.ArrayValue[0];
                    if (sp.tp != Variable::String) throw nullptr;
                    std::string ret;
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
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == "pop") {
                  if (args_temp.ArrayValue.size() != 0) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
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
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == "push") {
                  if (args_temp.ArrayValue.size() != 1)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
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
                                             Variable::var("ExpressionError"),
                                             scope, all_scope, this_scope);
                  }
                } else if (f.StringValue == "convert") {
                  if (args_temp.ArrayValue.size() != 1 ||
                      args_temp.ArrayValue[0].tp != Variable::String)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
                                             scope, all_scope, this_scope);
                  try {
                    return Lpp::Return_Value(
                        cmd, Lpp::Expression_Calc_Value,
                        fn_native.getConstParent().convert(Variable::getStrType(
                            args_temp.ArrayValue[0].StringValue)),
                        scope, all_scope, this_scope);
                  } catch (...) {
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == "toString") {
                  return Lpp::Return_Value(
                      cmd, Lpp::Expression_Calc_Value,
                      Variable::var(fn_native.getConstParent().toString()),
                      scope, all_scope, this_scope);
                } else if (f.StringValue == "insert") {
                  if (args_temp.ArrayValue.size() != 2 ||
                      args_temp.ArrayValue[0].tp != Variable::Int ||
                      args_temp.ArrayValue[0].IntValue < 0)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
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
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else if (f.StringValue == "resize") {
                  if (args_temp.ArrayValue.size() != 1 ||
                      args_temp.ArrayValue[0].tp != Variable::Int)
                    return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                             Variable::var("SyntaxError"),
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
                                             Variable::var("EvalError"), scope,
                                             all_scope, this_scope);
                  }
                } else
                  return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                           Variable::var("EvalError"), scope,
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
          temp_scope.ObjectValue["arguments"] = args_temp;
          temp_scope.ObjectValue["arguments"].isConst = true;
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
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
  };
  return temp;
}
int main(int argc, char** argv) {
  std::map<std::string, Lpp::Lpp::CmdType> behav = getFunc();
  /*
  behav["echo"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    }
    try {x
      std::cout << cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                                this_scope)
                       .toString()
                << std::endl;
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::var("ExpressionError"), scope,
                               all_scope, this_scope);
    }
    return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                             Variable::var(nullptr), scope, all_scope,
                             this_scope);
  };  // extend hotpatch
  */
#ifdef ENABLE_EXT
  behav["load"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var load_mod;
    try {
      load_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                              this_scope);
      if (load_mod.tp != Variable::String) throw nullptr;
      Variable::Fn_temp s;
      s.value.push_back("return (__load__ " + load_mod.toString() +
                        ",arguments)");
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(s), scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    }
  };  // extend command:load
  behav["__load__"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                         Variable::var& all_scope,
                         Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var load_mod;
    Variable::var arg;
    try {
      load_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                              this_scope);
      arg = cmd.exp_calc(Variable::parse(cmd.args[1]), scope, all_scope,
                         this_scope);
      if (load_mod.tp != Variable::String || arg.tp != Variable::Array)
        throw nullptr;
      std::string run = load_mod.toString() + " ";
      for (size_t i = 0; i < arg.ArrayValue.size(); i++) {
        run += arg.ArrayValue[i].toString() + " ";
      }
      return Lpp::Return_Value(
          cmd, Lpp::Expression_Calc_Value,
          Variable::var(system(run.substr(0, run.length() - 1).c_str())), scope,
          all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    }
  };  // extend internal command:__load__
  behav["import"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    Variable::var import_mod;
    try {
      import_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                                this_scope);
      if (import_mod.tp != Variable::String) throw nullptr;
      std::ifstream s(import_mod.StringValue);
      if (!s.is_open()) {
        return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                 Variable::parse("EvalError"), scope, all_scope,
                                 this_scope);
      }
      Variable::var temp_scope;
      temp_scope.isConst = false;
      temp_scope.tp = Variable::Object;
      while (!s.eof()) {
        std::string m, cm;
        std::getline(s, m);
        cm = m + "\n";
        while (!s.eof()) {
          size_t a = 0, j = 0;
          for (size_t i = 0, z = 0; i < cm.length(); i++) {
            if (cm[i] == '\\')
              z = !z;
            else if (cm[i] == '\"' && !z) {
              if (a == 0 || a == 1) a = !a;
            } else if (cm[i] == '\'' && !z) {
              if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
            } else
              z = 0;
            if (cm[i] == '#' && a == 0) {
              while (i < cm.length() && cm[i] != '\n') i++;
              i++;
            }
            if (cm[i] == '\n' &&
                (i <= 0 || (cm[i - 1] != '[' && cm[i - 1] != '{' &&
                            cm[i - 1] != '(' && cm[i - 1] != ';')))
              cm[i] = ';';
            if ((cm[i] == '[' || cm[i] == '{' || cm[i] == '(') && a == 0) j++;
            if ((cm[i] == ']' || cm[i] == '}' || cm[i] == ')') && a == 0) j--;
          }
          if (a != 0 || j != 0 || cm[cm.length() - 1] == ',') {
            std::string temp;
            std::getline(s, temp);
            cm += temp + "\n";
          } else
            break;
        }
        if (s.eof()) break;
        std::vector<std::string> w = Variable::code_split(cm);
        Lpp::Return_Value x;
        for (size_t i = 0; i < w.size(); i++) {
          x = Lpp::Lpp(w[i], cmd.cmd).eval(temp_scope);
          temp_scope = x.scope;
          if (x.tp == Lpp::Throw_Return_Value) {
            return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                                     Variable::parse("EvalError"), scope,
                                     all_scope, this_scope);
          }
        }
      }
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(temp_scope), scope, all_scope,
                               this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd, Lpp::Throw_Return_Value,
                               Variable::parse("SyntaxError"), scope, all_scope,
                               this_scope);
    }
  };  // extend command:import
#endif
  std::vector<std::string> arg(argc - 1);
  std::vector<std::string> program_arg;
  for (size_t i = 1; i < (size_t)argc; i++) arg[i - 1] = std::string(argv[i]);
  program_arg = std::vector<std::string>(arg.begin(), arg.end());
  if (arg.size() == 0 || arg[0] == "-h" || arg[0] == "--help") {
    std::cout << "lpp (" << VERSION_INFO << ")" << std::endl;
    std::cout << "Copyright(c) nu11ptr team 2021." << std::endl;
    std::cout << "This program was under the MIT license." << std::endl;
    std::cout << "usage:lpp [option...] (filename) (arguments...)" << std::endl;
    std::cout << "  -i, --interactive     open a interactive shell."
              << std::endl;
    std::cout << "  -h, --help            display this help." << std::endl;
    std::cout << "  filename              run script." << std::endl;
    std::cout
        << "  -v,--version          show the build version of this interpreter."
        << std::endl;
    std::cout << "  -                     run script from stdin." << std::endl;
    return 0;
  } else if (arg[0] == "-v" || arg[0] == "--version") {
    std::cout << "lpp (" << VERSION_INFO << ")" << std::endl;
    std::cout << "This program was under the MIT license." << std::endl;
    std::cout << "Copyright(c) nu11ptr team 2021." << std::endl;
    return 0;
  } else if (arg[0] == "-") {
    program_arg = std::vector<std::string>(arg.begin() + 1, arg.end());
    program_arg.insert(program_arg.begin(), std::string(argv[0]));
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
    f.ObjectValue["arguments"] = var_arg;
    while (!std::cin.eof()) {
      std::string m, cm;
      std::getline(std::cin, m);
      line++;
      cm = m + "\n";
      while (!std::cin.eof()) {
        size_t a = 0, j = 0;
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == '\\')
            z = !z;
          else if (cm[i] == '\"' && !z) {
            if (a == 0)
              a = 1;
            else if (a == 1)
              a = 0;
          } else if (cm[i] == '\'' && !z) {
            if (a == 2)
              a = 1;
            else if (a == 2)
              a = 0;
          } else
            z = 0;
          if (cm[i] == '#' && a == 0) {
            while (i < cm.length() && cm[i] != '\n') i++;
            i++;
          }
          if (cm[i] == '\n' &&
              (i <= 0 || (cm[i - 1] != '[' && cm[i - 1] != '{' &&
                          cm[i - 1] != '(' && cm[i - 1] != ';')))
            cm[i] = ';';
          if ((cm[i] == '[' || cm[i] == '{' || cm[i] == '(') && a == 0) j++;
          if ((cm[i] == ']' || cm[i] == '}' || cm[i] == ')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == ',') {
          std::string temp;
          std::getline(std::cin, temp);
          line++;
          cm += temp + "\n";
        } else
          break;
      }
      if (std::cin.eof()) break;
      std::vector<std::string> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::cout << str_factory("Uncaught {0}").format({x.value.toString()})
                    << std::endl;
          std::cout << str_factory("at {0};#stdin:{1}")
                           .format({x.base.toString(), std::to_string(line)})
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
  } else if (arg[0] == "-i" || arg[0] == "--interactive") {
    std::cout << "lpp interpreter (" << VERSION_INFO << ")" << std::endl;
    std::cout << "type \"copyright\" or \"help\" for more informations."
              << std::endl;
    behav["help"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> Lpp::Return_Value {
      std::cout << "How to use L++ in interactive mode" << std::endl;
      std::cout << "Use exit or EOF to exit." << std::endl;
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(nullptr), scope, all_scope,
                               this_scope);
    };  // extend hotpatch about help
    behav["copyright"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                            Variable::var& all_scope,
                            Variable::var& this_scope) -> Lpp::Return_Value {
      std::cout << "Copyright (c) 2021 nu11ptr team." << std::endl;
      std::cout << "This program was under the MIT license." << std::endl;
      std::cout << "Thanks for using L++." << std::endl;
      return Lpp::Return_Value(cmd, Lpp::Expression_Calc_Value,
                               Variable::var(nullptr), scope, all_scope,
                               this_scope);
    };  // extend hotpatch about copyright
    behav["exit"] =
        [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
           Variable::var& this_scope) -> Lpp::Return_Value { exit(0); };
    // std::cout << strmap["lpp_terminal_welcome"].format() << std::endl;
    program_arg = std::vector<std::string>(arg.begin() + 1, arg.end());
    program_arg.insert(program_arg.begin(), std::string(argv[0]));
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f;
    f.isConst = false;
    f.tp = Variable::Object;
    f.ObjectValue["arguments"] = var_arg;
    while (!std::cin.eof()) {
      std::string m, cm;
      std::cout << "> " << std::flush;
      std::cin.clear();
      std::getline(std::cin, m);
      cm = m + "\n";
      while (!std::cin.eof()) {
        size_t a = 0, j = 0;
        std::cin.clear();
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == '\\')
            z = !z;
          else if (cm[i] == '\"' && !z) {
            if (a == 0 || a == 1) a = !a;
          } else if (cm[i] == '\'' && !z) {
            if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
          } else
            z = 0;
          if (cm[i] == '#' && a == 0) {
            while (i < cm.length() && cm[i] != '\n') i++;
            i++;
          }
          if (cm[i] == '\n' && (i > 0 && cm[i - 1] != '[' && cm[i - 1] != '{' &&
                                cm[i - 1] != '(' && cm[i - 1] != ';'))
            cm[i] = ';';
          if ((cm[i] == '[' || cm[i] == '{' || cm[i] == '(') && a == 0) j++;
          if ((cm[i] == ']' || cm[i] == '}' || cm[i] == ')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == ',') {
          std::string temp;
          std::cout << "... " << std::flush;
          std::getline(std::cin, temp);
          cm += temp + "\n";
        } else
          break;
        a = 0;
        j = 0;
      }
      if (std::cin.eof()) return 0;
      std::vector<std::string> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      bool inerr = false;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::cout << str_factory("Uncaught {0}").format({x.value.toString()})
                    << std::endl;
          inerr = true;
          // x.value = Variable::var(nullptr, false);
        }
        if (x.base.name == "copyright" || x.base.name == "help") inerr = true;
      }
      if (!inerr) std::cout << x.value.toString() << std::endl;
      m = "";
      cm = "";
    }
  } else if (arg.size() > 0) {
    std::ifstream s(arg[0]);
    if (!s.is_open()) {
      std::cout
          << str_factory("{0}: No such file or directory").format({arg[0]})
          << std::endl;
      return 1;
    }
    program_arg = std::vector<std::string>(arg.begin() + 1, arg.end());
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
    f.ObjectValue["arguments"] = var_arg;
    while (!s.eof()) {
      std::string m, cm;
      std::getline(s, m);
      line++;
      cm = m + "\n";
      while (!s.eof()) {
        size_t a = 0, j = 0;
        for (size_t i = 0, z = 0; i < cm.length(); i++) {
          if (cm[i] == '\\')
            z = !z;
          else if (cm[i] == '\"' && !z) {
            if (a == 0)
              a = 1;
            else if (a == 1)
              a = 0;
          } else if (cm[i] == '\'' && !z) {
            if (a == 2)
              a = 1;
            else if (a == 2)
              a = 0;
          } else
            z = 0;
          if (cm[i] == '#' && a == 0) {
            while (i < cm.length() && cm[i] != '\n') i++;
            i++;
          }
          if (cm[i] == '\n' &&
              (i <= 0 || (cm[i - 1] != '[' && cm[i - 1] != '{' &&
                          cm[i - 1] != '(' && cm[i - 1] != ';')))
            cm[i] = ';';
          if ((cm[i] == '[' || cm[i] == '{' || cm[i] == '(') && a == 0) j++;
          if ((cm[i] == ']' || cm[i] == '}' || cm[i] == ')') && a == 0) j--;
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == ',') {
          std::string temp;
          std::getline(s, temp);
          line++;
          cm += temp + "\n";
        } else
          break;
      }
      if (s.eof()) break;
      std::vector<std::string> w = Variable::code_split(cm);
      Lpp::Return_Value x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Return_Value) {
          std::cout << str_factory("Uncaught {0}").format({x.value.toString()})
                    << std::endl;
          std::cout << str_factory("at {0};#{1}:{2}")
                           .format({x.base.toString(), arg[0],
                                    std::to_string(line)})
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
