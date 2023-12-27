# Description

jlog is a combination C++/lua program for analysing syslog output from
the Juniper SRX firewalls.  It uses flex/bison to do the tokenization
and parsing.  It could have been done with either a hand coded regex
tokenizer and a simple parser but using flex/bison works.

Individual lines are tokenized and collected into a nested lua table,
which is passed to a function called update.  At the end of the run
the fuction dump is called (if it exists).

# ChangeLog

## 2020-08-06

Updated to use lua-5.4.0 (no changes required).

Updated to use jsoncons-0.155.0.

Fix bug in collector.cc where, if the update function doesn't exist,
the collect table was being left on the stack, causing it to overflow.
