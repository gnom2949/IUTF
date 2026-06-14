(function(Prism) {

    // Comments
    Prism.languages.iutf = {
        'comment': {
            pattern: /\/\/.*|\/\*[\s\S]*?\*\/|#!.*/,
            greedy: true,
            alias: 'comment'
        },

        //keys
        'key': {
          pattern: /\b[a-zA-Z_][a-zA-Z0-9_]*\s*(?=:)/,
          alias: 'property'
        },

        // Strings
        'string': {
            pattern: /"([^"\\]|\\.)*"/,
            greedy: true,
            alias: 'string'
        },

        //extensions
        'extension': {
          pattern: /\.[a-zA-Z0-9]+/,
          alias: 'important'
        },

        //import kw
        'import': {
          pattern: /@import\s+\"([^\"]+)\"\s+from\s+([a-zA-Z_][a-zA-Z0-9_]*)/,
          inside: {
            'keyword': /@import|from/,
            'string': /"[^"]+"/,
            'import-name': {
              pattern: /from\s+([a-zA-Z_][a-zA-Z0-9_]*)/,
              lookbehind: true,
              alias: 'namespace'
            }
          },
          alias: 'important'
        },

        //version
        'version': {
          pattern: /\b\d+\.\d+\.\d+\b/,
          alias: 'number'
        },


        // characters
        'char': {
            pattern: /'([^'\\]|\\.)*'/,
            greedy: true,
            alias: 'character'
        },

        // BigString kw
        'bigstring': {
            pattern: /BigString\[[^\]]*\]/,
            inside: {
                'keyword': /BigString/,
                'punctuation': /[\[\]]/
            },
            alias: 'bigstring'
        },

        // numbers
        'number': [
            {
                pattern: /\b\d+\.\d+\b/,
                alias: 'float'
            },
            {
                pattern: /\b\d+L\b/,
                alias: 'long'
            },
            {
                pattern: /\b\d+\b/,
                alias: 'integer'
            }
        ],

        // kws
        'keyword': [
            {
                pattern: /\b(?:EOF|ERROR|NULL|TRUE|FALSE)\b/i,
                alias: 'constant'
            },
            {
                pattern: /\b(?:TOK)_?[A-Z_]+\b/,
            }
        ],


        // ops and punctuation
        'punctuation': [
            {
                pattern: /[{}()\[\]]/,
                alias: 'bracket'
            },
            {
                pattern: /[,:|=]/,
                alias: 'operator'
            }
        ],

    };

    // C lang support
    Prism.languages.insertBefore('iutf', 'comment', {
        'directive': {
            pattern: /#!\s*\w+/,
            alias: 'important'
        }
    });

    Prism.languages.insertBefore ('iutf', 'key', {
      'namespace': {
        pattern: /[a-zA-Z_][a-zA-Z0-9_]*(?=\s*\{)/,
        alias: 'class-name'
      }
    });
})(Prism);
