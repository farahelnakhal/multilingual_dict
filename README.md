# Translator: Multilingual Dictionary

A hash-table-backed multilingual dictionary that stores English words and
their translations across several languages, with support for large bulk
imports, collision tracking, and lazy deletion.

## Features

- Custom hash table implementation (no STL hash map) using linear or
  quadratic probing for collision resolution
- Configurable hash function (polynomial hashing, cyclic shift, division/MAD)
- Case-insensitive words and commands
- Import dictionary files in the format `word:meaning1;meaning2;...`
- Add, find, and delete words, translations, or individual meanings
- Lazy deletion — removed entries are marked, not physically removed
- Export a language's entries back out to a file in the original format
- Reports number of comparisons made during lookups, and collision stats
  for the hash table

## Commands

| Command | Description |
|---|---|
| `find <word>` | search a word and print all its meanings |
| `import <path>` | import a dictionary file |
| `add <word:meaning(s):language>` | add a word/meaning to the dictionary |
| `delTranslation <word:language>` | delete a word's translation in one language |
| `delMeaning <word:meaning:language>` | delete a single meaning |
| `delWord <word>` | delete a word and all its translations |
| `export <language:filename>` | export a language's entries to a file |
| `exit` | quit the program |

## Build

```bash
make
./translator
```

On startup, the program automatically imports the default English–German
dictionary file.

## Structure

- `hashtable.h` / `hashtable.cpp`: hash table, hashing functions, probing
- `main.cpp`: command loop
- `vector.h`: optional custom vector (STL vector is also permitted)
- `makefile`
