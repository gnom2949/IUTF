# IUTF (Int Unified Text Format)
![IUTF LOGO](../src/imgs/IUTFLogo.png)
**IUTF** — это формат конфигурации и сериализации данных, разработанный как альтернатива JSON, YAML и TOML. Он поддерживает ветки (branches), комментарии, многострочные строки, числа, булевы значения, массивы и расширения.

## Структура

IUTF использует **ветки** (`branch`) — иерархические структуры, аналогичные JSON-объектам, но с более гибким синтаксисом.

### Корневая ветка (main branch)

```iutf
iutf:init:main {
  // содержимое
}
```
## Типы данных
| тип | Пример | Описание |
|---|---|---|
| string | "Hello"  | Строка  |
| Integer(int) | 1853 | Целое число |
| float | 3.18 | Число с плавающей точкой |
| long | 1853 | Длинное целое число |
| character(char) | 'X' | Символ |
| boolean | true/false | Логическое значение |
| null | null | Отсутствие значения |
| array | [1, 2, 3] | Массив данных |
| branch | { key: value } | Объект/ветка |
| bigstring | BigString[ ... ] | Многострочная строка |
| pipestring | | ... | | Многострочная строка |

## Синтаксис
**Комментарии**
```
#! Однострочный комментарий
// Однострочный комментарий (C-Style)
/* 
    Многострочный
    Комментарий
*/
```

**Ключ-значение**
```
key: value
```

**Ветки**
```
config { #! инициализация ветки
    debug: true #! ветка запущена(если значений нет то ветка помечается как неактивная(null))
    port: 8053
} #! конец ветки 
```

**Массивы** 
```
    tags: [ "dev", "gopher", "debug" ]  
```

**Многострочные строки**
1. BigString
```
    license: BigString[
        Copyright(C) 2026 example LLC
        All rights reserved.
```
2. PipeString -> |
```
readme: |
    This is a IUTF documentation!
    Give me money, please.
|
```

**Числа**
В iutf существуют нескольно типов чисел:
1. Integer->int :: Целое число, например 134
2. float :: Число с плавающей точкой, например 148.98
3. Long :: Длинное целое число, например 137L

**Символы**
В iutf символы содержат в себе значение char(Character),
обозначаются символы такими знаками='', например:
grade: 'G'

**Пример файла IUTF**
```
iutf:init:main { #! инициализация iutf, обязательная ветка.
    #! hello John!
    title: "GONOME"
    version: 50
    versionL: 50L
    versionF: 49.9999999999999999999999
    char: 'G'
    authors: ["jack", "bob"]
    meta { #! новая ветка
        CI:Status
        created: [tms]
        public: true
        tags: [dev, gtk, glib, debug]
    }
    license: BigString[
        Copyright(C) 2026 goople LLC
        All rights reserved.
    ] 
    readme: |
        Hello!
        This is a demo file of iutf.
    |
}
```

## Расширения (UTEXT)
UTEXT - Unified Text EXTension - формат расширения для iutf с поддержкой пользовательских типов, функций, тегов и локализации.

```
iutf:extension:colors {
    provides: ["col", "render_color"]
    type col {
        validator: is_valid_color_code
    }
}
```
iutf:extension -> инициализация типа "расширение", colors -> имя расширения.

## Валидация
IUTF поддерживает валидацию через ast и пользовательские правила.

## Подключение расширений в .iutf файле
Чтобы использовать расширения в IUTF файле, нужно подключить их через ветку deps находящуюся в ветке main.

```
iutf:init:main {
    deps::init { #! Инициализация ветки deps
        @import<colors> from "utext://colors.utext" #! Импортируем библиотеку colors из локальной директории, при желании можно указать любую другую директорию.
        @import<markdown> from "sfs" #! Импортируем библиотеку "Markdown" из sfs(/usr/include/sfs/).
    }

    title: "My Project"
    version: 1.0

    #! Используем тип из расширения
    primary_color[col]: 'R'
    readme: |
        # My Project
        Built with IUTF.
    |
}
```

## Интеграция с другими языками

IUTF разработан для **простой интеграции** с различными языками программирования. Ниже приведены примеры использования IUTF в **Vala**, **Python**, **Rust**, **Go** и других.

---

### Vala

IUTF поддерживает **Vala VAPI-файл** (`IUTF.vapi`), который позволяет использовать IUTF-парсер в проектах GNOME и других Vala-приложениях.
Также есть поддержка C позволяющая интегрировать **IUTF** в ваши C проекты

#### Установка

Помести `IUTF.vapi` в `vapi/` или `/usr/share/vala/vapi/`.

#### Пример:

```vala
using IUTF;

int main() 
{
    var lexer = new Lexer("iutf:init:main { title: \"Hello\" }");
    var token = lexer.next();

    while (token.type != TokenType.EOF) {
        stdout.printf("[%d:%d] %s\n", token.line, token.col, token.type.to_string());
        token = lexer.next();
    }

    return 0;
}
```

#### Компиляция
```shell
valac --pkg glib-2.0 main.vala -X -Isrc/includes -X src/core/iutf-lexer.c -X src/core/iutf-ast.c -X src/core/iutf-parser.c -X src/core/iutf-validator.c
```

### Python

**IUTF** можно использовать в Python через **ctypes**, если скомпилировать его как `.so`.

#### Пример
```python
import ctypes

lib = ctypes.CDLL('./libiutf.so')

lib.iutf_lexer_new.argtypes = [ctypes.c_char_p]
lib.iutf_lexer_next.restype = ctypes.c_void_p    
```

### Rust

**IUTF** можно использовать в Rust через bindgen и `cc` crate.

#### Пример

```rust
use std::env;
use std::path::PathBuf;

fn main()
{
    println!("cargo:rerun-if-changed=includes/");
    println!("cargo:rerun-if-changed=src/core/");

    cc::Build::new()
        .file("src/core/iutf-lexer.c")
        .file("src/core/iutf-ast.c")
        .file("src/core/iutf-parser.c")
        .file("src/core/iutf-validator.c")
        .include("includes/")
        .compile("libiutf.a");

    let bindings = bindgen::Builder::default()
        .header("includes/iutf-lexer.h")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs")).expect("Couldn't write bindings");
}
```

### Go

**IUTF** можно использовать в Go через **CGO**

#### Пример
```
package main

/*
#cgo CFLAGS: -Iincludes
#cgo LDFLAGS: -L. -liutf
#include "iutf-lexer.h"
*/
import "C"
import "fmt"

func main()
{
    lexer := C.iutf_lexer_new(C.CString("iutf:init:main { title: \"Hello\" }))
    defer C.iutf_lexer_corrupt(lexer)

    for {
        token := C.iutf_lexer_next(lexer)
        if token.type == C.IUTF_TOK_EOF {
            break
        }
        fmt.Printf("Token: %s\n", C.GoString(C.iutf_token_type_to_string(token.type)))
    }
}
```

## IUTF API
API который появился вместе с версией 0.4 позволяющий удобно создавать структуры

#### Пример
```
IutfNode* table = iutf_new_branch();
iutf_branch_add(table, "StatVal", iutf_new_int(42));
iutf_branch_add(table, "enabled", iutf_new_bool(1));

char* output = debug_print_string(table);
printf("%s\n", output);
free(output);
iutf_node_free(table);
```

### Список функций

|Имя функции|Что делает|
|-------|------|
| iutf_new_branch | Создает новую ветку с указанным значением |
| to_branch | Создает значение в указанной ветке с указанным значением |
| iutf_new_str | Создает новую строку с указанным значением |
| iutf_new_int | Создает новое целое указанное число |
| iutf_new_float | Создает новое указанное число с плавающей точкой | 
| iutf_new_long | Создает новое длинное указанное число |
| iutf_new_char | Создает новый символ |
| iutf_new_bool | Создает новое булево значение |
| iutf_new_null | Создает новое значение 'NULL' |
| iutf_new_array | Создает новый массив без значения |
| add_to_array | Добавляет указанный элемент в указанный массив |
| iutf_new_BigString | Создает новую строку в формате BigString(BigString[]) |
| iutf_new_PipeStr | Создает новую строку в формате PipeString |
| debug_print_string | Функция чтобы вам не приходилось вручную передавать технические параметры (уровень отступа, указатели на размер буфера) при каждом вызове |
| debug_print_recursive | Функция помощник для debug_print_string, обходит дерево и превращает каждую ноду в текст |
