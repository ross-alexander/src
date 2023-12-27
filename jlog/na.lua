local na = require "NetAddr"

a = na.new('127.0.0.1')
b = na.new('127.0.0.1')
print(b, b:resolve())
print(a:resolve())
print(a < b)
print(a == b)
