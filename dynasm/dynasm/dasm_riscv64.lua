------------------------------------------------------------------------------
-- DynASM ARM64 module.
--
-- Copyright (C) 2005-2021 Mike Pall. All rights reserved.
-- See dynasm.lua for full copyright notice.
------------------------------------------------------------------------------

----------------------------------------------------------------------
--
-- RISCV64 copied from ARM64
--
-- Ross Alexander 3.12.2021
--
--------------------------------------------------------------------------------

-- Module information:
local _info = {
  arch =	"riscv64",
  description =	"DynASM RISCV64 module",
  version =	"1.5.0",
  vernum =	 10500,
  release =	"2021-12-03",
  author =	"Ross Alexander",
  license =	"MIT",
}

-- Exported glue functions for the arch-specific module.
local _M = { _info = _info }

-- Cache library functions.
local type, tonumber, pairs, ipairs = type, tonumber, pairs, ipairs
local assert, setmetatable, rawget = assert, setmetatable, rawget
local _s = string
local format, byte, char = _s.format, _s.byte, _s.char
local match, gmatch, gsub = _s.match, _s.gmatch, _s.gsub
local concat, sort, insert = table.concat, table.sort, table.insert
local bit = bit or require("bit")
local band, shl, shr, sar = bit.band, bit.lshift, bit.rshift, bit.arshift
local ror, tohex, tobit = bit.ror, bit.tohex, bit.tobit

-- Inherited tables and callbacks.
local g_opt, g_arch
local wline, werror, wfatal, wwarn

-- Action name list.
-- CHECK: Keep this in sync with the C code!
local action_names = {
  "STOP", "SECTION", "ESC", "REL_EXT",
  "ALIGN", "REL_LG", "LABEL_LG",
  "LABEL_PC", "REL_PC_B", "REL_PC_J",
  "IMM_I", "IMM_S", "IMM_B", "IMM_U", "IMM_J", "IMML", "IMMV",
  "VREG",
}

-- Maximum number of section buffer positions for dasm_put().
-- CHECK: Keep this in sync with the C code!
local maxsecpos = 25 -- Keep this low, to avoid excessively long C lines.

-- Action name -> action number.
local map_action = {}
for n,name in ipairs(action_names) do
   map_action[name] = shl(n-1, 7) + 0x0B
end

-- Action list buffer.
local actlist = {}

-- Argument list for next dasm_put(). Start with offset 0 into action list.
local actargs = { 0 }

-- Current number of section buffer positions for dasm_put().
local secpos = 1

------------------------------------------------------------------------------

-- Dump action names and numbers.
local function dumpactions(out)
  out:write("DynASM encoding engine action codes:\n")
  for n,name in ipairs(action_names) do
    local num = map_action[name]
    out:write(format("  %-10s %02X  %d\n", name, num, num))
  end
  out:write("\n")
end

-- Write action list buffer as a huge static C array.
local function writeactions(out, name)
   local nn = #actlist
   if nn == 0 then nn = 1; actlist[1] = map_action.STOP end
   out:write("static const unsigned int ", name, "[", nn, "] = {\n")
   for i = 1,nn-1 do
      assert(out:write("0x", tohex(actlist[i]), ",\n"))
   end
   assert(out:write("0x", tohex(actlist[nn]), "\n};\n\n"))
end

------------------------------------------------------------------------------

-- Add word to action list.
local function wputxw(n)
  assert(n >= 0 and n <= 0xffffffff and n % 1 == 0, "word out of range")
  actlist[#actlist+1] = n
end

-- Add action to list with optional arg. Advance buffer pos, too.
local function waction(action, val, a, num)
  local w = assert(map_action[action], "bad action name `"..action.."'")
  wputxw(w + shl((val or 0), 12))
  if a then actargs[#actargs+1] = a end
  if a or num then secpos = secpos + (num or 1) end
end

-- Flush action list (intervening C code or buffer pos overflow).
local function wflush(term)
  if #actlist == actargs[1] then return end -- Nothing to flush.
  if not term then waction("STOP") end -- Terminate action list.
  wline(format("dasm_put(Dst, %s);", concat(actargs, ", ")), true)
  actargs = { #actlist } -- Actionlist offset is 1st arg to next dasm_put().
  secpos = 1 -- The actionlist offset occupies a buffer position, too.
end

-- Put escaped word.
local function wputw(n)
  if n <= 0x000fffff then waction("ESC") end
  wputxw(n)
end

-- Reserve position for word.
local function wpos()
  local pos = #actlist+1
  actlist[pos] = ""
  return pos
end

-- Store word to reserved position.
local function wputpos(pos, n)
--   print(string.format("wputpos %d %08x", pos, n)) -- xyzzy
--  assert(n >= 0 and n <= 0xffffffff and n % 1 == 0, "word out of range")
--  if n <= 0x000fffff then
--    insert(actlist, pos+1, n)
--    n = map_action.ESC * 0x10000
--    print("Adding ESC")
--  end
  actlist[pos] = n
end

------------------------------------------------------------------------------

-- Global label name -> global label number. With auto assignment on 1st use.
local next_global = 20
local map_global = setmetatable({}, { __index = function(t, name)
  if not match(name, "^[%a_][%w_]*$") then werror("bad global label") end
  local n = next_global
  if n > 2047 then werror("too many global labels") end
  next_global = n + 1
  t[name] = n
  return n
end})

-- Dump global labels.
local function dumpglobals(out, lvl)
  local t = {}
  for name, n in pairs(map_global) do t[n] = name end
  out:write("Global labels:\n")
  for i=20,next_global-1 do
    out:write(format("  %s\n", t[i]))
  end
  out:write("\n")
end

-- Write global label enum.
local function writeglobals(out, prefix)
  local t = {}
  for name, n in pairs(map_global) do t[n] = name end
  out:write("enum {\n")
  for i=20,next_global-1 do
    out:write("  ", prefix, t[i], ",\n")
  end
  out:write("  ", prefix, "_MAX\n};\n")
end

-- Write global label names.
local function writeglobalnames(out, name)
  local t = {}
  for name, n in pairs(map_global) do t[n] = name end
  out:write("static const char *const ", name, "[] = {\n")
  for i=20,next_global-1 do
    out:write("  \"", t[i], "\",\n")
  end
  out:write("  (const char *)0\n};\n")
end

------------------------------------------------------------------------------

-- Extern label name -> extern label number. With auto assignment on 1st use.
local next_extern = 0
local map_extern_ = {}
local map_extern = setmetatable({}, { __index = function(t, name)
  -- No restrictions on the name for now.
  local n = next_extern
  if n > 2047 then werror("too many extern labels") end
  next_extern = n + 1
  t[name] = n
  map_extern_[n] = name
  return n
end})

-- Dump extern labels.
local function dumpexterns(out, lvl)
  out:write("Extern labels:\n")
  for i=0,next_extern-1 do
    out:write(format("  %s\n", map_extern_[i]))
  end
  out:write("\n")
end

-- Write extern label names.
local function writeexternnames(out, name)
  out:write("static const char *const ", name, "[] = {\n")
  for i=0,next_extern-1 do
    out:write("  \"", map_extern_[i], "\",\n")
  end
  out:write("  (const char *)0\n};\n")
end

------------------------------------------------------------------------------

-- Arch-specific maps.

-- Ext. register sname -> int. name.
local map_archdef = { zero = "x0", ra = "x1", sp = "x2",
		      t0 = "x5", t1 = "x6", t2 = "x7",
		      a0 = "x10", a1 = "x11", a2 = "x12", a3 = "x13",
		      a4 = "x14", a5 = "x15", a6 = "x16", a7 = "x17",
		      s2 = "x18", s3 = "x19", s4 = "x20",
		      }

-- Int. register name -> ext. name.
local map_reg_rev = { x0 = "zero", x1 = "ra", x2 = "sp", x5 = "t0", x6 = "t1", x7 = "t2",
		      x10 = "a0", x11 = "a1", x18 = "s2", x19 = "s3", x20 = "s4" }

local map_type = {}		-- Type name -> { ctype, reg }
local ctypenum = 0		-- Type number (for Dt... macros).

-- Reverse defines for registers.
function _M.revdef(s)
  return map_reg_rev[s] or s
end

local map_shift = { lsl = 0, lsr = 1, asr = 2, }

local map_extend = {
  uxtb = 0, uxth = 1, uxtw = 2, uxtx = 3,
  sxtb = 4, sxth = 5, sxtw = 6, sxtx = 7,
}

local map_cond = {
  eq = 0, ne = 1, cs = 2, cc = 3, mi = 4, pl = 5, vs = 6, vc = 7,
  hi = 8, ls = 9, ge = 10, lt = 11, gt = 12, le = 13, al = 14,
  hs = 2, lo = 3,
}

-- ----------------------------------------------------------------------
--
-- parse_reg
--
-- Only in Load and Store instructions are these complex register
-- forms, otherwise only the base register is allowed.  This is
-- ultimately either x for integer or f for floats.  Both integer and
-- floats have 32 registers 0-31.
--
-- Code for VREG currently in here but could not see example in luajit.
--
-- ----------------------------------------------------------------------

local parse_reg_type

local function parse_reg(expr, shift)
  if not expr then werror("expected register name") end
  local tname, ovreg = match(expr, "^([%w_]+):(@?%l%d+)$") -- for type:REG
  if not tname then
    tname, ovreg = match(expr, "^([%w_]+):(R[xwqdshb]%b())$")  -- for VREG
  end
  local tp = map_type[tname or expr]
  if tp then -- if type return override or type default register
    local reg = ovreg or tp.reg
    if not reg then
      werror("type `"..(tname or expr).."' needs a register override")
    end
    expr = reg
  end
  local rt, r = match(expr, "^([xf])([123]?[0-9])$") -- match xn or fn where 0 <= n < 32
  if r then
    r = tonumber(r)
    if ((r >= 0) and (r <= 31)) then
       if not parse_reg_type then
	  parse_reg_type = rt
       elseif parse_reg_type ~= rt then
	  werror("register size mismatch")
       end
--       print("parse_reg", r, shift, expr)
       return shl(r, shift), tp, rt
    end
  end
  local vrt, vreg = match(expr, "^R([xwqdshb])(%b())$")
  if vreg then
    if not parse_reg_type then
      parse_reg_type = vrt
    elseif parse_reg_type ~= vrt then
      werror("register size mismatch")
    end
    if shift then waction("VREG", shift, vreg) end
    return 0
  end
  werror("bad register name `"..expr.."'")
end

-- ----------------------------------------------------------------------
--
-- parse_reg_base
--
-- Returns unshifted register and checks it can hold an address.
--
-- ---------------------------------------------------------------------- 

local function parse_reg_base(expr)
--   print("parse_reg_base", expr)
   local base, tp, reg_type = parse_reg(expr, 0)
   if parse_reg_type ~= "x" then werror("bad register type") end
   parse_reg_type = false
   return base, tp, reg_type
end

local parse_ctx = {}

local loadenv = setfenv and function(s)
  local code = loadstring(s, "")
  if code then setfenv(code, parse_ctx) end
  return code
end or function(s)
  return load(s, "", nil, parse_ctx)
end

-- Try to parse simple arithmetic, too, since some basic ops are aliases.
local function parse_number(n)
   local x = tonumber(n)
   if x then return x end
   local code = loadenv("return "..n)
   if code then
      local ok, y = pcall(code)
      if ok and type(y) == "number" then return y end
   end
  return nil
end

-- ----------------------------------------------------------------------
--
-- parse_imm_I
--
-- parse 12-bit signed immediate and shift to bits [31-20]
--
-- ----------------------------------------------------------------------

local function parse_imm_I(imm)
  imm = match(imm, "^#(.*)$")
  if not imm then werror("expected immediate operand") end
  local n = parse_number(imm)
  if n then
     if n >= -2048 and n < 2048 then
	if (n < 0) then
	   n = 4096 + n
	end
	return shl(n, 20)
     end
     werror("out of range immediate `"..imm.."'")
  else
     waction("IMM_I", 0, imm)
     return 0
  end
end
   
-- ----------------------------------------------------------------------
--
-- parse_load_save
--
-- ----------------------------------------------------------------------
local function parse_load_save(param, load_save, op)
   local reg_num
   local reg_type
   local immediate
   local tp
--   print("parse_load_save", param)
   local r = string.match(param, "(%([%w_:]+%))$") -- is of the form (r) or n(r)
   if (r ~= nil) then
      local reg = string.match(r, "%(([%w_:]+)%)")
      reg_num, tp, reg_type = parse_reg(reg, 0)
      immediate = string.gsub(param, "(%([%w_:]+%))$", "") -- Remove (REG) from string to get immediate
   else
--      print("parse_load_save", param)
      local reg, tailr = match(param, "^([%w_:]+)%s*(.*)$")
      if reg and (tailr ~= "") then
--	 print("parse_load_save", reg, tailr)
	 reg_num, tp, reg_type = parse_reg_base(reg)
	 if tp then
	    immediate = string.format(tp.ctypefmt, tailr)
	 else
	    werror("expected address operand")
	 end
      end
   end
   if (immediate == nil) or (immediate == "") then
      immediate = 0
   end
--   print("parse_load_save", reg_type, reg_num, immediate, tp)
--   print(string.format("Type %s, num %s, imm %s", reg_type, reg_num, immediate))
   if (load_save == "L") then
      waction("IMM_I", 0, immediate);
   end
   if (load_save == "S") then
      waction("IMM_S", 0, immediate);
   end
   return op + shl(reg_num, 15)
end


-- ----------------------------------------------------------------------
--
-- parse_label
--
-- ----------------------------------------------------------------------

local function parse_label(label, def)
  local prefix = label:sub(1, 2)  -- =>label (pc label reference)
  if prefix == "=>" then
    return "PC", 0, label:sub(3)
  end
  if prefix == "->" then   -- ->name (global label reference)
    return "LG", map_global[label:sub(3)]
  end
  if def then
    -- [1-9] (local label definition)
    if match(label, "^[1-9]$") then
      return "LG", 10+tonumber(label)
    end
  else
    -- [<>][1-9] (local label reference)
    local dir, lnum = match(label, "^([<>])([1-9])$")
    if dir then -- Fwd: 1-9, Bkwd: 11-19.
      return "LG", lnum + (dir == ">" and 0 or 10)
    end
    -- extern label (extern label reference)
    local extname = match(label, "^extern%s+(%S+)$")
    if extname then
      return "EXT", map_extern[extname]
    end
    -- &expr (pointer)
    if label:sub(1, 1) == "&" then
      return "A", 0, format("(ptrdiff_t)(%s)", label:sub(2))
    end
  end
end

------------------------------------------------------------------------------

local map_op, op_template

local function op_alias(opname, f)
  return function(params, nparams)
    if not params then return "-> "..opname:sub(1, -3) end
    f(params, nparams)
    op_template(params, map_op[opname], nparams)
  end
end

-- ----------------------------------------------------------------------
--
-- Templates for RV64I instructions
--
-- There are five instruction formats, R, I, S, B & J.  The immediate in
-- I is divided into arithmetric and shift operations.
--
-- All registers are always in the same place in the instructions, with
-- the immediate swirled around them.

-- RV32I registers are rs2, rs1 and rd and are always in the same
-- positions in the instruction as bits [24:20], [19:15] and [11:7].

-- In some cases the instruction format is the same but the syntax is
-- different.  For example ADDI is rd, rs1, imm packed into the I
-- format, where as load is rd, imm(rs1).  The instruction format is
-- the same but the assembly syntax is not.
--
-- R format is rd, rs1, rs2 and uses D, 1 & 2.  For load it is D, L
-- (RS1 + IMM) while store is RS2, S (RS1 + IMM)
--
-- ----------------------------------------------------------------------

map_op = {
   mv_2   = "00000013D1",
   addi_3 = "00000013D1I",
   ret_0  = "00008067",
   sb_2   = "000000232S",
   sd_2   = "000030232S",
   lb_2   = "00000003DL",
   ld_2   = "00003003DL",
   jalr_1 = "000000e71",
   jalr_2 = "00000067DI",
   jal_2  = "0000006fDJ",
   beq_3  = "0000006312B",
   bne_3  = "0000106312B",
}

for cond,c in pairs(map_cond) do
  map_op["b"..cond.."_1"] = tohex(0x54000000+c).."B"
end

------------------------------------------------------------------------------

-- Handle opcodes defined with template strings.
local function parse_template(params, template, nparams, pos)
  local op = tonumber(template:sub(1, 8), 16)
  local n = 1
  local rtt = {}

  -- print(params, template, nparams, pos) -- xyzzy
  
  parse_reg_type = false

  -- Process each character.
  for p in gmatch(template:sub(9), ".") do
     local q = params[n]

    if p == "D" then -- rd [11:7]
      op = op + parse_reg(q, 7); n = n + 1
    elseif p == "1" then -- rs1 [19:15]
      op = op + parse_reg(q, 15); n = n + 1
    elseif p == "2" then  -- rs2 [24:20]
      op = op + parse_reg(q, 20); n = n + 1
    elseif p == "I" then -- 12-bit immediate (I-format)
      op = op + parse_imm_I(q); n = n + 1
    elseif p == "S" then -- Save (S-format)
       op = parse_load_save(q, "S", op)
    elseif p == "L" then -- Load (I-format)
       op = parse_load_save(q, "L", op)
    elseif p == "B" then
       local mode, v, s = parse_label(q, false); n = n + 1
       if not mode then werror("bad label `"..q.."'") end
       waction("REL_PC_B", 0, s)
    elseif p == "J" then
       local mode, v, s = parse_label(q, false); n = n + 1
       if not mode then werror("bad label `"..q.."'") end
       waction("REL_PC_J", 0, s)
    else
      assert(false)
    end
  end
  wputpos(pos, op)
end

function op_template(params, template, nparams)
  if not params then return template:gsub("%x%x%x%x%x%x%x%x", "") end

  -- Limit number of section buffer positions used by a single dasm_put().
  -- A single opcode needs a maximum of 4 positions.
  if secpos+4 > maxsecpos then wflush() end
  local pos = wpos()
  local lpos, apos, spos = #actlist, #actargs, secpos

  local ok, err
  for t in gmatch(template, "[^|]+") do
    ok, err = pcall(parse_template, params, t, nparams, pos)
    if ok then return end
    secpos = spos
    actlist[lpos+1] = nil
    actlist[lpos+2] = nil
    actlist[lpos+3] = nil
    actlist[lpos+4] = nil
    actargs[apos+1] = nil
    actargs[apos+2] = nil
    actargs[apos+3] = nil
    actargs[apos+4] = nil
  end
  error(err, 0)
end

map_op[".template__"] = op_template

------------------------------------------------------------------------------

-- Pseudo-opcode to mark the position where the action list is to be emitted.
map_op[".actionlist_1"] = function(params)
  if not params then return "cvar" end
  local name = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeactions(out, name) end)
end

-- Pseudo-opcode to mark the position where the global enum is to be emitted.
map_op[".globals_1"] = function(params)
  if not params then return "prefix" end
  local prefix = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeglobals(out, prefix) end)
end

-- Pseudo-opcode to mark the position where the global names are to be emitted.
map_op[".globalnames_1"] = function(params)
  if not params then return "cvar" end
  local name = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeglobalnames(out, name) end)
end

-- Pseudo-opcode to mark the position where the extern names are to be emitted.
map_op[".externnames_1"] = function(params)
  if not params then return "cvar" end
  local name = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeexternnames(out, name) end)
end

------------------------------------------------------------------------------

-- Label pseudo-opcode (converted from trailing colon form).
map_op[".label_1"] = function(params)
  if not params then return "[1-9] | ->global | =>pcexpr" end

  if secpos+1 > maxsecpos then wflush() end
  local mode, n, s = parse_label(params[1], true)
  if not mode or mode == "EXT" then werror("bad label definition") end
  waction("LABEL_"..mode, n, s, 1)
end

------------------------------------------------------------------------------

-- Pseudo-opcodes for data storage.
local function op_data(params)
  if not params then return "imm..." end
  local sz = params.op == ".long" and 4 or 8
  for _,p in ipairs(params) do
    local imm = parse_number(p)
    if imm then
      local n = tobit(imm)
      if n == imm or (n < 0 and n + 2^32 == imm) then
	wputw(n < 0 and n + 2^32 or n)
	if sz == 8 then
	  wputw(imm < 0 and 0xffffffff or 0)
	end
      elseif sz == 4 then
	werror("bad immediate `"..p.."'")
      else
	imm = nil
      end
    end
    if not imm then
      local mode, v, s = parse_label(p, false)
      if sz == 4 then
	if mode then werror("label does not fit into .long") end
	waction("IMMV", 0, p)
      elseif mode and mode ~= "A" then
	waction("REL_"..mode, v+0x8000, s, 1)
      else
	if mode == "A" then p = s end
	waction("IMMV", 0, format("(unsigned int)(%s)", p))
	waction("IMMV", 0, format("(unsigned int)((unsigned long long)(%s)>>32)", p))
      end
    end
    if secpos+2 > maxsecpos then wflush() end
  end
end
map_op[".long_*"] = op_data
map_op[".quad_*"] = op_data
map_op[".addr_*"] = op_data

-- Alignment pseudo-opcode.
map_op[".align_1"] = function(params)
  if not params then return "numpow2" end
  if secpos+1 > maxsecpos then wflush() end
  local align = tonumber(params[1])
  if align then
    local x = align
    -- Must be a power of 2 in the range (2 ... 256).
    for i=1,8 do
      x = x / 2
      if x == 1 then
	waction("ALIGN", align-1, nil, 1) -- Action byte is 2**n-1.
	return
      end
    end
  end
  werror("bad alignment")
end

------------------------------------------------------------------------------

-- Pseudo-opcode for (primitive) type definitions (map to C types).
map_op[".type_3"] = function(params, nparams)
  if not params then
    return nparams == 2 and "name, ctype" or "name, ctype, reg"
  end
  local name, ctype, reg = params[1], params[2], params[3]
  if not match(name, "^[%a_][%w_]*$") then
    werror("bad type name `"..name.."'")
  end
  local tp = map_type[name]
  if tp then
    werror("duplicate type `"..name.."'")
  end
  -- Add #type to defines. A bit unclean to put it in map_archdef.
  map_archdef["#"..name] = "sizeof("..ctype..")"
  -- Add new type and emit shortcut define.
  local num = ctypenum + 1
  map_type[name] = {
    ctype = ctype,
    ctypefmt = format("Dt%X(%%s)", num),
    reg = reg,
  }
  wline(format("#define Dt%X(_V) (int)(ptrdiff_t)&(((%s *)0)_V)", num, ctype))
  ctypenum = num
end
map_op[".type_2"] = map_op[".type_3"]

-- Dump type definitions.
local function dumptypes(out, lvl)
  local t = {}
  for name in pairs(map_type) do t[#t+1] = name end
  sort(t)
  out:write("Type definitions:\n")
  for _,name in ipairs(t) do
    local tp = map_type[name]
    local reg = tp.reg or ""
    out:write(format("  %-20s %-20s %s\n", name, tp.ctype, reg))
  end
  out:write("\n")
end

------------------------------------------------------------------------------

-- Set the current section.
function _M.section(num)
  waction("SECTION", num)
  wflush(true) -- SECTION is a terminal action.
end

------------------------------------------------------------------------------

-- Dump architecture description.
function _M.dumparch(out)
  out:write(format("DynASM %s version %s, released %s\n\n",
    _info.arch, _info.version, _info.release))
  dumpactions(out)
end

-- Dump all user defined elements.
function _M.dumpdef(out, lvl)
  dumptypes(out, lvl)
  dumpglobals(out, lvl)
  dumpexterns(out, lvl)
end

------------------------------------------------------------------------------

-- Pass callbacks from/to the DynASM core.
function _M.passcb(wl, we, wf, ww)
  wline, werror, wfatal, wwarn = wl, we, wf, ww
  return wflush
end

-- Setup the arch-specific module.
function _M.setup(arch, opt)
  g_arch, g_opt = arch, opt
end

-- Merge the core maps and the arch-specific maps.
function _M.mergemaps(map_coreop, map_def)
  setmetatable(map_op, { __index = map_coreop })
  setmetatable(map_def, { __index = map_archdef })
  return map_op, map_def
end

return _M

------------------------------------------------------------------------------

