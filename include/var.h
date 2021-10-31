/*
L++ programming language was under the MIT license.
Copyright(c) 2021 nu11ptr team.
*/
#ifndef VAR_MODULE
#define VAR_MODULE
#include <locale>
#include <map>
#include <string>
#include <vector>
namespace Variable {
typedef struct BaseErr {
  const std::wstring &what() const { return _err; }
  BaseErr() {}
  BaseErr(const std::wstring &err) { _err = err; }

 private:
  std::wstring _err;
} BaseErr;
typedef struct ConvFail : public BaseErr {
  ConvFail(const std::wstring &t) : BaseErr(t){};
} ConvFail;
typedef struct ExprErr : public BaseErr {
  ExprErr(const std::wstring &t) : BaseErr(t){};
} ExprErr;
typedef struct SyntaxErr : public BaseErr {
  SyntaxErr(const std::wstring &t) : BaseErr(t){};
} SyntaxErr;
const int get_op_priority(const std::wstring &op) {
  if (op == L"+") return 1;
  if (op == L",") return 0;
  if (op == L"-") return 1;
  if (op == L"*") return 2;
  if (op == L"/") return 2;
  if (op == L"%") return 2;
  if (op == L"(") return 1;
  if (op == L")") return 3;
  if (op == L"==") return 1;
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
  if (op == L">>") return 1;
  if (op == L"<<") return 1;
  if (op == L"^") return 1;
  if (op == L"&") return 1;
  if (op == L"~") return 0;
  if (op == L"|") return 1;
  if (op == L">>>") return 1;
  if (op == L"=") return 1;
  return -1;
}
const bool isExpression(const std::wstring &p) {
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
    else if (get_op_priority(std::wstring(1, p[i])) != -1 &&
             (i == 0 || (p[i - 1] != L'e' && p[i - 1] != L'E')) &&
             (i != 0 || p[i] != L'-') && a == 0 && j == 0)
      return true;
  }
  return false;
}
const std::vector<std::wstring> genExpression(
    const std::vector<std::wstring> &p) {
  std::vector<std::wstring> ret;
  std::vector<std::wstring> temp;
  std::vector<std::wstring> fin;
  std::wstring op;
  for (std::vector<std::wstring>::const_reverse_iterator i = p.crbegin();
       i != p.crend(); i++) {
    op = (*i);
    if ((get_op_priority(op) != -1) && op != L")" && op != L"(") {
      while (!temp.empty() && temp[temp.size() - 1] != L")" &&
             get_op_priority(temp[temp.size() - 1]) > get_op_priority(op)) {
        ret.push_back(temp[temp.size() - 1]);
        temp.pop_back();
      }
      temp.push_back(op);
    } else if (op == L")") {
      temp.push_back(op);
    } else if (op == L"(") {
      while (!temp.empty()) {
        if (temp[temp.size() - 1] == L")") {
          temp.pop_back();
          break;
        }
        ret.push_back(temp[temp.size() - 1]);
        temp.pop_back();
      }
    } else
      ret.push_back(op);
  }
  for (size_t i = 0; i < temp.size(); i++) ret.push_back(*(temp.crbegin() + i));
  for (size_t i = 0; i < ret.size(); i++) fin.push_back(*(ret.crbegin() + i));
  return fin;
}
const std::vector<std::wstring> splitExpression(const std::wstring &p) {
  size_t a = 0;
  int f = 0;
  bool x = false;
  bool z = false;
  std::vector<std::wstring> ret;
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
      case L'|':
      case L'<': {
        if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
          if (ret[ret.size() - 1][ret[ret.size() - 1].length() - 1] == p[i])
            ret[ret.size() - 1] += p[i];
          else
            ret.push_back(std::wstring(1, p[i]));
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
          if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
          if (get_op_priority(ret[ret.size() - 1]) != -1)
            ret[ret.size() - 1] += p[i];
          else
            ret.push_back(std::wstring(1, p[i]));
          x = false;
        } else
          temp += p[i];
        break;
      }
      case L'+':
      case L'-': {
        if (a == 0 && f == 0 && (!ret.empty()) &&
            get_op_priority(ret[ret.size() - 1]) != -1 &&
            ret[ret.size() - 1] != L")")
          ret[ret.size() - 1] += p[i];
        else if (a == 0 && f == 0 && x) {
          if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
          ret[ret.size() - 1] += p[i];
        } else if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
            x = true;
          }
          ret.push_back(std::wstring(1, p[i]));
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
          if (p[i] == L'+') {
            if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
            if (ret[ret.size() - 1][ret[ret.size() - 1].length() - 1] == L'+') {
              ret[ret.size() - 1] += L'+';
              break;
            }
          }
          x = true;
          ret.push_back(std::wstring(1, p[i]));
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
  if (f != 0) throw ExprErr(L"f!=0");
  if (a != 0) throw ExprErr(L"a!=0");
  if (temp != L"") ret.push_back(temp);
  if (get_op_priority(ret[ret.size() - 1]) != -1 &&
      ret[ret.size() - 1] != L"--" && ret[ret.size() - 1] != L"++" &&
      ret[ret.size() - 1] != L")")
    throw ExprErr(L"got a unexpected operator at EOL");
  return ret;
}
const size_t Hex2Dec(const std::wstring &m) {
  size_t l;
  l = std::stoi(m, 0, 16);
  return l;
}
const std::wstring Unicode2String(const std::wstring &str) {
  if (str == L"") return L"";
  return std::wstring(1, Hex2Dec(L"0x" + str));
}
const std::wstring String2WString(const std::string &s) {
  std::wstring t(s.length(), L'\0');
  size_t q = mbstowcs(&t[0], &s[0], s.length());
  if ((int)q == -1)
    return std::wstring(s.length(), L'?');
  else
    t.resize(q);
  return t;
}
const std::string WString2String(const std::wstring &s) {
  std::string t(s.length() * 8, L'\0');
  size_t q = wcstombs(&t[0], &s[0], t.length());
  if ((int)q == -1)
    return std::string(t.length(), L'?');
  else
    t.resize(q);
  return t;
}
const std::wstring trim(const std::wstring &p) {
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
const std::wstring clearnull(const std::wstring &p) {
  std::wstring x = trim(p), tmp;
  bool have_exp = true;
  for (size_t i = 0, a = 0, z = 0, j = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' && !z) {
      if (a == 0 || a == 1) a = !a;
    } else if (x[i] == L'\'' && !z) {
      if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
    } else
      z = 0;
    if ((x[i] == L'\r' || x[i] == L'\n' || x[i] == L'\t') && a == 0)
      continue;
    else if (x[i] == L' ' && a == 0 && j == 0 &&
             (i + 1 == x.length() ||
              (x[i + 1] != L'[' && x[i + 1] != L'{' && x[i + 1] != L'(' &&
               x[i + 1] != L'\'' && x[i + 1] != L'\"') ||
              have_exp) &&
             (i + 1 == x.length() ||
              get_op_priority(std::wstring(1, x[i + 1])) != -1 ||
              x[i + 1] == L' ')) {
      if (have_exp && x[i + 1] != L' ') have_exp = false;
      continue;
    } else if ((x[i] == L'[' || x[i] == L'{' || x[i] == L'(') && a == 0)
      j++;
    else if ((x[i] == L']' || x[i] == L'}' || x[i] == L')') && a == 0)
      j--;
    if (get_op_priority(std::wstring(1, x[i])) != -1) have_exp = true;
    tmp += x[i];
  }
  return tmp;
}
const std::vector<std::wstring> code_split(const std::wstring &x) {
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
    if (x[i] == L'\n' && a == 0 && j == 0)
      p += ';';
    else if ((x[i] == L'[' || x[i] == L'{' || x[i] == L'(') && a == 0)
      j++;
    else if ((x[i] == L']' || x[i] == L'}' || x[i] == L')') && a == 0)
      j--;
    if (x[i] != L'\n') p += x[i];
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
    std::wstring w;
    for (size_t j = 0, flag = 0; j < ret[i].length(); j++) {
      if (ret[i][j] == L' ' && flag == 0)
        continue;
      else {
        flag = 1;
        w += ret[i][j];
      }
    }
    ret[i] = w;
  }
  std::vector<std::wstring> fin;
  for (size_t i = 0; i < ret.size(); i++) {
    if (ret[i] != L"" && ret[i] != L";") fin.push_back(ret[i]);
  }
  return fin;
}
typedef enum var_tp {
  Null = 0,
  Int = 1,
  Boolean = 2,
  String = 3,
  Array = 4,
  Object = 5,
  Function = 6,
  StmtBlock = 7,
  Expression = 8,
} var_tp;
const std::wstring getTypeStr(const var_tp &x) {
  switch (x) {
    case Null:
      return L"null";
    case Int:
      return L"int";
    case Boolean:
      return L"bool";
    case String:
      return L"string";
    case Array:
      return L"array";
    case Object:
      return L"object";
    case Function:
      return L"function";
    default:
      throw ExprErr(L"This type cannot convert to string");
  }
}
const var_tp getStrType(const std::wstring &x) {
  if (x == L"null") return Null;
  if (x == L"int") return Int;
  if (x == L"bool") return Boolean;
  if (x == L"string") return String;
  if (x == L"array") return Array;
  if (x == L"object") return Object;
  if (x == L"function") return Function;
  throw ExprErr(L"This type cannot convert from string");
}
const std::vector<std::wstring> splitBy(const std::wstring &x,
                                        const wchar_t &delim) {
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
typedef class var {
  bool needtoRemove;

 public:
  typedef struct Stmt_temp {
    std::vector<std::wstring> value;
    Stmt_temp() {}
    Stmt_temp(const std::vector<std::wstring> &x) { value = x; }
  } Stmt_temp;
  typedef struct Func_temp {
    typedef struct Arg_Item {
      std::wstring name;
      std::wstring value;
      Arg_Item() {}
      Arg_Item(const std::wstring &_name, const std::wstring &_value = L"") {
        name = _name;
        value = _value;
      }
      const std::wstring toString() const {
        if (value != L"") return name + L"=" + value;
        return name;
      }
      const bool operator==(const Arg_Item &comp) const {
        return name == comp.name && value == comp.value;
      }
    } Arg_Item;
    std::vector<Arg_Item> args;
    Stmt_temp block;
    Func_temp() {}
    Func_temp(const std::vector<Arg_Item> &_args, const Stmt_temp &_block) {
      args = _args;
      block = _block;
    }
  } Func_temp;
  bool isConst;
  std::wstring StringValue;
  std::vector<var> ArrayValue;
  std::map<std::wstring, var> ObjectValue;
  Stmt_temp StmtValue;
  Func_temp FunctionValue;
  std::vector<std::wstring> ExpressionValue;
  var() {
    tp = Null;
    isConst = true;
    needtoRemove = false;
  }
  var(const bool x, bool c = true) {
    tp = Boolean;
    BooleanValue = x;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::nullptr_t &, const bool c = true) {
    tp = Null;
    isConst = c;
    needtoRemove = false;
  }
  var(const int &x, const bool c = true) {
    IntValue = x, tp = Int;
    isConst = c;
    needtoRemove = false;
  }
  var(const double &x, const bool c = true) {
    IntValue = x, tp = Int;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::wstring &x, const bool c = true) {
    StringValue = x, tp = String;
    isConst = c;
    needtoRemove = false;
  }
  var(const wchar_t *const x, const bool c = true) {
    StringValue = std::wstring(x), tp = String;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::vector<var> &x, const bool c = true) {
    ArrayValue = x, tp = Array;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::map<std::wstring, var> &x, const bool c = true) {
    ObjectValue = x, tp = Object;
    if (x.find(L"__constructor__") == x.cend()) {
      ObjectValue[L"__constructor__"].tp = Object;
    }
    isConst = c;
    needtoRemove = false;
  }
  var(const Stmt_temp &x, const bool c = true) {
    StmtValue = x, tp = StmtBlock;
    isConst = c;
    needtoRemove = false;
  }
  var(const Func_temp &x, const bool c = true) {
    FunctionValue = x, tp = Function;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::vector<std::wstring> &x, const bool c = true) {
    ExpressionValue = x, tp = Expression;
    isConst = c;
    needtoRemove = false;
  }
  double IntValue;
  bool BooleanValue;
  var_tp tp;
  void remove() { needtoRemove = true; }
  void update() {
    switch (tp) {
      case Array: {
        std::vector<var> temp;
        for (size_t i = 0; i < ArrayValue.size(); i++) {
          if (ArrayValue[i].needtoRemove == false)
            temp.push_back(ArrayValue[i]);
        }
        ArrayValue = temp;
        return;
      }
      case Object: {
        std::map<std::wstring, var> temp;
        for (std::map<std::wstring, var>::const_iterator x =
                 ObjectValue.cbegin();
             x != ObjectValue.cend(); x++) {
          if (x->second.needtoRemove == false) temp[x->first] = x->second;
        }
        ObjectValue = temp;
        return;
      }
      default:
        return;
    }
  }
  const std::wstring toString() const {
    std::wstring tmp;
    switch (tp) {
      case Null:
        return L"null";
      case Int: {
        if (IntValue == (int)IntValue) return std::to_wstring((int)IntValue);
        return std::to_wstring(IntValue);
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
        tmp = L"function(";
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
  const var convert(const var_tp &type) const {
    if (tp == type) return *this;
    if (type == String) return toString();
    switch (tp) {
      case Int: {
        if (type == Boolean) return (bool)IntValue;
        break;
      }
      case Boolean: {
        if (type == Int) return (bool)IntValue;
        break;
      }
      case String: {
        if (type == Array) {
          std::vector<var> ret(StringValue.length());
          for (size_t i = 0; i < StringValue.length(); i++) {
            ret[i] = std::wstring(1, StringValue[i]);
          }
          return ret;
        } else if (type == Int) {
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
    throw ConvFail(getTypeStr(tp) + L"->" + getTypeStr(type));
  }
  const var opcall_single(const std::wstring &op) const {
    if (op == L"~") {
      switch (tp) {
        case Int: {
          return ~(int)IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (op == L"-") {
      switch (tp) {
        case Int: {
          return -IntValue;
        }
        case Boolean: {
          return -BooleanValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (op == L"+") {
      switch (tp) {
        case Int: {
          return IntValue;
        }
        case Boolean: {
          return BooleanValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (op == L"!") {
      if (tp == Boolean || tp == Int) {
        if (tp == Boolean)
          return !BooleanValue;
        else
          return !(bool)IntValue;
      }
      return false;
    }
    return nullptr;
  }
  const bool operator==(const var &x) const {
    return opcall(L"==", x).BooleanValue;
  }
  const var opcall(const std::wstring &str_op, const var &value) const {
    if (str_op == L"==" || str_op == L"!=" || str_op == L">=" ||
        str_op == L"<=" || str_op == L">" || str_op == L"<") {
      Variable::var op;
      try {
        op = value.convert(tp);
      } catch (...) {
        return false;
      }
      if (str_op == L"==") {
        switch ((size_t)op.tp) {
          case Int: {
            return op.IntValue == IntValue;
          }
          case Null: {
            return true;
          }
          case Boolean: {
            return op.BooleanValue == BooleanValue;
          }
          case String: {
            return op.StringValue == StringValue;
          }
          case Array: {
            return ArrayValue == op.ArrayValue;
          }
          case Object: {
            return ObjectValue == op.ObjectValue;
          }
          case StmtBlock: {
            return StmtValue.value == op.StmtValue.value;
          }
          case Function: {
            return FunctionValue.args == op.FunctionValue.args &&
                   FunctionValue.block.value == op.FunctionValue.block.value;
          }
          default: {
            return false;
          }
        }
      } else if (str_op == L"!=") {
        return !opcall(L"==", value).BooleanValue;
      } else if (str_op == L">") {
        switch (op.tp) {
          case Int: {
            return IntValue > op.IntValue;
          }
          case String: {
            return StringValue > op.StringValue;
          }
          default: {
            return false;
          }
        }
        return false;
      } else if (str_op == L"<") {
        switch (op.tp) {
          case Int: {
            return IntValue < op.IntValue;
          }
          case String: {
            return StringValue < op.StringValue;
          }
          default: {
            return false;
          }
        }
        return false;
      } else if (str_op == L">=") {
        return (opcall(L">", value).BooleanValue) ||
               (opcall(L"==", op).BooleanValue);
      } else if (str_op == L"<=") {
        return (opcall(L"<", value).BooleanValue) ||
               (opcall(L"==", op).BooleanValue);
      }
    }
    if (str_op == L"+") {
      var ret;
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return IntValue + op.IntValue;
        }
        case String: {
          return StringValue + op.StringValue;
        }
        case Array: {
          ret.tp = Array;
          ret.ArrayValue = ArrayValue;
          for (size_t i = 0; i < op.ArrayValue.size(); i++) {
            ret.ArrayValue.push_back(op.ArrayValue[i]);
          }
          return ret;
        }
        case Object: {
          ret.tp = Object;
          ret.ObjectValue = ObjectValue;
          for (std::map<std::wstring, var>::const_iterator i =
                   op.ObjectValue.cbegin();
               i != op.ObjectValue.cend(); i++) {
            ret.ObjectValue[i->first] = i->second;
          }
          return ret;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"-") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return IntValue - op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"*") {
      switch (tp) {
        case Int: {
          const Variable::var &op = value.convert(tp);
          return IntValue * op.IntValue;
        }
        case String: {
          const var &temp = value.convert(Int);
          std::wstring w;
          for (size_t i = 0; i < temp.IntValue; i++) {
            w += StringValue;
          }
          return w;
        }
        case Array: {
          const var &temp = value.convert(Int);
          std::vector<var> w;
          for (size_t i = 0; i < temp.IntValue; i++) {
            for (size_t q = 0; q < ArrayValue.size(); q++) {
              w.push_back(ArrayValue[q]);
            }
          }
          return w;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"/") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          if (op.IntValue == 0) throw ExprErr(L"div by zero");  // div by zero
          return IntValue / op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"%") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          if (op.IntValue == 0) throw ExprErr(L"div by zero");  // div by zero
          return (int)IntValue % (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"&") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return (int)IntValue & (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"|") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return (int)IntValue | (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"^") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return (int)IntValue ^ (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"<<") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          if (op.IntValue >= 32)
            return (int)IntValue;
          else
            return (int)IntValue << (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L">>") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          if (op.IntValue >= 32)
            return (int)IntValue;
          else
            return (int)IntValue >> (int)op.IntValue;
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L">>>") {
      const Variable::var &op = value.convert(tp);
      switch (tp) {
        case Int: {
          return var((double)((size_t)IntValue >> (size_t)op.IntValue));
        }
        default: {
          throw nullptr;
        }
      }
    } else if (str_op == L"&&") {
      return (var(true).opcall(L"==", *this).BooleanValue) &&
             (var(true).opcall(L"==", value).BooleanValue);
    } else if (str_op == L"||") {
      return (var(true).opcall(L"==", *this).BooleanValue) ||
             (var(true).opcall(L"==", value).BooleanValue);
    }
    return nullptr;
  }
} var;
const var parse(const std::wstring &x, const bool isConst = false) {
  std::wstring p = clearnull(x);
  if (p == L"") return var(nullptr, isConst);
  try {
    if (isExpression(p) == false) throw nullptr;
  } catch (...) {
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
        std::stod(p);
      else
        std::stoi(p, 0, 0);
    } catch (...) {
      if (p == L"null") return var(nullptr, isConst);
      if (p == L"true" || p == L"false") return var(p == L"true", isConst);
      if ((p[0] == L'\"' && p[p.length() - 1] == L'\"') ||
          (p[0] == L'\'' && p[p.length() - 1] == L'\'')) {
        std::wstring tmp = p.substr(1, p.length() - 2), ret = L"";
        for (size_t i = 0; i < tmp.length(); i++) {
          if (tmp[i] == L'\\') {
            i++;
            switch (tmp[i]) {
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
              case L'e': {
                ret += L'\e';
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
                ret += L'?';
                break;
              }
            }
          } else
            ret += tmp[i];
        }
        return var(ret, isConst);
      }
      if (p.substr(0, 9) == L"function(" && p[p.length() - 1] == L'}') {
        std::vector<var::Func_temp::Arg_Item> arg;
        std::wstring name_temp, value_temp;
        std::wstring cont_temp;
        size_t i;
        for (i = 9; i < p.length(); i++) {
          if (p[i] == ')') break;
          if (p[i] == L',') {
            arg.push_back(var::Func_temp::Arg_Item(name_temp, value_temp));
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
            arg.push_back(var::Func_temp::Arg_Item(name_temp, value_temp));
            name_temp.clear(), value_temp.clear();
          } else
            name_temp += p[i];
        }
        if (name_temp != L"") {
          arg.push_back(var::Func_temp::Arg_Item(name_temp, value_temp));
          name_temp.clear(), value_temp.clear();
        }
        i++;
        if (p[i] != L'{') throw SyntaxErr(L"Brace does not match");
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
        return var::Func_temp(arg, var::Stmt_temp(code_split(cont_temp)));
      }
      if ((p[0] == L'{' && p[p.length() - 1] == L'}') ||
          (p[0] == L'[' && p[p.length() - 1] == L']')) {
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
                  var::Stmt_temp(code_split(p.substr(1, p.length() - 2))),
                  isConst);
            var t = parse(temp2[0]);
            if (t.tp != String) throw ExprErr(L"Object's key must be a string");
            ret[parse(temp2[0]).StringValue] = parse(temp2[1], false);
          } else
            ret2.push_back(parse(temp[i], false));
        }
        if (isobject)
          return var(ret, isConst);
        else
          return var(ret2, isConst);
      }
      return var(genExpression(splitExpression(p)), isConst);
    }
    if (p.find_first_of('.') == std::wstring::npos &&
        p.find_first_of('e') == std::wstring::npos)
      return var(std::stoi(p, 0, 0), isConst);
    else
      return var(std::stod(p), isConst);
  }
  return var(genExpression(splitExpression(p)), isConst);
}
const var instanceOf(const var_tp &i) {
  switch (i) {
    case Int:
      return 0;
    case Boolean:
      return false;
    case String:
      return L"";
    case Array:
      return std::vector<var>();
    case Object:
      return std::map<std::wstring, var>();
    case Null:
      return Variable::var();
    case Function:
      return var::Func_temp();
    default:
      return nullptr;
  }
}
};  // namespace Variable
#endif
