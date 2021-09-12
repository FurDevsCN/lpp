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
typedef class ConvFail {
  std::wstring _err;

 public:
  const std::wstring what() const { return _err; }
  ConvFail() {}
  ConvFail(const std::wstring &err) { _err = err; }
} ConvFail;
typedef class ExprErr {
  std::wstring _err;

 public:
  const std::wstring what() const { return _err; }
  ExprErr() {}
  ExprErr(const std::wstring &err) { _err = err; }
} ExprErr;
typedef class SyntaxErr {
  std::wstring _err;

 public:
  const std::wstring what() const { return _err; }
  SyntaxErr() {}
  SyntaxErr(const std::wstring &err) { _err = err; }
} SyntaxErr;
size_t colon_judge(char now, size_t nowa, bool z) {
  if (now == L'\"' && !z) {
    if (nowa == 0 || nowa == 1) return !nowa;
  } else if (now == L'\'' && !z) {
    if (nowa == 0 || nowa == 2) return (!nowa) == 1 ? 2 : 0;
  }
  return nowa;
}
int get_op_priority(const std::wstring &op) {
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
const bool isExpression(std::wstring p) {
  for (size_t i = 0, j = 0, a = 0, z = 0; i < p.length(); i++) {
    if (p[i] == L'\\')
      z = !z;
    else if (p[i] == L'\"' || p[i] == L'\'')
      a = colon_judge(p[i], a, z);
    else
      z = 0;
    if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
      j++;
    else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
      j--;
    else if (get_op_priority(std::wstring(1, p[i])) != -1 && i > 0 &&
             p[i - 1] != L'e' && p[i - 1] != L'E' && a == 0 && j == 0)
      return true;
  }
  return false;
}
std::wstring castExpression(const std::vector<std::wstring> &p) {
  std::wstring ret;
  std::vector<std::wstring> temp;
  std::wstring last, temp2;
  std::wstring op;
  for (std::vector<std::wstring>::const_reverse_iterator i = p.crbegin();
       i != p.crend(); i++) {
    op = (*i);
    if (get_op_priority(op) != -1 && op != L"!" && op != L"~") {
      temp2 = temp[temp.size() - 1];
      temp.pop_back();
      temp2 += op;
      temp2 += temp[temp.size() - 1];
      temp.pop_back();
      if (get_op_priority(op) >= get_op_priority(last)) {
        temp.push_back(L"(" + temp2 + L")");
      } else {
        temp.push_back(temp2);
      }
      last = op;
    } else if (op == L"!") {
      temp2 = op + temp[temp.size() - 1];
      temp.pop_back();
      temp.push_back(temp2);
      last = op;
    } else
      temp.push_back(op);
  }
  while (!temp.empty()) {
    if (temp.size() > 1 && get_op_priority(temp[temp.size() - 2]) != -1) {
      ret = temp[temp.size() - 1] + ret;
    } else {
      ret += temp[temp.size() - 1];
    }
    temp.pop_back();
  }
  return ret;
}
std::vector<std::wstring> genExpression(const std::vector<std::wstring> &p) {
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
        } else {
          ret.push_back(temp[temp.size() - 1]);
          temp.pop_back();
        }
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
      case L'\\': {
        z = !z;
        temp += p[i];
        break;
      }
      case L'\'':
      case L'\"': {
        a = colon_judge(p[i], a, z);
        temp += p[i];
        break;
      }
      case L'>':
      case L'=':
      case L'&':
      case L'|':
      case L'<': {
        if (a == 0 && f == 0 &&
            (i == 0 || p[i - 1] == p[i] || p[i - 1] == L'>' ||
             p[i - 1] == L'<' || x)) {
          if (temp != L"") {
            if (x) {
              if (ret.empty())
                throw ExprErr(L"assert ret.empty()!=true failed");
              ret[ret.size() - 1] += temp;
            } else
              ret.push_back(temp);
            temp = L"";
          }
          if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
          ret[ret.size() - 1] += p[i];
          if (p[i] == L'<' || p[i] == L'>') x = !x;
        } else if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          ret.push_back(std::wstring(1, p[i]));
          if (p[i] == L'&' || p[i] == L'|') x = true;
        } else
          temp += p[i];
        break;
      }
      case L'-': {
        if (a == 0 && f == 0 &&
            (i == 0 || p[i - 1] == L'e' || p[i - 1] == L'E'))
          temp += p[i];  // ret[ret.size()-1]+=p[i];
        else if (x) {
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
      case L'+':
      case L'*':
      case L'/':
      case L',':
      case L'%': {
        if (a == 0 && f == 0) {
          if (temp != L"") {
            ret.push_back(temp);
            temp = L"";
          }
          if (ret.empty()) throw ExprErr(L"assert ret.empty()!=true failed");
          if (p[i] == L'+' &&
              ret[ret.size() - 1][ret[ret.size() - 1].length() - 1] == L'+') {
            ret[ret.size() - 1] += L"+";
            break;
          }
          x = true;
          ret.push_back(std::wstring(1, p[i]));
        } else
          temp += p[i];
        break;
      }
      case L'(': {
        if (a == 0 && f == 0) {
          if (temp != L"") ret.push_back(temp), temp = L"";
          f++;
          ret.push_back(L"(");
        } else
          temp += L"(";
        break;
      }
      case L')': {
        if (a == 0 && f == 1) {
          if (temp != L"") ret.push_back(temp), temp = L"";
          f--;
          ret.push_back(L")");
        } else
          temp += L")";
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
  return ret;
}
const int Hex2Dec(const std::wstring &m) {
  int l;
  l = std::stoi(m, 0, 16);
  return l;
}
const std::wstring Unicode2String(const std::wstring &str) {
  if (str == L"") return L"";
  return std::wstring(1, Hex2Dec(L"0x" + str));
}
std::wstring String2WString(const std::string &s) {
  std::wstring t(s.length(), L'\0');
  size_t q=mbstowcs(&t[0], &s[0], s.length());
  if((int)q==-1)return std::wstring(s.length(),L'?');
  else t.resize(q);
  return t;
}
std::string WString2String(const std::wstring &s) {
  std::string t(s.length()*8, L'\0');
  size_t q=wcstombs(&t[0], &s[0], t.length());
  if((int)q==-1)return std::string(t.length(),L'?');
  else t.resize(q);
  return t;
}
const std::wstring clearnull(const std::wstring &x) {
  std::wstring tmp;
  for (size_t i = 0, a = 0, z = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' || x[i] == L'\'')
      a = colon_judge(x[i], a, z);
    else
      z = 0;
    if ((x[i] == L'\r' || x[i] == L'\n' || x[i] == L'\t') && a == 0)
      continue;
    else if (x[i] == L' ' && a == 0 &&
             (i <= 0 || (tmp[i - 1] == L'(' || tmp[i - 1] == L'[' ||
                         tmp[i - 1] == L'{' || tmp[i - 1] == L' ')) &&
             (i <= 0 || !((tmp[i - 1] >= L'a' && tmp[i - 1] <= L'z') ||
                          (tmp[i - 1] >= L'A' && tmp[i - 1] <= L'Z') ||
                          tmp[i - 1] == L'_' ||
                          (tmp[i - 1] >= L'0' && tmp[i - 1] <= L'9')))) {
      continue;
    } else
      tmp += x[i];
  }
  return tmp;
}
const std::vector<std::wstring> code_split(const std::wstring &x) {
  std::wstring p;
  for (size_t i = 0, a = 0, z = 0; i < x.length(); i++) {
    if (x[i] == L'\\')
      z = !z;
    else if (x[i] == L'\"' || x[i] == L'\'')
      a = colon_judge(x[i], a, z);
    else
      z = 0;
    if (x[i] == L'\n' && a == 0 && i >= 1 && x[i - 1] != L'[' &&
        x[i - 1] != L'(' && x[i - 1] != L'{')
      p += L';';  // p[i]=';';else p[i]=' L';
    if (x[i] != L'\n') p += x[i];
  }
  std::vector<std::wstring> ret;
  std::wstring temp;
  for (size_t i = 0, j = 0, a = 0, z = 0; i < p.length(); i++) {
    if (p[i] == L'\\')
      z = !z;
    else if (p[i] == L'\"' || p[i] == L'\'')
      a = colon_judge(p[i], a, z);
    else
      z = 0;
    if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
      j++;
    else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
      j--;
    if (p[i] == L';' && a == 0 && j == 0)
      ret.push_back(temp), temp = L"";
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
  Expression = 7,
} var_tp;
const std::wstring getTypeStr(const var_tp &x) {
  switch (x) {
    case Null:
      return L"null";
    case Int:
      return L"int";
    case Boolean:
      return L"boolean";
    case String:
      return L"string";
    case Array:
      return L"array";
    case Object:
      return L"object";
    case Function:
      return L"function";
    case Expression:
      return L"null";  // no use?
  }
  return L"null";
}
const var_tp getStrType(const std::wstring &x) {
  if (x == L"null") return Null;
  if (x == L"int") return Int;
  if (x == L"boolean") return Boolean;
  if (x == L"string") return String;
  if (x == L"array") return Array;
  if (x == L"Object") return Object;
  if (x == L"function") return Function;
  // if(x=="expression")return Expression;
  return Null;
}
typedef struct Fn_temp {
  std::vector<std::wstring> value;
  Fn_temp() {}
  Fn_temp(std::vector<std::wstring> x) { value = x; }
} Fn_temp;
typedef class var {
  bool needtoRemove;

 public:
  bool isConst;
  std::wstring StringValue;
  std::vector<var> ArrayValue;
  std::map<std::wstring, var> ObjectValue;
  Fn_temp FunctionValue;
  std::vector<std::wstring> ExpressionValue;
  var() {
    tp = Null;
    isConst = true;
    needtoRemove = false;
  }
  var(bool x, bool c = true) {
    tp = Boolean;
    BooleanValue = x;
    isConst = c;
    needtoRemove = false;
  }
  var(std::nullptr_t x, bool c = true) {
    tp = Null;
    isConst = c;
    needtoRemove = false;
  }
  var(const int &x, bool c = true) {
    IntValue = x, tp = Int;
    isConst = c;
    needtoRemove = false;
  }
  var(const double &x, bool c = true) {
    IntValue = x, tp = Int;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::wstring &x, bool c = true) {
    StringValue = x, tp = String;
    isConst = c;
    needtoRemove = false;
  }
  var(const wchar_t *const x, bool c = true) {
    StringValue = std::wstring(x), tp = String;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::vector<var> &x, bool c = true) {
    ArrayValue = x, tp = Array;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::map<std::wstring, var> &x, bool c = true) {
    ObjectValue = x, tp = Object;
    isConst = c;
    needtoRemove = false;
  }
  var(const Fn_temp &x, bool c = true) {
    FunctionValue = x, tp = Function;
    isConst = c;
    needtoRemove = false;
  }
  var(const std::vector<std::wstring> &x, bool c = true) {
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
          tmp += var(it->first).toString() + L":";
          tmp += it->second.toString();
          if ((++std::map<std::wstring, var>::const_iterator(it)) !=
              ObjectValue.cend())
            tmp += L",";
        }
        return tmp + L"}";
      }
      case Function: {
        tmp = L"{";
        for (size_t i = 0; i < FunctionValue.value.size(); i++) {
          if (FunctionValue.value[i] != L"")
            tmp += FunctionValue.value[i] + L";";
        }
        if (tmp == L"{") tmp += L";";
        return tmp + L"}";
      }
      case Expression: {
        return castExpression(ExpressionValue);
      }
    }
    return L"null";
  }
  const var convert(const var_tp &type) const {
    if (tp == type) return *this;
    if (type == String) return var(toString());
    switch (tp) {
      case Int: {
        if (type == Boolean) return var((bool)IntValue);
        break;
      }
      case Boolean: {
        if (type == Int) return var((bool)IntValue);
        break;
      }
      case String: {
        if (type == Array) {
          std::vector<var> ret;
          for (size_t i = 0; i < StringValue.length(); i++) {
            ret.push_back(var(std::wstring(1, StringValue[i])));
          }
          return var(ret);
        }
        break;
      }
      case Array: {
        if (type == Object) {
          std::map<std::wstring, var> ret;
          for (size_t i = 0; i < ArrayValue.size(); i++) {
            ret[std::to_wstring(i)] = ArrayValue[i];
          }
          return var(ret);
        }
        break;
      }
      default:
        throw nullptr;
    }
    throw ConvFail(getTypeStr(tp) + L"->" + getTypeStr(type));
  }
  const var operator+(const var &opx) const {
    var ret;
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var(IntValue + op.IntValue);
      }
      case String: {
        return var(StringValue + op.StringValue);
      }
      case Array: {
        ret.tp = Array;
        ret.ArrayValue = ArrayValue;
        for (size_t i = 0; i < op.ArrayValue.size(); i++) {
          ret.ArrayValue.push_back(op.ArrayValue[i]);
        }
        return ret;
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator-(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var(IntValue - op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator*(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var(IntValue * op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator/(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        if (op.IntValue == 0) throw nullptr;  // div by zero
        return var(IntValue / op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator%(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        if (op.IntValue == 0) throw nullptr;  // div by zero
        return var((int)IntValue % (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator&(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var((int)IntValue & (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator|(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var((int)IntValue | (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator^(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var((int)IntValue ^ (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator~() const {
    switch (tp) {
      case Int: {
        return var(~(int)IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var operator-() const {
    switch (tp) {
      case Int: {
        return var(-IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var leftmove(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        if (op.IntValue >= 32)
          return var((int)IntValue);
        else
          return var((int)IntValue << (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var rightmove_signed(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        if (op.IntValue >= 32)
          return var((int)IntValue);
        else
          return var((int)IntValue >> (int)op.IntValue);
      }
      default: {
        throw nullptr;
      }
    }
  }
  const var rightmove_unsigned(const var &opx) const {
    const var op = opx.convert(tp);
    switch (tp) {
      case Int: {
        return var(double((unsigned int)IntValue >> (unsigned int)op.IntValue));
      }
      default: {
        throw nullptr;
      }
    }
  }
  const bool operator==(const var &opx) const {
    try {
      opx.convert(tp);
    } catch (...) {
      return false;
    }
    const var op = opx.convert(tp);
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
      case Function: {
        return FunctionValue.value == op.FunctionValue.value;
      }
    }
    return false;
  }
  const bool operator!=(const var &op) const { return !operator==(op); }
  const bool operator>(const var &opx) const {
    try {
      opx.convert(tp);
    } catch (...) {
      return false;
    }
    const var op = opx.convert(tp);
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
  }
  const bool operator<(const var &op) const {
    return (!operator>(op)) && (!operator==(op));
  }
  const bool operator>=(const var &op) const {
    return operator>(op) || operator==(op);
  }
  const bool operator<=(const var &op) const {
    return operator<(op) || operator==(op);
  }
  const bool operator&&(const var &op) const {
    return (operator==(var(true))) && (var(true) == op);
  }
  const bool operator||(const var &op) const {
    return (operator==(var(true))) || (var(true) == op);
  }
  const bool operator!() const {
    if (tp == Boolean || tp == Int) {
      if (tp == Boolean)
        return !BooleanValue;
      else
        return !(bool)IntValue;
    }
    return false;
  }
} var;
const var parse(const std::wstring &x, const bool &isConst = false) {
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
              case L'n': {
                ret += L'\n';
                break;
              }
              case L'e': {
                ret += L'\e';
                break;
              }
              case L'u': {
                ret += Unicode2String(tmp.substr(i + 1, 4));
                i += 4;
                break;
              }
            }
          } else
            ret += tmp[i];
        }
        return var(ret, isConst);
      }
      if ((p[0] == L'{' || p[0] == L'[') &&
          (p[p.length() - 1] == L'}' || p[p.length() - 1] == L']')) {
        std::wstring key = L"", value = L"";
        std::map<std::wstring, var> ret;
        std::vector<var> ret2;
        bool isobject = (p[0] == L'{');
        for (size_t i = 1, a = 0; i < p.length() - 1; i++) {
          if (isobject) {
            if (p[i] != L'\"' && p[i] != L'\'')
              return var(Fn_temp(code_split(p.substr(1, p.length() - 2))),
                         isConst);
            for (bool z = false; i < p.length() - 1; i++) {
              if (p[i] == L'\\')
                z = !z;
              else if (p[i] == L'\"' || p[i] == L'\'')
                a = colon_judge(p[i], a, z);
              else
                z = 0;
              if ((p[i] == L'\"' || p[i] == L'\'') &&
                  (p[i - 1] != L'{' || p[i - 1] != L',') && a == 0) {
                key += p[i++];
                break;
              }
              key += p[i];
            }
            if (p[i] != L':')
              return var(Fn_temp(code_split(p.substr(1, p.length() - 2))),
                         isConst);
            i++;  //: token
          }
          for (size_t j = 0, z = 0; i < p.length() - 1; i++) {
            if (p[i] == L'\\')
              z = !z;
            else if (p[i] == L'\"' || p[i] == L'\'')
              a = colon_judge(p[i], a, z);
            else
              z = 0;
            if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
              j++;
            else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
              j--;
            if (p[i] == L',' && j == 0 && a == 0)
              break;
            else
              value += p[i];
          }
          if (isobject) ret[parse(key).StringValue] = parse(value);
          if (!isobject) {
            ret2.push_back(parse(value, false));
          } else
            ret2.push_back(parse(value));
          key = L"";
          value = L"";
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
};  // namespace Variable
#endif
