-- ----------------------------------------------------------------------
--
-- 2023-12-27: Add header
--
-- ----------------------------------------------------------------------

collect = {}

protocols = {}
protocols['1'] = 'icmp'
protocols['6'] = 'tcp'
protocols['17'] = 'udp'

local NetAddr = require('NetAddr')

-- ----------------------------------------------------------------------
--
-- Structured log from Junos R13
--
-- user.info 2016-01-22T14:17:54.654645+00:00 2016-01-22T14:16:52.241Z BWP-BDC-LS-ROOT02 RT_FLOW RT_FLOW_SESSION_CLOSE_LS [junos@2636.1.1.1.2.49 logical-system-name="WIG-BDC-LS01" reason="idle Timeout" source-address="10.201.14.7" source-port="56240" destination-address="172.25.168.12" destination-port="135" service-name="junos-ms-rpc-tcp" nat-source-address="10.201.14.7" nat-source-port="56240" nat-destination-address="172.25.168.12" nat-destination-port="135" src-nat-rule-name="None" dst-nat-rule-name="None" protocol-id="6" policy-name="Maypole" source-zone-name="WIG-WAN" destination-zone-name="WIG-PRM-LAN-VMA-MAI" session-id-32="20636865" packets-from-client="5" bytes-from-client="480" packets-from-server="3" bytes-from-server="364" elapsed-time="58" application="UNKNOWN" nested-application="UNKNOWN" username="N/A" roles="N/A" packet-incoming-interface="ge-2/0/0.0" encrypted="UNKNOWN"]
--
-- ----------------------------------------------------------------------


-- ----------------------------------------------------------------------
--
-- update
--
--
-- linenum 1
-- syslog-src      172.17.2.1
-- syslog-ts       2014-08-09T00:15:36.574+01:00 (converted to double)
-- timestamp       2014-08-09T00:15:36.574 (converted to double)
-- action
-- jlog    table: 0x1ab8170 (see below for entries)

-- logical-system-name     BOL-BDC-LS01
-- source-zone-name        BOL-WAN
-- destination-zone-name   BOL-LEG-Voters_Server_Farm

-- policy-name     catch-all

-- source-address  10.145.227.41
-- nat-source-address      10.145.227.41
-- destination-address     10.20.15.5
-- nat-destination-address 10.20.15.5

-- protocol-id     6

-- source-port     1575
-- nat-source-port 1575
-- destination-port        8192
-- nat-destination-port    8192
-- service-name    None

-- packet-incoming-interface       ge-2/0/1.0
-- roles   N/A
-- username        N/A
-- src-nat-rule-name       None
-- dst-nat-rule-name       None
-- session-id-32   20425157

-- ----------------------------------------------------------------------

function update(t)

-- If tdiff (time difference in seconds) is defined as a global (set in
-- main.cc and passed on command line as -t [mins]) then compare to syslog
-- timestamp and return 0 if greater.

   if (tdiff ~= nil)
   then
--      print(t['syslog-ts'], tdiff, t['syslog-ts'] - tdiff)
      if (t['syslog-ts'] < tdiff)
      then
	 return 0
      end
   end

   local jlog = t.jlog

-- Convert action field from Junos to simpler value

   local action
   if (t.action == 'RT_FLOW_SESSION_CLOSE') then
       action = 'accept'
   elseif (t.action == 'RT_FLOW_SESSION_CREATE') then
       action = 'accept'
   elseif (t.action == 'RT_FLOW_SESSION_DENY') then
       action = 'deny'
   else
       return
   end

-- Convert seperate port and protocol-id into single string

   local protocol = string.format("%s/%s", protocols[jlog['protocol-id']], jlog['destination-port'])

-- Because icmp destination-port often changes for every ping ignore it

   if jlog['protocol-id'] == '1' then
      protocol = "icmp/-"
   end

--    local tt = {}
--    table.insert(tt, jlog['destination-zone-name'])
--    table.insert(tt, jlog['destination-address'])
--    table.insert(tt, jlog['source-zone-name'])
--    table.insert(tt, jlog['source-address'])
--    table.insert(tt, protocol)
--    table.insert(tt, jlog['policy-name'])
--    table.insert(tt, action)

--    local key = table.concat(tt, '_')

   local key = jlog['destination-zone-name'] .. '_' .. jlog['destination-address'] .. '_' ..jlog['source-zone-name'] .. '_' ..jlog['source-address'] .. '_' .. protocol .. '_' ..jlog['policy-name'] .. '_' .. action

   if collect[key] == nil then
     collect[key] = {
	count = 1,
	action = action,
       first = t,
       last = t}
    else
      collect[key].count = collect[key].count + 1
      collect[key].last = t
    end

   return 1
end

-- ----------------------------------------------------------------------
--
-- output
--
-- ----------------------------------------------------------------------

function output()
   local res = {}
   for k,v in pairs(collect) do
      local sa = NetAddr.new(v.first.jlog['source-address'])
      local da = NetAddr.new(v.first.jlog['destination-address'])
      local pdn = string.format("%s/%s", protocols[v.first.jlog['protocol-id']], v.first.jlog['destination-port'])
      if v.first.jlog['protocol-id'] == '1' then
         pdn = "icmp/-"
      end
      local drfc1918
      if (da:rfc1918()) then
	 drfc1918 = "1918"
      else
	 drfc1918 = "public"
      end

      if resolve > 0 then
	 table.insert(res, {
		    count = v.count,
		    sz = v.first.jlog['source-zone-name'],
		    dz = v.first.jlog['destination-zone-name'],
		    pn = v.first.jlog['policy-name'],
		    si = sa,
		    sa = tostring(sa),
		    sn = sa:resolve(),
		    di = da,
		    da = tostring(da),
		    dr = drfc1918,
		    dn = da:resolve(),
		    pt = pdn,
		    action = v.action
	 })
      else
	 table.insert(res, {
		    count = v.count,
		    sz = v.first.jlog['source-zone-name'],
		    dz = v.first.jlog['destination-zone-name'],
		    pn = v.first.jlog['policy-name'],
		    si = sa,
		    sa = tostring(sa),
		    sn = tostring(sa),
		    di = da,
		    da = tostring(da),
		    dn = tostring(da),
		    pt = pdn,
		    action = v.action
	 })
      end
   end
   
   -- --------------------
   -- Sort table by count
   -- --------------------
   
--   table.sort(res, function(a, b) return a.di < b.di end)
   table.sort(res, function(a, b) return a.count > b.count end)
   
   for k, v in ipairs(res) do
      print(string.format("%d\t%-24s\t%-24s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s", v.count, v.sz, v.dz, v.pn, v.sa, v.sn, v.da, v.dn, v.pt, v.action, v.dr))
   end
   if json ~= nil then
      local stream = io.open(json, "w")
      stream:write(JSON:encode_pretty(res))
   end
end

----------------------------------------------------------------------
--
-- dump_table
--
----------------------------------------------------------------------

function dump_table(t)
   print("Dumping table", t)
   for k,v in pairs(t) do
      print(k, v)
      for k1, v1 in pairs(v) do
          print(k1, v1)
      end
   end
end

-- ----------------------------------------------------------------------
--
-- dump
--
-- ----------------------------------------------------------------------


function dump()
   output()
   local t = collect
end
