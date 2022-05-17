/*
L++ was under the MIT license.
copyright(c) 2022 FurDevsCN.
*/
#ifndef _PARSE_MODULE
#define _PARSE_MODULE
#include <string>
/*
Lpp命名空间存储了L++基本解析器。
*/
namespace Lpp {
/*
struct Lpp;
Lpp类用于解析原字符串（单条语句，不允许空白字符）并存储解析后的结果。
*/
typedef struct Lpp {
  // 关键字名称（或者表达式的一部分）。
  std::wstring name;
  // 传递给关键字的参数（或者表达式的另一部分）。
  std::wstring args;
  // 注意：您应当使用toString()方法来获得完整的表达式。
  /*
  explicit Lpp(const std::wstring &p);
  解析原串p并构造存储类。
  */
  explicit Lpp(const std::wstring &p) {
    size_t a = 0, j = 0, z = 0;
    for (size_t i = 0; i < p.length(); i++) {
      if (p[i] == L'\\')
        z = !z;
      else if (p[i] == L'\"' && !z) {
        if (a == 0 || a == 1) a = !a;
      } else if (p[i] == L'\'' && !z) {
        if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
      } else
        z = 0;
      if (p[i] == L'\n' || p[i] == L'\t') continue;
      if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
        j++;
      else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
        j--;
      if (p[i] == L'(' && a == 0 && j == 1) break;
      if (p[i] == L' ' && a == 0 && j == 0) {
        name = p.substr(0, i);
        args = p.substr(i + 1);
        return;
      }
    }
    a = j = z = 0;
    for (size_t i = p.length(); i--;) {
      if (p[i] == L'\\')
        z = !z;
      else if (p[i] == L'\"' && !z) {
        if (a == 0 || a == 1) a = !a;
      } else if (p[i] == L'\'' && !z) {
        if (a == 0 || a == 2) a = ((!a) == 1 ? 2 : 0);
      } else
        z = 0;
      if (p[i] == L'\n' || p[i] == L'\t') continue;
      if ((p[i] == L'(' || p[i] == L'{' || p[i] == L'[') && a == 0)
        j--;
      else if ((p[i] == L')' || p[i] == L'}' || p[i] == L']') && a == 0)
        j++;
      if ((p[i] == L'{' || p[i] == L'(') && a == 0 && j == 0) {
        if (p[i] != L'{' || i < 1 || p[i - 1] != L')') {
          name = p.substr(0, i);
          args = p.substr(i);
          return;
        }
      }
    }
    name = p;
  }
  /*
  Lpp(const std::wstring &__name, const Args &__args);
  由已经解析的调用名__name和参数__args来构造存储类。
  */
  Lpp(const std::wstring &__name, const std::wstring &__args)
      : name(__name), args(__args) {}
  Lpp() {}
  /*
  std::wstring toString() const;
  将解析完毕的内容再次转换回原串。
  */
  std::wstring toString() const {
    return name + ((args.length() == 0 && name != L"") ? L"" : L" ") + args;
  }
  /*
  bool operator==(const Lpp &x) const;
  操作符重载。比较两个L++语句是否相同。
  */
  bool operator==(const Lpp &x) const {
    return x.name == name && x.args == args;
  }
} Lpp;
}  // namespace Lpp
#endif
