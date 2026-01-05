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
        @import<markdown> from "sfs" #! Импортируем библиотеку "Markdown" из sfs(/usr/bin/sfs/).
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
