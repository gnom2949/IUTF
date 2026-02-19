#ifndef PIUTF_EXTENSION_H
#define PIUTF_EXTENSION_H

#include "php.h"
#include "../../includes/colors.h"
#include "../../includes/iutf-api.h"
#include "../../includes/iutf-ast.h"
#include "../../includes/iutf-import.h"
#include "../../includes/iutf-lexer.h"
#include "../../includes/iutf-parser.h"
#include "../../includes/iutf-stream.h"
#include "../../includes/iutf-validator.h"

extern zend_module_entry iutf_module_entry;
#define php_ext_piutf_ptr &iutf_module_entry

#define PIUTF_EXTENSION_VERSION "0.1.3"

#endif
