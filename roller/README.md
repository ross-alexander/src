---
title: Simple dice rolling using lex/yacc
author: Ross Alexander
date: December 15, 2023
---

# Precis

Use flex/bison to parse a string to eval a dice rolling string, for
example "4d10+2".

- Changed from having operators as subclasses to converting them to
functions.

- Evaluation changed from inside first to outside first, effectively
giving lazy evaluation.  This is needed for the repeat function,
since we want to evaluate `die` multiple times.

