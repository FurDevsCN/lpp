/*
L++ programming language was under the MIT license.
Copyright(c) 2021 nu11ptr team.
*/
/*
compile flags:
ENABLE_EXT : enable extend commands.
*/
// your compile flag here.
#include <fstream>
#include <iostream>

#include "./include/parse.h"
#define VERSION_INFO L"1.6.3-20211029_preview-1"
// typedef class str_factory {
//   std::wstring fmt;

//  public:
//   str_factory() {}
//   str_factory(const std::wstring& f) {
//     fmt = f;
//     return;
//   }
//   const std::wstring format(const std::vector<std::wstring>& value = {}) {
//     bool flag = false;
//     std::wstring fin;
//     for (size_t i = 0; i < fmt.length(); i++) {
//       if (fmt[i] == L'\\') flag = !flag;
//       if (fmt[i] == L'{' && !flag) {
//         size_t j = i;
//         while (j < fmt.length() && fmt[j] != L'}') j++;
//         fin += value.at(std::stoi(fmt.substr(i + 1, j - 1)));
//         i = j;
//         continue;
//       }
//       if (!flag) fin += fmt[i];
//     }
//     return fin;
//   }
// } str_factory;
const std::map<std::wstring, Lpp::Lpp::CmdType> getFunc() {
  std::map<std::wstring, Lpp::Lpp::CmdType> temp;
  temp[L"return"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() == 0) return Lpp::Return_Value(Lpp::Ret_Value, nullptr);
    try {
      return Lpp::Return_Value(Lpp::Ret_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope));
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
  };
  temp[L"char"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var ch;
    try {
      ch = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                        this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    if (ch.tp == Variable::String && ch.StringValue.length() == 1) {
      return Lpp::Return_Value(Lpp::Calc_Value, (int)ch.StringValue[0]);
    } else if (ch.tp == Variable::Int && ch.IntValue >= 0) {
      return Lpp::Return_Value(Lpp::Calc_Value,
                               std::wstring(1, (int)ch.IntValue));
    } else {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value, L"SyntaxError");
    }
  };
  temp[L"instance"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                         Variable::var& all_scope,
                         Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var obj;
    try {
      obj = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                         this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    if (obj.tp != Variable::Object) {
      return Lpp::Return_Value(Lpp::Calc_Value, Variable::instanceOf(obj.tp));
    } else {
      return Lpp::Return_Value(Lpp::Calc_Value,
                               obj.ObjectValue[L"__constructor__"]);
    }
  };
  temp[L"function"] = temp[L"int"] = temp[L"bool"] = temp[L"string"] =
      temp[L"array"] = temp[L"object"] =
          [](const Lpp::Lpp& cmd, Variable::var& scope,
             Variable::var& all_scope,
             Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() == 0) {
      return Lpp::Return_Value(
          Lpp::Calc_Value,
          Variable::instanceOf(Variable::getStrType(cmd.name)));
    }
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    try {
      return Lpp::Return_Value(Lpp::Calc_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope)
                                   .convert(Variable::getStrType(cmd.name)));
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
  };
  temp[L"throw"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    }
    try {
      return Lpp::Return_Value(Lpp::Throw_Value,
                               cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope));
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
  };
  temp[L"exist"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    }
    try {
      Lpp::Lpp::Return_Object a = cmd.get_object(cmd.args[0], scope, all_scope,
                                                 this_scope, true, false);
    } catch (Lpp::Lpp::Object_Errors i) {
      return Lpp::Return_Value(Lpp::Calc_Value, false);

    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    return Lpp::Return_Value(Lpp::Calc_Value, true);
  };
  temp[L"var"] = temp[L"const"] =
      [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
         Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    size_t i;
    try {
      Variable::var s;
      for (i = 0; i < cmd.args.size(); i++) {
        std::vector<std::wstring> exp =
            Variable::splitBy(Variable::clearnull(cmd.args[i]), L'=');
        if (exp.size() == 1 && cmd.isIdentifier(exp[0]) &&
            scope.ObjectValue.find(exp[0]) == scope.ObjectValue.cend()) {
          scope.ObjectValue[exp[0]] =
              Variable::var(nullptr, cmd.name == L"const");
          continue;
        }
        if (exp.size() != 2 || !cmd.isIdentifier(exp[0]))
          throw Variable::ExprErr(L"Expression invaild");
        if (scope.ObjectValue.find(exp[0]) != scope.ObjectValue.cend())
          throw nullptr;
        scope.ObjectValue[exp[0]] =
            cmd.exp_calc(Variable::parse(exp[1]), scope, all_scope, this_scope);
        scope.ObjectValue[exp[0]].isConst = (cmd.name == L"const");
      }
    } catch (Variable::SyntaxErr&) {
      return Lpp::Return_Value(cmd.args[i], Lpp::Throw_Value, L"SyntaxError");
    } catch (...) {
      return Lpp::Return_Value(cmd.args[i], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"delete"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
    size_t count = 0;
    if (cmd.args.size() < 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    for (size_t i = 0; i < cmd.args.size(); i++) {
      Lpp::Lpp::Return_Object y;
      try {
        y = cmd.get_object(cmd.args[i], scope, all_scope, this_scope, true,
                           true);
        if (y.tp == Lpp::Lpp::is_const_value || y.getValue().isConst)
          throw Lpp::Lpp::member_cant_visit;
        y.getValue().remove();
      } catch (const Lpp::Lpp::Object_Errors&) {
        continue;
      } catch (...) {
        return Lpp::Return_Value(cmd.args[i], Lpp::Throw_Value,
                                 L"ExpressionError");
      }
      y.getParent().update();
      count++;
    }
    return Lpp::Return_Value(Lpp::Calc_Value, (int)count);
  };
  temp[L"if"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                   Variable::var& all_scope,
                   Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() < 2 || cmd.args.size() > 3)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var exp;
    Variable::var fn;
    Variable::var temp_scope = scope;
    Lpp::Return_Value res;
    try {
      exp = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                         this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    if (Variable::var(true).opcall(L"==", exp).BooleanValue) {
      try {
        fn = Variable::parse(cmd.args[1]);
        if (cmd.args[1] == L"{}") fn.tp = Variable::StmtBlock;
        if (fn.tp != Variable::StmtBlock) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value, L"SyntaxError");
      }
      try {
        cmd.RunStmt(fn, scope, all_scope, this_scope, temp_scope, nullptr,
                    false, false);
      } catch (const Lpp::Exec_Info& ret) {
        return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
      } catch (...) {
        return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
      }
    } else if (cmd.args.size() == 3 && cmd.args[2] != L"") {
      try {
        fn = Variable::parse(cmd.args[2]);
        if (cmd.args[2] == L"{}") fn.tp = Variable::StmtBlock;
        if (fn.tp != Variable::StmtBlock) throw nullptr;
      } catch (...) {
        return Lpp::Return_Value(cmd.args[2], Lpp::Throw_Value, L"SyntaxError");
      }
      try {
        cmd.RunStmt(fn, scope, all_scope, this_scope, temp_scope, nullptr,
                    false, false);
      } catch (const Lpp::Exec_Info& ret) {
        return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
      } catch (...) {
        return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
      }
    }
    return Lpp::Return_Value(Lpp::Calc_Value,
                             Variable::var(true).opcall(L"==", exp));
  };
  temp[L"switch"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var x;
    Variable::var exp;
    Variable::var func;
    try {
      exp = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                         this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    try {
      x = Variable::parse(cmd.args[1]);
      if (x.tp != Variable::Array) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value, L"SyntaxError");
    }
    for (size_t i = 0; i < x.ArrayValue.size(); i++) {
      if (i + 1 != x.ArrayValue.size() ||
          x.ArrayValue[i].ExpressionValue[1] != L"default") {
        Variable::var val;
        try {
          if (x.ArrayValue[i].tp != Variable::Expression ||
              x.ArrayValue[i].ExpressionValue.size() != 3 ||
              x.ArrayValue[i].ExpressionValue[0] != L"=")
            throw nullptr;
          val =
              cmd.exp_calc(Variable::parse(x.ArrayValue[i].ExpressionValue[1]),
                           scope, all_scope, this_scope);
          func = Variable::parse(x.ArrayValue[i].ExpressionValue[2]);
          if (x.ArrayValue[i].ExpressionValue[2] == L"{}")
            func.tp = Variable::StmtBlock;
          if (func.tp != Variable::StmtBlock) throw nullptr;
        } catch (const std::nullptr_t&) {
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"SyntaxError");
        } catch (...) {
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"ExpressionError");
        }
        if (exp.opcall(L"==", val).BooleanValue) {
          Variable::var temp_scope = scope;
          try {
            cmd.RunStmt(func, scope, all_scope, this_scope, temp_scope, nullptr,
                        true, false);
          } catch (const Lpp::Exec_Info& ret) {
            return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
          } catch (const size_t& stopmode) {
            if (stopmode == 2) {
              break;
            }
          }
        }
      } else {
        try {
          if (x.ArrayValue[i].tp != Variable::Expression ||
              x.ArrayValue[i].ExpressionValue.size() != 3 ||
              x.ArrayValue[i].ExpressionValue[0] != L"=" ||
              x.ArrayValue[i].ExpressionValue[1] != L"default")
            throw nullptr;
          func = Variable::parse(x.ArrayValue[i].ExpressionValue[2]);
          if (x.ArrayValue[i].ExpressionValue[2] == L"{}")
            func.tp = Variable::StmtBlock;
          if (func.tp != Variable::StmtBlock) throw nullptr;
        } catch (const std::nullptr_t&) {
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"SyntaxError");
        } catch (...) {
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"ExpressionError");
        }
        Variable::var temp_scope = scope;
        try {
          cmd.RunStmt(func, scope, all_scope, this_scope, temp_scope, nullptr,
                      true, false);
        } catch (const Lpp::Exec_Info& ret) {
          return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
        } catch (size_t stopmode) {
          if (stopmode == 2) {
            break;
          }
        }
      }
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"while"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                      Variable::var& all_scope,
                      Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var exp;
    Variable::var func;
    Variable::var temp_scope = scope;
    Variable::var exp_res;
    try {
      exp = Variable::parse(cmd.args[0]);
      func = Variable::parse(cmd.args[1]);
      if (cmd.args[1] == L"{}") func.tp = Variable::StmtBlock;
      if (func.tp != Variable::StmtBlock) throw nullptr;
    } catch (const std::nullptr_t&) {
      return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value, L"SyntaxError");
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    while (true) {
      try {
        exp_res = cmd.exp_calc(exp, scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                 L"ExpressionError");
      }
      if (Variable::var(true).opcall(L"==", exp_res).BooleanValue) {
        try {
          cmd.RunStmt(func, scope, all_scope, this_scope, temp_scope, nullptr,
                      true, true);
        } catch (const Lpp::Exec_Info& ret) {
          return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
        } catch (size_t stopmode) {
          if (stopmode == 1) {
            continue;
          } else if (stopmode == 2) {
            break;
          }
        }
      } else
        break;
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"for"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 2 || cmd.args[0][0] != L'(' ||
        cmd.args[0][cmd.args[0].length() - 1] != L')')
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    std::wstring start;
    Variable::var exp;
    std::wstring routine;
    Variable::var func;
    Variable::var temp_scope;
    Lpp::Exec_Info res;
    Variable::var exp_res;
    Variable::var exclude;
    try {
      std::vector<std::wstring> temp = Variable::splitBy(
          Variable::clearnull(cmd.args[0].substr(1, cmd.args[0].length() - 2)),
          L';');
      func = Variable::parse(cmd.args[1]);
      if (cmd.args[1] == L"{}") func.tp = Variable::StmtBlock;
      if (temp.size() == 2) temp.push_back(L"");
      if (temp.size() != 3 || func.tp != Variable::StmtBlock) throw nullptr;
      start = temp[0];
      exp = Variable::parse(temp[1] == L"" ? L"true" : temp[1]);
      routine = temp[2];
    } catch (const std::nullptr_t&) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value, L"SyntaxError");
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"ExpressionError");
    }
    temp_scope = scope;
    res = Lpp::Lpp(start, cmd.cmd).eval(temp_scope, all_scope, this_scope);
    // temp_scope = res.scope;
    if (res.tp != Lpp::Calc_Value) {
      return Lpp::Return_Value(start, res.tp, res.value);
    }
    exclude = cmd.exclude_scope(scope, temp_scope);
    while (true) {
      try {
        exp_res = cmd.exp_calc(exp, temp_scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                 L"ExpressionError");
      }
      if (Variable::var(true).opcall(L"==", exp_res).BooleanValue) {
        try {
          cmd.RunStmt(func, scope, all_scope, this_scope, temp_scope, exclude,
                      true, true);
        } catch (const Lpp::Exec_Info& ret) {
          return Lpp::Return_Value(ret.cmd.toString(), ret.tp, ret.value);
        } catch (size_t stopmode) {
          if (stopmode == 1) {
            continue;
          } else if (stopmode == 2) {
            break;
          }
        }
      } else
        break;
      res = Lpp::Lpp(routine, cmd.cmd).eval(temp_scope, all_scope, this_scope);
      if (res.tp != Lpp::Calc_Value) {
        return Lpp::Return_Value(routine, res.tp, res.value);
      }
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"new"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> const Lpp::Return_Value {
    Variable::var func;
    Variable::var func_arg;
    if (cmd.args.size() < 1 || cmd.args.size() > 2)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    try {
      func = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                          this_scope);
      if (func.tp != Variable::Function) throw nullptr;
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value, L"SyntaxError");
    }
    Variable::var temp_scope =
        Variable::var(std::map<std::wstring, Variable::var>());
    Variable::var temp_this_scope =
        Variable::var(std::map<std::wstring, Variable::var>());
    temp_this_scope.ObjectValue[L"__constructor__"] = func;
    if (cmd.args.size() >= 2) {
      try {
        const Variable::var& temp = Variable::parse(cmd.args[1]);
        if (temp.tp != Variable::Array)
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"SyntaxError");
        func_arg = cmd.exp_calc(temp, scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                 L"ExpressionError");
      }
    } else
      func_arg = std::vector<Variable::var>();
    try {
      cmd.RunFunc(func, temp_scope, all_scope, temp_this_scope, func_arg, true);
      return Lpp::Return_Value(Lpp::Calc_Value, temp_this_scope);
    } catch (Variable::SyntaxErr&) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    } catch (Variable::ExprErr&) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"ExpressionError");
    } catch (const Lpp::Exec_Info& a) {
      return Lpp::Return_Value(a.cmd.toString(), a.tp, a.value);
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
    }
  };
  temp[L"try"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                    Variable::var& all_scope,
                    Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var hook;
    Variable::var fn;
    try {
      const std::vector<std::wstring> a =
          Variable::splitBy(Variable::clearnull(cmd.args[1]), L'=');
      if (a.size() != 2 || a[0] != L"catch") throw nullptr;
      hook = Variable::parse(cmd.args[0]);
      if (cmd.args[0] == L"{}") hook.tp = Variable::StmtBlock;
      fn = Variable::parse(a[1]);
      if (a[1] == L"{}") hook.tp = Variable::StmtBlock;
      if (hook.tp != Variable::StmtBlock) throw 0;
      if (fn.tp != Variable::StmtBlock) throw 1;
    } catch (const size_t i) {
      return Lpp::Return_Value(cmd.args[i], Lpp::Throw_Value, L"SyntaxError");
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    }
    try {
      Variable::var temp = scope;
      cmd.RunStmt(hook, scope, all_scope, this_scope, temp, nullptr, false,
                  false);
    } catch (const Lpp::Exec_Info& i) {
      if (i.tp == Lpp::Throw_Value) {
        try {
          Variable::var temp_scope = scope;
          std::map<std::wstring, Lpp::Lpp::CmdType> s = cmd.cmd;
          s[L"what"] =
              [i](const Lpp::Lpp& cmd, Variable::var& scope,
                  Variable::var& all_scope,
                  Variable::var& this_scope) -> const Lpp::Return_Value {
            if (!cmd.args.empty())
              return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
            return Lpp::Return_Value(Lpp::Calc_Value, i.value);
          };
          Lpp::Lpp(L"", s).RunStmt(fn, scope, all_scope, this_scope, temp_scope,
                                   nullptr, false, false);
        } catch (const Lpp::Exec_Info& i) {
          return Lpp::Return_Value(i.cmd.toString(), i.tp, i.value);
        }
      } else
        return Lpp::Return_Value(i.cmd.toString(), i.tp, i.value);
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"typeof"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    try {
      return Lpp::Return_Value(
          Lpp::Calc_Value,
          Variable::getTypeStr(cmd.exp_calc(Variable::parse(cmd.args[0]), scope,
                                            all_scope, this_scope)
                                   .tp));
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
  };
  temp[L"eval"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var op;
    try {
      op = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                        this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    if (op.tp == Variable::String)
      return Lpp::Return_Value(
          Lpp::Lpp(op.StringValue, cmd.cmd).eval(scope, all_scope, this_scope));
    else
      return Lpp::Return_Value(Lpp::Calc_Value, op);
  };
  temp[L"void"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    try {
      cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope, this_scope);
    } catch (...) {
      return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                               L"ExpressionError");
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  };
  temp[L"export"] = temp[L"ext"] =
      [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var all_scope,
         Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 0)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    return Lpp::Return_Value(Lpp::Calc_Value, false);
  };
  temp[L"break"] = temp[L"continue"] = temp[L"default"] = temp[L"catch"] =
      temp[L"__native__"] = temp[L"what"] =
          [](const Lpp::Lpp& cmd, Variable::var& scope,
             Variable::var& all_scope,
             Variable::var& this_scope) -> const Lpp::Return_Value {
    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
  };
  temp[L""] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                 Variable::var& all_scope,
                 Variable::var& this_scope) -> const Lpp::Return_Value {
    try {
      if (cmd.isStatement(cmd.toString())) {
        // Variable::var func_temp;
        Lpp::Lpp::Return_Object func_temp;
        try {
          func_temp = cmd.get_object(cmd.name, scope, all_scope, this_scope,
                                     false, false);
        } catch (...) {
          return Lpp::Return_Value(cmd.name, Lpp::Throw_Value,
                                   L"ExpressionError");
        }
        if (func_temp.getConstValue().tp != Variable::Function &&
            (func_temp.getConstValue().tp != Variable::Object ||
             func_temp.getConstValue().ObjectValue.find(L"operator()") ==
                 func_temp.getConstValue().ObjectValue.cend()) &&
            cmd.args.size() != 0) {
          return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
        } else if ((func_temp.getConstValue().tp == Variable::Function ||
                    (func_temp.getConstValue().tp == Variable::Object &&
                     func_temp.getConstValue().ObjectValue.find(
                         L"operator()") !=
                         func_temp.getConstValue().ObjectValue.cend())) &&
                   cmd.args.size() == 1) {
          Variable::var args_temp;
          try {
            const Variable::var& temp = Variable::parse(cmd.args[0]);
            if (temp.tp != Variable::Array)
              return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                       L"SyntaxError");
            args_temp = cmd.exp_calc(temp, scope, all_scope, this_scope);
          } catch (...) {
            return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                     L"ExpressionError");
          }
          // test native
          if (func_temp.getConstValue().FunctionValue.block.value.size() == 1) {
            Lpp::Lpp_base x = Lpp::Lpp_base(
                func_temp.getConstValue().FunctionValue.block.value[0]);
            if (x.name == L"__native__" && x.args.size() == 1) {
              if (func_temp.native()) {
                Variable::var f = Variable::parse(x.args[0]);
                if (f.tp != Variable::String)
                  return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                           L"SyntaxError");
                if (f.StringValue == L"substr") {
                  if (args_temp.ArrayValue.size() < 1 ||
                      args_temp.ArrayValue.size() > 2)
                    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
                  if (args_temp.ArrayValue[0].tp != Variable::Int &&
                      (args_temp.ArrayValue.size() != 2 ||
                       args_temp.ArrayValue[1].tp != Variable::Int))
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"SyntaxError");
                  try {
                    if (args_temp.ArrayValue[0].IntValue < 0 ||
                        args_temp.ArrayValue[0].IntValue >=
                            func_temp.getConstParent().StringValue.length())
                      return Lpp::Return_Value(Lpp::Calc_Value, L"");
                    size_t start = (size_t)args_temp.ArrayValue[0].IntValue;
                    if (args_temp.ArrayValue.size() == 1)
                      return Lpp::Return_Value(
                          Lpp::Calc_Value,
                          func_temp.getConstParent().StringValue.substr(start));
                    if (args_temp.ArrayValue.size() == 2) {
                      if (args_temp.ArrayValue[1].IntValue < 0)
                        return Lpp::Return_Value(Lpp::Calc_Value, L"");
                      size_t count = (size_t)args_temp.ArrayValue[1].IntValue;
                      return Lpp::Return_Value(
                          Lpp::Calc_Value,
                          func_temp.getConstParent().StringValue.substr(start,
                                                                        count));
                    }
                  } catch (...) {
                    return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
                  }
                } else if (f.StringValue == L"split") {
                  if (args_temp.ArrayValue.size() != 1)
                    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
                  if (args_temp.ArrayValue[0].tp != Variable::String)
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"SyntaxError");
                  try {
                    std::vector<Variable::var> ret;
                    std::wstring str = func_temp.getConstParent().StringValue +
                                       args_temp.ArrayValue[0].StringValue;
                    size_t pos;
                    if (args_temp.ArrayValue[0].StringValue == L"") {
                      for (pos = 0; pos < str.length(); pos++) {
                        ret.push_back(
                            Variable::var(std::wstring(1, str[pos]), false));
                      }
                    } else {
                      while ((pos = str.find(
                                  args_temp.ArrayValue[0].StringValue)) !=
                             std::wstring::npos) {
                        ret.push_back(Variable::var(str.substr(0, pos), false));
                        str = str.substr(
                            pos + args_temp.ArrayValue[0].StringValue.length());
                      }
                    }
                    return Lpp::Return_Value(Lpp::Calc_Value, ret);
                  } catch (...) {
                    return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
                  }
                } else if (f.StringValue == L"trim") {
                  if (args_temp.ArrayValue.size() != 0)
                    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
                  return Lpp::Return_Value(
                      Lpp::Calc_Value,
                      Variable::trim(func_temp.getConstParent().StringValue));
                } else if (f.StringValue == L"join") {
                  try {
                    if (args_temp.ArrayValue.size() != 1) throw nullptr;
                    Variable::var sp = args_temp.ArrayValue[0];
                    if (sp.tp != Variable::String) throw nullptr;
                    std::wstring ret;
                    for (size_t i = 0;
                         i < func_temp.getConstParent().ArrayValue.size();
                         i++) {
                      if (i + 1 !=
                          func_temp.getConstParent().ArrayValue.size()) {
                        ret += func_temp.getConstParent()
                                   .ArrayValue[i]
                                   .toString() +
                               sp.StringValue;
                      } else
                        ret +=
                            func_temp.getConstParent().ArrayValue[i].toString();
                    }
                    return Lpp::Return_Value(Lpp::Calc_Value, ret);
                  } catch (...) {
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"SyntaxError");
                  }
                } else if (f.StringValue == L"pop") {
                  if (args_temp.ArrayValue.size() != 0) {
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"SyntaxError");
                  }
                  try {
                    if (func_temp.getThis().ArrayValue.empty()) throw nullptr;
                    func_temp.getThis().ArrayValue.pop_back();
                    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
                  } catch (...) {
                    return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
                  }
                } else if (f.StringValue == L"push") {
                  if (args_temp.ArrayValue.size() != 1)
                    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
                  try {
                    Variable::var obj = args_temp.ArrayValue[0];
                    obj.isConst = false;
                    func_temp.getThis().ArrayValue.push_back(obj);
                    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
                  } catch (...) {
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"ExpressionError");
                  }
                } else if (f.StringValue == L"toString") {
                  return Lpp::Return_Value(
                      Lpp::Calc_Value, func_temp.getConstParent().toString());
                } else if (f.StringValue == L"insert") {
                  if (args_temp.ArrayValue.size() != 2 ||
                      args_temp.ArrayValue[0].tp != Variable::Int ||
                      args_temp.ArrayValue[0].IntValue < 0)
                    return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                             L"SyntaxError");
                  try {
                    if (func_temp.getThis().ArrayValue.size() <=
                        (size_t)args_temp.ArrayValue[0].IntValue)
                      func_temp.getThis().ArrayValue.resize(
                          (size_t)args_temp.ArrayValue[0].IntValue + 1);
                    func_temp.getThis().ArrayValue.insert(
                        func_temp.getThis().ArrayValue.begin() +
                            ((size_t)args_temp.ArrayValue[0].IntValue),
                        args_temp.ArrayValue[1]);
                    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
                  } catch (...) {
                    return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
                  }
                } else if (f.StringValue == L"resize") {
                  if (args_temp.ArrayValue.size() != 1 ||
                      args_temp.ArrayValue[0].tp != Variable::Int)
                    return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
                  try {
                    func_temp.getThis().ArrayValue.resize(
                        (size_t)args_temp.ArrayValue[0].IntValue);
                    for (size_t i = 0;
                         i < func_temp.getThis().ArrayValue.size(); i++)
                      func_temp.getThis().ArrayValue[i].isConst = false;
                    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
                  } catch (...) {
                    return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
                  }
                } else
                  return Lpp::Return_Value(Lpp::Throw_Value, L"EvalError");
              }
            }
          }
          // not native
          Variable::var temp_scope =
              Variable::var(std::map<std::wstring, Variable::var>());
          Variable::var* parent;
          Variable::var c_this;
          Variable::var realfunc;
          try {
            if (func_temp.getConstValue().tp == Variable::Object) {
              Variable::var temp =
                  func_temp.getConstValue().ObjectValue.at(L"operator()");
              realfunc = temp;
              // Lpp::Lpp::Return_Object&& t = cmd.get_object(
              //     cmd.name, scope, all_scope, this_scope, true, false);
              if (func_temp.tp != Lpp::Lpp::is_pointer) {
                c_this = func_temp.getConstValue();
                parent = &c_this;
              } else
                parent = &func_temp.getValue();
            } else {
              realfunc = func_temp.getConstValue();
              // Lpp::Lpp::Return_Object&& t = cmd.get_object(
              //     cmd.name, scope, all_scope, this_scope, true, false);
              if (func_temp.tp != Lpp::Lpp::is_pointer) {
                c_this = func_temp.getConstParent();
                parent = &c_this;
              } else
                parent = &func_temp.getThis();
            }
            if (parent->tp != Variable::Object) parent = &scope;
          } catch (...) {
            parent = &scope;
          }
          std::map<std::wstring, Lpp::Lpp::CmdType> s = cmd.cmd;
          s[L"break"] = s[L"continue"] = s[L"what"] =
              [](const Lpp::Lpp& cmd, Variable::var& scope,
                 Variable::var& all_scope,
                 Variable::var& this_scope) -> const Lpp::Return_Value {
            return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
          };
          try {
            return Lpp::Return_Value(
                Lpp::Calc_Value,
                Lpp::Lpp(L"", s).RunFunc(realfunc, temp_scope, all_scope,
                                         *parent, args_temp, false));
          } catch (Variable::SyntaxErr&) {
            return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
          } catch (const Lpp::Exec_Info& a) {
            return Lpp::Return_Value(a.cmd.toString(), a.tp, a.value);
          } catch (Variable::ExprErr&) {
            return Lpp::Return_Value(Lpp::Throw_Value, L"ExpressionError");
          }
        }
      }
      return Lpp::Return_Value(
          Lpp::Calc_Value, cmd.exp_calc(Variable::parse(cmd.toString()), scope,
                                        all_scope, this_scope));
    } catch (const Lpp::Exec_Info& s) {
      return Lpp::Return_Value(s.cmd.toString(), s.tp, s.value);
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"ExpressionError");
    }
  };
  temp[L"import"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() != 1)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    Variable::var import_mod;
    Variable::var ret_value =
        Variable::var(std::map<std::wstring, Variable::var>());
    try {
      import_mod = cmd.exp_calc(Variable::parse(cmd.args[0]), scope, all_scope,
                                this_scope);
      if (import_mod.tp != Variable::String) throw nullptr;
      std::wifstream s(Variable::WString2String(import_mod.StringValue));
      if (!s.is_open()) {
        return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value, L"EvalError");
      }
      std::map<std::wstring, Lpp::Lpp::CmdType> cmd_temp = cmd.cmd;
      cmd_temp[L"export"] =
          [&ret_value](const Lpp::Lpp& cmd, Variable::var& scope,
                       Variable::var& all_scope,
                       Variable::var& this_scope) -> const Lpp::Return_Value {
        if (cmd.args.size() == 0)
          return Lpp::Return_Value(Lpp::Calc_Value, true);
        if (cmd.args.size() != 1)
          return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
        std::vector<std::wstring> temp =
            Variable::splitBy(Variable::clearnull(cmd.args[0]), L'=');
        if (temp.size() != 2 || !cmd.isIdentifier(temp[0], true))
          return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
        try {
          ret_value.ObjectValue[temp[0]] = cmd.exp_calc(
              Variable::parse(temp[1]), scope, all_scope, this_scope);
        } catch (...) {
          return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                   L"ExpressionError");
        }
        return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
      };
      Variable::var temp_scope =
          Variable::var(std::map<std::wstring, Variable::var>());
      while (!s.eof()) {
        std::wstring m, cm, fin;
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
              continue;
            } else if (cm[i] == L'\n' && a == 0 && j == 0)
              fin += ';';
            else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') &&
                     a == 0)
              j++;
            else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') &&
                     a == 0)
              j--;
            if (cm[i] != L'\n') fin += cm[i];
          }
          if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
            std::wstring temp;
            std::getline(s, temp);
            cm += temp + L"\n";
            fin = L"";
          } else
            break;
        }
        if (s.eof()) break;
        std::vector<std::wstring> w = Variable::code_split(fin);
        Lpp::Exec_Info x;
        for (size_t i = 0; i < w.size(); i++) {
          x = Lpp::Lpp(w[i], cmd_temp).eval(temp_scope);
          temp_scope = x.scope;
          if (x.tp == Lpp::Throw_Value) {
            return Lpp::Return_Value(x.cmd.toString(), x.tp, x.value);
          }
        }
      }
      return Lpp::Return_Value(Lpp::Calc_Value, ret_value);
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, "SyntaxError");
    }
    return Lpp::Return_Value(Lpp::Calc_Value, ret_value);
  };
  return temp;
}
#ifdef ENABLE_EXT
Lpp::Return_Value ext_fn(const std::wstring& name,
                         const std::vector<Variable::var>& args,
                         Variable::var& scope, Variable::var& all_scope,
                         Variable::var& this_scope) {
  if (name == L"system") {
    /*
    POSIX Standard Function "system"(name:string,arguments:array)
    execute a program with arguments.
    */
    if (args.size() != 2 || args[0].tp != Variable::String ||
        args[1].tp != Variable::Array)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");
    std::wstring run = args[0].StringValue + L" ";
    for (size_t i = 0; i < args[1].ArrayValue.size(); i++) {
      if (args[1].ArrayValue[i].tp == Variable::String)
        run += args[1].ArrayValue[i].StringValue + L" ";
      else
        run += args[1].ArrayValue[i].toString() + L" ";
    }
    return Lpp::Return_Value(
        Lpp::Calc_Value,
        system(
            Variable::WString2String(run.substr(0, run.length() - 1)).c_str()));
  } else if (name == L"print") {
    /*
    STL Standard Function "print"(...)
    print a value to screen.
    */
    for (size_t i = 0; i < args.size(); i++) {
      if (args[i].tp == Variable::String)
        std::wcout << args[i].StringValue;
      else
        std::wcout << args[i].toString();
    }
    return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
  } else if (name == L"getline") {
    /*
    STL Standard Function "getline"(null)
    Get a line from standard input.
    */
    std::wcin.clear();
    std::wstring temp;
    std::getline(std::wcin, temp);
    std::wcin.clear();
    return Lpp::Return_Value(Lpp::Calc_Value, temp);
  } else
    return Lpp::Return_Value(name, Lpp::Throw_Value, L"EvalError");
}
#endif
int main(int argc, char** argv) {
  std::map<std::wstring, Lpp::Lpp::CmdType> behav = getFunc();
#ifdef ENABLE_EXT
  behav[L"ext"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                     Variable::var& all_scope,
                     Variable::var& this_scope) -> const Lpp::Return_Value {
    if (cmd.args.size() == 0) return Lpp::Return_Value(Lpp::Calc_Value, true);
    if (cmd.args.size() != 2)
      return Lpp::Return_Value(Lpp::Throw_Value, L"SyntaxError");

    try {
      const Variable::var& load_mod = cmd.exp_calc(
          Variable::parse(cmd.args[0]), scope, all_scope, this_scope);
      if (load_mod.tp != Variable::String)
        return Lpp::Return_Value(cmd.args[0], Lpp::Throw_Value,
                                 L"ExpressionError");
      Variable::var arg;
      try {
        const Variable::var& temp = Variable::parse(cmd.args[1]);
        if (temp.tp != Variable::Array)
          return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                   L"SyntaxError");
        arg = cmd.exp_calc(temp, scope, all_scope, this_scope);
      } catch (...) {
        return Lpp::Return_Value(cmd.args[1], Lpp::Throw_Value,
                                 L"ExpressionError");
      }
      return ext_fn(load_mod.StringValue, arg.ArrayValue, scope, all_scope,
                    this_scope);
    } catch (...) {
      return Lpp::Return_Value(Lpp::Throw_Value, L"ExpressionError");
    }
  };  // extend internal command:__ext__
#endif
  std::locale::global(std::locale(""));
  std::vector<std::wstring> arg(argc - 1);
  std::vector<std::wstring> program_arg;
  for (size_t i = 1; i < (size_t)argc; i++) {
    arg[i - 1] = Variable::String2WString(argv[i]);
  }
  if (!arg.empty())
    program_arg = std::vector<std::wstring>(arg.cbegin(), arg.cend());
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
    program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    program_arg.insert(program_arg.begin(), Variable::String2WString(argv[0]));
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f = Variable::var(std::map<std::wstring, Variable::var>());
    f.ObjectValue[L"arguments"] = var_arg;
    while (!std::wcin.eof()) {
      std::wstring m, cm, fin;
      std::getline(std::wcin, m);
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
            continue;
          } else if (cm[i] == L'\n' && a == 0 && j == 0)
            fin += ';';
          else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0)
            j++;
          else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0)
            j--;
          if (cm[i] != L'\n') fin += cm[i];
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::getline(std::wcin, temp);
          cm += temp + L"\n";
          fin = L"";
        } else
          break;
      }
      if (std::wcin.eof()) break;
      std::vector<std::wstring> w = Variable::code_split(fin);
      Lpp::Exec_Info x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Value) {
          if (x.msg == L"")
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`" << std::endl;
          else
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`" << L",near `" << x.msg
                       << L"`" << std::endl;
          return 1;
        } else if (x.tp == Lpp::Ret_Value) {
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
    std::wcout << L"type \"copyright\" or \"help\" for more information."
               << std::endl;
    behav[L"help"] = [](const Lpp::Lpp& cmd, Variable::var& scope,
                        Variable::var& all_scope,
                        Variable::var& this_scope) -> const Lpp::Return_Value {
      std::wcout << L"How to use L++ in interactive mode" << std::endl;
      std::wcout << L"Use exit or EOF to exit." << std::endl;
      return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
    };  // extend hotpatch about help
    behav[L"copyright"] =
        [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
           Variable::var& this_scope) -> const Lpp::Return_Value {
      std::wcout << L"Copyright (c) 2021 nu11ptr team." << std::endl;
      std::wcout << L"This program was under the MIT license." << std::endl;
      std::wcout << L"Thanks for using L++." << std::endl;
      return Lpp::Return_Value(Lpp::Calc_Value, nullptr);
    };  // extend hotpatch about copyright
    behav[L"exit"] =
        [](const Lpp::Lpp& cmd, Variable::var& scope, Variable::var& all_scope,
           Variable::var& this_scope) -> const Lpp::Return_Value { exit(0); };
    // std::wcout << strmap[L"lpp_terminal_welcome"].format() << std::endl;
    program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    program_arg.insert(program_arg.begin(), Variable::String2WString(argv[0]));
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f = Variable::var(std::map<std::wstring, Variable::var>());
    f.ObjectValue[L"arguments"] = var_arg;
    while (!std::wcin.eof()) {
      std::wstring m, cm, fin;
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
            continue;
          } else if (cm[i] == L'\n' && a == 0 && j == 0)
            fin += ';';
          else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0)
            j++;
          else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0)
            j--;
          if (cm[i] != L'\n') fin += cm[i];
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::wcout << L"... " << std::flush;
          std::getline(std::wcin, temp);
          cm += temp + L"\n";
          fin = L"";
        } else
          break;
        a = 0;
        j = 0;
      }
      if (std::wcin.eof()) return 0;
      std::vector<std::wstring> w = Variable::code_split(fin);
      Lpp::Exec_Info x;
      bool inerr = false;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Value) {
          if (x.msg == L"")
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`" << std::endl;
          else
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`,near `" << x.msg << L"`"
                       << std::endl;
          inerr = true;
          break;
        }
      }
      if (!inerr) std::wcout << L"<- " << x.value.toString() << std::endl;
      m = L"";
      cm = L"";
    }
  } else if (arg.size() > 0) {
    std::wifstream s(argv[1]);
    if (!s.is_open()) {
      std::wcout << arg[0] << L": No such file or directory" << std::endl;
      return 1;
    }
    program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    program_arg.insert(program_arg.begin(), arg[0]);
    Variable::var var_arg(nullptr, true);
    var_arg.tp = Variable::Array;
    var_arg.ArrayValue = std::vector<Variable::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg.ArrayValue[i] = Variable::var(program_arg[i], false);
    }
    Variable::var f = Variable::var(std::map<std::wstring, Variable::var>());
    f.ObjectValue[L"arguments"] = var_arg;
    while (!s.eof()) {
      std::wstring m, cm, fin;
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
            continue;
          } else if (cm[i] == L'\n' && a == 0 && j == 0)
            fin += ';';
          else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a == 0)
            j++;
          else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a == 0)
            j--;
          if (cm[i] != L'\n') fin += cm[i];
        }
        if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
          std::wstring temp;
          std::getline(s, temp);
          cm += temp + L"\n";
          fin = L"";
        } else
          break;
      }
      if (s.eof()) break;
      std::vector<std::wstring> w = Variable::code_split(fin);
      Lpp::Exec_Info x;
      for (size_t i = 0; i < w.size(); i++) {
        x = Lpp::Lpp(w[i], behav).eval(f);
        f = x.scope;
        if (x.tp == Lpp::Throw_Value) {
          if (x.msg == L"")
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`" << std::endl;
          else
            std::wcout << L"Uncaught " << x.value.toString() << L" at `"
                       << x.cmd.toString() << L"`,near `" << x.msg << L"`"
                       << std::endl;
          return 1;
        } else if (x.tp == Lpp::Ret_Value) {
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
