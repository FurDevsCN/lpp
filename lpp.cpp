/*
L++ was under the MIT license.
Copyright(c) 2022 FurDevsCN.
*/
#include "./lpp.h"

#include <iostream>
#include <locale>
#include <string>
#include <vector>
constexpr const wchar_t* VERSION_INFO = L"1.8.0-20220515_dev";

/*
  public const std::string String2WString(const std::wstring& s);
  converts narrow string 's' to wide string.
*/
const std::wstring String2WString(const std::string& s) {
  const std::codecvt<wchar_t, char, std::mbstate_t>& cvt =
      std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(
          std::locale());
  std::mbstate_t mb{};
  std::wstring t(s.length(), L'\0');
  const char* from_next;
  wchar_t* to_next;
  std::codecvt_base::result result = cvt.in(
      mb, &s[0], &s[s.length()], from_next, &t[0], &t[t.length()], to_next);
  if (result != std::codecvt_base::ok) {
    return std::wstring(s.length(), '?');
  }
  t.resize(to_next - &t[0]);
  return t;
}
/*
  public const std::string WString2String(const std::wstring& s);
  converts wide string 's' to narrow string.
*/
const std::string WString2String(const std::wstring& s) {
  const std::codecvt<wchar_t, char, std::mbstate_t>& cvt =
      std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(
          std::locale());
  std::mbstate_t mb{};
  std::string t(s.length() * cvt.max_length(), L'\0');
  const wchar_t* from_next;
  char* to_next;
  std::codecvt_base::result result = cvt.out(
      mb, &s[0], &s[s.length()], from_next, &t[0], &t[t.length()], to_next);
  if (result != std::codecvt_base::ok) {
    return std::string(s.length(), '?');
  }
  t.resize(to_next - &t[0]);
  return t;
}
/*
  public const std::wstring trim(const std::wstring& p);
  removes the blank characters before and after the real content of the wide
  string 'p'.
*/
const std::wstring trim(const std::wstring& p) {
  std::wstring x = p;
  x.erase(0, x.find_first_not_of(' '));
  x.erase(x.find_last_not_of(' ') + 1);
  x.erase(0, x.find_first_not_of('\r'));
  x.erase(x.find_last_not_of('\r') + 1);
  x.erase(0, x.find_first_not_of('\n'));
  x.erase(x.find_last_not_of('\n') + 1);
  x.erase(0, x.find_first_not_of('\t'));
  x.erase(x.find_last_not_of('\t') + 1);
  x.erase(0, x.find_first_not_of('\v'));
  x.erase(x.find_last_not_of('\v') + 1);
  x.erase(0, x.find_first_not_of('\f'));
  x.erase(x.find_last_not_of('\f') + 1);
  return x;
}
/*
public const std::wstring getTypeStr(const _ValueType& x);
转换_ValueType到字符串标识符。
*/
const std::wstring getTypeStr(const Var::_ValueType& x) {
  switch (x) {
    case Var::Null:
      return L"null";
    case Var::Number:
      return L"number";
    case Var::Boolean:
      return L"boolean";
    case Var::String:
      return L"string";
    case Var::Array:
      return L"array";
    case Var::Object:
      return L"object";
    case Var::Function:
      return L"function";
    default:
      throw Var::Error::ConvError;
  }
}
/*
public _ValueType getStrType(const std::wstring& x);
转换字符串标识符到_ValueType。
*/
Var::_ValueType getStrType(const std::wstring& x) {
  if (x == L"null") return Var::Null;
  if (x == L"int") return Var::Number;
  if (x == L"boolean") return Var::Boolean;
  if (x == L"string") return Var::String;
  if (x == L"array") return Var::Array;
  if (x == L"object") return Var::Object;
  if (x == L"function") return Var::Function;
  throw Var::Error::ConvError;
}
Handler::TableType getFunc() {
  Handler::TableType f;
  //f[L"return"] = [] keyword(cmd, handler){};
  return f;
}
int main(int argc, char** argv) {
  Handler::TableType behav = getFunc();
#ifdef ENABLE_EXT
  behav[L"ext"] = [](const Parser& cmd, Handler& handler) -> const RetVal {
    if (cmd.args.size() == 0) return Calc(true);
    if (cmd.args.size() != 1) throw Var::var(L"SyntaxError");
    std::pair<std::wstring, std::wstring> sp = cmd.splitCall(cmd.args[0]);
    sp.first = Var::clearnull(sp.first);
    sp.second = Var::clearnull(sp.second);
    Var::var arg;
    try {
      const Var::var& temp = Var::parse(sp.second);
      if (temp.tp != Var::Array)
        return RetVal(sp.second, Throw_Value, L"SyntaxError");
      arg = handler.expr(temp, scope, all_scope, this_scope);
    } catch (...) {
      return RetVal(sp.second, Throw_Value, L"ExpressionError");
    }
    return ext_fn(sp.first, arg(Var::Array), scope, all_scope, this_scope);
  };  // extend internal command:__ext__
#else
  behav[L"ext"] = [](const Parser& cmd, Handler&) -> RetVal {
    if (cmd.args.size() != 0) throw Var::var(L"SyntaxError");
    return Calc(false);
  };
#endif
  std::locale::global(std::locale(""));
  std::vector<std::wstring> arg(argc - 1);
  std::vector<std::wstring> program_arg;
  for (size_t i = 1; i < (size_t)argc; i++) {
    arg[i - 1] = String2WString(argv[i]);
  }
  if (!arg.empty())
    program_arg = std::vector<std::wstring>(arg.cbegin(), arg.cend());
  if (arg.size() == 0 || arg[0] == L"-h" || arg[0] == L"--help") {
    std::wcout << L"lpp (" << VERSION_INFO << L")" << std::endl;
    std::wcout << L"Copyright(c) FurDevsCN 2022." << std::endl;
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
    std::wcout << L"Copyright(c) FurDevsCN 2022." << std::endl;
    return 0;
  } else if (arg[0] == L"-") {
    // program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    // program_arg.insert(program_arg.begin(), String2WString(argv[0]));
    // Var::var var_arg(nullptr, true);
    // var_arg.tp = Var::Array;
    // var_arg(Var::Array) = std::vector<Var::var>(program_arg.size());
    // for (size_t i = 0; i < program_arg.size(); i++) {
    //   var_arg[i] = Var::var(program_arg[i], false);
    // }
    // Var::var f = Var::var(std::map<std::wstring, Var::var>());
    // Next_Value z;
    // f[L"arguments"] = var_arg;
    // while (!std::wcin.eof()) {
    //   std::wstring m, cm, fin;
    //   std::getline(std::wcin, m);
    //   cm = m + L"\n";
    //   while (!std::wcin.eof()) {
    //     size_t a = 0, j = 0;
    //     for (size_t i = 0, z = 0; i < cm.length(); i++) {
    //       if (cm[i] == L'\\')
    //         z = !z;
    //       else if (cm[i] == L'\"' && !z) {
    //         if (a == 0 || a == 1) a = !a;
    //       } else if (cm[i] == L'\'' && !z) {
    //         if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    //       } else
    //         z = 0;
    //       if (cm[i] == L'#' && a == 0) {
    //         while (i < cm.length() && cm[i] != L'\n') i++;
    //         continue;
    //       } else if (cm[i] == L'\n' && a == 0 && j == 0)
    //         fin += ';';
    //       else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a ==
    //       0)
    //         j++;
    //       else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a ==
    //       0)
    //         j--;
    //       if (cm[i] != L'\n') fin += cm[i];
    //     }
    //     if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
    //       std::wstring temp;
    //       std::getline(std::wcin, temp);
    //       cm += temp + L"\n";
    //       fin = L"";
    //     } else
    //       break;
    //   }
    //   if (std::wcin.eof()) break;
    //   std::vector<std::wstring> w = Var::code_split(fin);
    //   RetVal x;
    //   for (size_t i = 0; i < w.size(); i++) {
    //     x = Parser(w[i], behav).eval(f, z);
    //     f = x.scope;
    //     z = x.last;
    //     if (x.tp == Throw_Value) {
    //       if (x.msg == L"")
    //         std::wcout << L"Uncaught " << x.value.toString() << L" at `"
    //                    << x.cmd.toString() << L"`" << std::endl;
    //       else
    //         std::wcout << L"Uncaught " << x.value.toString() << L" at `"
    //                    << x.cmd.toString() << L"`" << L",near `" << x.msg
    //                    << L"`" << std::endl;
    //       return 1;
    //     } else if (x.tp == Ret_Value) {
    //       try {
    //         x.value = x.value.convert(Var::Number);
    //         return (int)x.value(Var::Number);
    //       } catch (...) {
    //         return 0;
    //       }
    //     }
    //   }
    // }
  } else if (arg[0] == L"-i" || arg[0] == L"--interactive") {
    std::wcout << L"lpp interpreter (" << VERSION_INFO << L")" << std::endl;
    std::wcout << L"type \"copyright\" or \"help\" for more information."
               << std::endl;
    behav[L"help"] = [](const Parser&, Handler&) -> RetVal {
      std::wcout << L"How to use L++ in interactive mode" << std::endl;
      std::wcout << L"Use exit or EOF to exit." << std::endl;
      return Calc(nullptr);
    };  // extend hotpatch about help
    behav[L"copyright"] = [](const Parser&, Handler&) -> RetVal {
      std::wcout << L"Copyright (c) 2021 nu11ptr team." << std::endl;
      std::wcout << L"This program was under the MIT license." << std::endl;
      std::wcout << L"Thanks for using L++." << std::endl;
      return Calc(nullptr);
    };  // extend hotpatch about copyright
    behav[L"exit"] = [](const Parser&, Handler&) -> RetVal { exit(0); };
    program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    program_arg.insert(program_arg.begin(), String2WString(argv[0]));
    Var::var var_arg(nullptr, true);
    var_arg.tp = Var::Array;
    var_arg(Var::Array) = std::vector<Var::var>(program_arg.size());
    for (size_t i = 0; i < program_arg.size(); i++) {
      var_arg(Var::Array)[i] = Var::var(program_arg[i], false);
    }
    Var::var f = ScopeType();
    f(Var::Object)[L"arguments"] = var_arg;
    Handler g(f(Var::Object), f(Var::Object), f, NextVal(), behav, Native());
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
          fin += cm[i];
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
      std::vector<std::wstring> w = Var::codeSplit(fin);
      RetVal x;
      bool inerr = false;
      for (size_t i = 0; i < w.size(); i++) {
        try {
          x = g(Parser(w[i]));
        } catch (const Error& t) {
          std::wcout << t.msg << std::endl;
          for (std::list<Handler>::const_iterator i = t.stack.cbegin();
               i != t.stack.cend(); i++) {
            std::wcout << L"at " << (*i).this_scope().toString() << std::endl;
          }
          inerr = true;
          break;
        }
        if (x.tp == Throw_Value) {
          std::wcout << L"Uncaught " << x.value.toString() << std::endl;
          for (std::list<Handler>::const_iterator i = x.stack.cbegin();
               i != x.stack.cend(); i++) {
            std::wcout << L"at " << (*i).this_scope().toString() << std::endl;
          }
          inerr = true;
          break;
        }
      }
      if (!inerr) std::wcout << L"<- " << x.value.toString() << std::endl;
      m = L"";
      cm = L"";
    }
  } else if (arg.size() > 0) {
    // std::wifstream s(argv[1]);
    // if (!s.is_open()) {
    //   std::wcout << arg[0] << L": No such file or directory" << std::endl;
    //   return 1;
    // }
    // program_arg = std::vector<std::wstring>(arg.cbegin() + 1, arg.cend());
    // program_arg.insert(program_arg.begin(), arg[0]);
    // Var::var var_arg(nullptr, true);
    // var_arg.tp = Var::Array;
    // var_arg(Var::Array) = std::vector<Var::var>(program_arg.size());
    // for (size_t i = 0; i < program_arg.size(); i++) {
    //   var_arg[i] = Var::var(program_arg[i], false);
    // }
    // Var::var f = Var::var(std::map<std::wstring, Var::var>());
    // f[L"arguments"] = var_arg;
    // while (!s.eof()) {
    //   std::wstring m, cm, fin;
    //   std::getline(s, m);
    //   cm = m + L"\n";
    //   while (!s.eof()) {
    //     size_t a = 0, j = 0;
    //     for (size_t i = 0, z = 0; i < cm.length(); i++) {
    //       if (cm[i] == L'\\')
    //         z = !z;
    //       else if (cm[i] == L'\"' && !z) {
    //         if (a == 0 || a == 1) a = !a;
    //       } else if (cm[i] == L'\'' && !z) {
    //         if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    //       } else
    //         z = 0;
    //       if (cm[i] == L'#' && a == 0) {
    //         while (i < cm.length() && cm[i] != L'\n') i++;
    //         continue;
    //       } else if (cm[i] == L'\n' && a == 0 && j == 0)
    //         fin += ';';
    //       else if ((cm[i] == L'[' || cm[i] == L'{' || cm[i] == L'(') && a ==
    //       0)
    //         j++;
    //       else if ((cm[i] == L']' || cm[i] == L'}' || cm[i] == L')') && a ==
    //       0)
    //         j--;
    //       if (cm[i] != L'\n') fin += cm[i];
    //     }
    //     if (a != 0 || j != 0 || cm[cm.length() - 1] == L',') {
    //       std::wstring temp;
    //       std::getline(s, temp);
    //       cm += temp + L"\n";
    //       fin = L"";
    //     } else
    //       break;
    //   }
    //   if (s.eof()) break;
    //   std::vector<std::wstring> w = Var::code_split(fin);
    //   RetVal x;
    //   for (size_t i = 0; i < w.size(); i++) {
    //     x = Parser(w[i], behav).eval(f, Next_Value());
    //     f = x.scope;
    //     if (x.tp == Throw_Value) {
    //       if (x.msg == L"")
    //         std::wcout << L"Uncaught " << x.value.toString() << L" at `"
    //                    << x.cmd.toString() << L"`" << std::endl;
    //       else
    //         std::wcout << L"Uncaught " << x.value.toString() << L" at `"
    //                    << x.cmd.toString() << L"`,near `" << x.msg << L"`"
    //                    << std::endl;
    //       return 1;
    //     } else if (x.tp == Ret_Value) {
    //       try {
    //         x.value = x.value.convert(Var::Number);
    //         return (int)x.value(Var::Number);
    //       } catch (...) {
    //         return 0;
    //       }
    //     }
    //   }
    // }
    // s.close();
  }
  return 0;
}